#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game2Scene.h"

//盤面ステート + 状態変数 (ファイルスコープ static、シーン入場/再入場時に init で初期化)
//詳細は GameSceneMain.h の ReversiBoard / GAME_STATUS / GAME_TURN / GAME_ROUND を参照
static ReversiBoard	state		= {};
static GAME_STATUS	status		= GAME_STATUS_TURN_MSG;
static GAME_TURN	turn		= GAME_TURN_BLACK;

//画像ハンドル (LoadDivGraph の戻り値、-1 = 未読込)
//γ-1 で init 読込 + release 解放に整理 (δ-1 のリソースリーク解消も同時達成)
static int			pieces[2]	= { -1, -1 };

//まきもどり 2 ラウンド制管理 (γ-1 で static + init リセット化、再入場時の状態残留バグ解消)
static GAME_ROUND	CurrentRound		= GAME_ROUND_FIRST;	//ラウンド管理用変数
static int			excuted				= 0;	//BGM ループ対策用変数 (ラウンド 2 BGM 切替を 1 度だけ)
static int			roundTransitWait	= 0;	//ラウンド遷移待ち (240 フレーム = 4 秒、旧 WaitTimer(4000) 相当)
static int			finishedMsgRand		= 0;	//ラウンド 1 終了メッセージの抽選結果 (1 回固定、γ-1 でフリッカー解消)

//B2 ランクシステム関連 (1.6.0 で追加、Game1 と同方式、Round 2 終了時のみ XP 計算)
static int			resultXpGained	= 0;
static int			resultOldTier	= 0;
static int			resultNewTier	= 0;
static int			rankUpFrame		= 0;
static bool			xpApplied		= false;

//1.6.1 で追加 (項目 4 中断機能): Q キー 2 段階確認タイマー
//Game2 は active 範囲が広く、PLAYING/TURN_MSG/PASS_MSG に加え「ラウンド遷移待ち (FINISHED + Round 1)」も含む
static int			abortConfirmTimer	= 0;

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

// シーン開始前の初期化を行う (シーン入場/再入場時に毎回呼ばれる)
BOOL initGame2Scene(void)
{
	//状態リセット (再入場時の前回状態クリアに必須、γ-1 で導入)
	status				= GAME_STATUS_TURN_MSG;
	turn				= GAME_TURN_BLACK;
	CurrentRound		= GAME_ROUND_FIRST;
	excuted				= 0;
	roundTransitWait	= 0;
	finishedMsgRand		= 0;
	rbInit(&state, 12);	//Game2 (まきもどり) は常に 12×12 固定 (1.5.8 で size 引数明示、ラウンド間 96 マス削除前提)
	rbSetMsg(&state, turn, 0);	//先頭ターンの "BLACK TURN" メッセージをセット

	//1.6.0 ランクシステム状態リセット (再入場時に前回試合の演出が残らないように)
	resultXpGained	= 0;
	resultOldTier	= 0;
	resultNewTier	= 0;
	rankUpFrame		= 0;
	xpApplied		= false;

	//1.6.1 中断確認タイマーリセット
	abortConfirmTimer	= 0;

	//フォント設定 (init で 1 回、move/render では呼ばない)
	ChangeFont("ＭＳ 明朝");

	//リソース読込 (コマ画像 1 枚を 2x2 分割で 2 ハンドル取得: 0=黒 1=白)
	LoadDivGraph("res/piece.png", 2, 2, 1, PIECE_SIZE_PX, PIECE_SIZE_PX, pieces);

	//ループ BGM 再生 (ラウンド 1 は loop_95、ラウンド 2 で changeBGM により loop_68 に切替)
	PlaySoundFile("res/loop_95.wav", DX_PLAYTYPE_LOOP);

	return TRUE;
}

//BGM変更関数 (ラウンド 2 移行時に呼ばれる、loop_95.wav → loop_68.wav)
void changeBGM()
{
	StopSoundFile();
	PlaySoundFile("res/loop_68.wav", DX_PLAYTYPE_LOOP);
}

//	フレーム処理 (1 フレーム分の状態遷移のみ、γ-1 で独自 while ループ撤去)
void moveGame2Scene()
{
	//1.6.1: 対局中の Q キー中断機能 (PLAYING/TURN_MSG/PASS_MSG + ラウンド遷移待ち中も active)
	//ラウンド 1 終了 → 240f 待ち中 (Round 2 開始前) も「対局途中」として扱い、Q で中断可
	//Round 2 FINISHED + RANK_UP/DEMOTED 中は非アクティブ (X キー復帰の通常経路)
	bool abortActive =
		(status == GAME_STATUS_PLAYING || status == GAME_STATUS_TURN_MSG || status == GAME_STATUS_PASS_MSG) ||
		(status == GAME_STATUS_FINISHED && CurrentRound == GAME_ROUND_FIRST);
	if (tryAbortMidGame(&abortConfirmTimer, abortActive))
	{
		changeScene(SCENE_MENU);
		return;
	}

	switch (status)
	{
	case GAME_STATUS_PLAYING:               //プレイモードに入る
		if (rbIsPass(&state, turn)) //パスと判断された場合
		{
			rbSetMsg(&state, turn, 1); //その時点のプレイヤーにパスを宣告する
			status = GAME_STATUS_PASS_MSG;   //パスフェイズ移行
		}
		else
		{
			/*

			思考ルーチンのthink？の組み合わせを変えると、違った形式が楽しめる！
			例：rbThinkPlayer 同士でプレイヤー同士の対戦
			　　rbThinkCpu のみにして CPU 同士の対戦

			*/
			bool (*think[])(ReversiBoard*, int) = { rbThinkPlayer, rbThinkCpu };  //思考ルーチン選択の準備

			if ((*think[turn - 1])(&state, turn))//思考ルーチンを呼び出す
			{
				turn = (GAME_TURN)(3 - (int)turn); //黒、白のターンを入れ替えて次へ
				status = GAME_STATUS_TURN_MSG;
				rbSetMsg(&state, turn, 0);
			}
		}

		if (rbCheckResult(&state)) {
			status = GAME_STATUS_FINISHED;
			//1.6.0: Round 2 終了時のみ XP 計算 (Round 1 はラウンド遷移として扱う、ユーザー指示)
			if (CurrentRound == GAME_ROUND_SECOND && !xpApplied) {
				int pcnum[2]; rbCountPieces(&state, pcnum);
				bool won = (pcnum[0] > pcnum[1]);
				int diff = pcnum[0] - pcnum[1]; if (diff < 0) diff = -diff;
				bool perfect = won && pcnum[1] <= 5;
				resultXpGained = calcXpGain(MODE_GAME2, won, diff, state.moveCount, perfect,
					false, false, false, false);
				applyXpAndCheck(resultXpGained, won, &resultOldTier, &resultNewTier);
				g_playerStats.totalGames++;
				if (won) g_playerStats.totalWins++;
				saveOptions();
				xpApplied = true;
				if (resultNewTier > resultOldTier) {
					status = GAME_STATUS_RANK_UP;
					rankUpFrame = 0;
					//1.6.3: SE 再生を撤去 (BGM = Round 2 で loop_68.wav LOOP 中。SE 流用は同曲被りで音響的に最悪だった)
					//「演出＋無音」設計、BGM は触らず継続。将来専用 SE wav 追加時はここに PlaySoundMem 復活で対応
				}
				else if (resultNewTier < resultOldTier) {
					status = GAME_STATUS_DEMOTED;
					rankUpFrame = 0;
				}
			}
		}
		break;

	case GAME_STATUS_TURN_MSG: //TURNメッセージ表示中の場合
		if (state.msg_wait > 0) state.msg_wait--;       //msg_waitの分だけカウントする
		else status = GAME_STATUS_PLAYING;    //カウントが終了したらプレイに移行する
		break;

	case GAME_STATUS_PASS_MSG: //PASSと判定され、メッセージが表示される
		if (state.msg_wait > 0) state.msg_wait--;       //msg_waitの分だけカウントする
		else
		{
			turn = (GAME_TURN)(3 - (int)turn); //カウントが終了したらターンを移行、次へ
			status = GAME_STATUS_TURN_MSG;
			rbSetMsg(&state, turn, 0);
		}
		break;

	case GAME_STATUS_FINISHED:
		if (CurrentRound == GAME_ROUND_FIRST)
		{
			//初回フレームで終了メッセージを 1 回だけ抽選 (γ-1 で毎フレーム再抽選フリッカー解消)
			if (roundTransitWait == 0) finishedMsgRand = GetRand(2);

			//240 フレーム待ってからラウンド遷移 (旧 WaitTimer(4000) 相当、フレーム駆動化)
			roundTransitWait++;
			if (roundTransitWait >= 240)
			{
				rbRemovePieces(&state, 96);	//ピースを消し (96 マスをランダムに 0 化、まきもどり演出)
				CurrentRound = GAME_ROUND_SECOND; //ラウンドカウントアップ
				status = GAME_STATUS_TURN_MSG;    //ステータスをプレイ中扱いに戻す
				roundTransitWait = 0;
			}
		}
		else if (CurrentRound == GAME_ROUND_SECOND)
		{
			//ラウンド 2 終了時: X キーでメニュー復帰 (γ-1 で旧 releaseGame2Scene 直呼びを撤去)
			//1.6.1 polish: isXKeyJustPressed でエッジ検出、RANK_UP/DEMOTED スキップからの連鎖を防止
			if (isXKeyJustPressed()) changeScene(SCENE_MENU);
		}
		break;

	case GAME_STATUS_RANK_UP:	//1.6.0 ランクアップ演出 (240f アニメ、スキップ可)
		rankUpFrame++;
		if (rankUpFrame >= RANK_UP_DURATION_FRAMES || (EdgeInput & PAD_INPUT_1) || isXKeyJustPressed()) {
			status = GAME_STATUS_FINISHED;
			//1.6.3: SE 撤去により StopSoundMem も不要、BGM は元から触っていないため自然継続
		}
		break;

	case GAME_STATUS_DEMOTED:	//1.6.0 降格演出 (120f アニメ、スキップ可)
		rankUpFrame++;
		if (rankUpFrame >= DEMOTE_DURATION_FRAMES || (EdgeInput & PAD_INPUT_1) || isXKeyJustPressed()) {
			status = GAME_STATUS_FINISHED;
		}
		break;
	}

	//ラウンド 2 移行時の BGM 切替 (1 度だけ、excuted フラグで再呼び出し防止)
	if (CurrentRound == GAME_ROUND_SECOND && excuted == 0)
	{
		changeBGM();
		excuted = 1;
	}
}

//	レンダリング処理 (δ-4 で共通描画ヘルパに圧縮、Game2 専用はラウンド表示と終了メッセージ)
void renderGame2Scene(void)
{
	//盤面背景 (明緑) + 枠線 + 格子線
	rbDrawBoard(&state, GetColor(0, 140, 20));
	rbDrawGrid(&state);

	//コマ・メッセージ箱・カウントパネル・ターンインジケータ
	rbDrawPieces(&state, pieces);
	rbDrawMsg(&state, status);
	rbDrawCountPanel(&state);
	rbDrawTurnIndicator(turn);

	//Game2 専用: 現在のラウンド数を表示 (ラウンド 2 は強調赤、ラウンド 1 は白)
	DrawString(PANEL_X, PANEL_ROUND_LABEL_Y, "Round: ", ColorWhite);
	int roundColor = (CurrentRound == GAME_ROUND_SECOND) ? ColorRed : ColorWhite;
	DrawFormatString(PANEL_ROUND_VALUE_X, PANEL_ROUND_LABEL_Y, roundColor, "%d", CurrentRound);

	//Game2 専用: ゲーム終了時のメニュー復帰ガイド (ラウンドで文言切替)
	if (status == GAME_STATUS_FINISHED && CurrentRound == GAME_ROUND_SECOND)
	{
		DrawString(PANEL_X, PANEL_END_MSG_Y, "Xキーで\nメニュー\nESCで終了", ColorWhite);
		//1.6.0: ラウンド 2 終了時のみ獲得 XP オーバーレイ (Round 1 終了は表示しない)
		rbDrawResultOverlay(resultXpGained, resultOldTier, resultNewTier);
	}
	else if (status == GAME_STATUS_FINISHED && CurrentRound == GAME_ROUND_FIRST)
	{
		//表示するメッセージは move で 1 回抽選した finishedMsgRand を参照 (フリッカー解消)
		if (finishedMsgRand == 0)
		{
			DrawString(PANEL_X, PANEL_END_MSG_Y, "まだだ……！\nまだ終わらん\nよ！", ColorWhite);
		}
		else if (finishedMsgRand == 1)
		{
			DrawString(PANEL_X, PANEL_END_MSG_Y, "ここまでか？\n……いや！\nまだだ！", ColorWhite);
		}
		else if (finishedMsgRand == 2)
		{
			DrawString(PANEL_X, PANEL_END_MSG_Y, "ノーカウント\nノーカウント\nなんだ…！！", ColorWhite);
		}
	}

	//1.6.0: 対局中の右パネル下部に小ランク章を表示 (FINISHED/RANK_UP/DEMOTED 中はオーバーレイで隠れるので非表示、Round 表示の下)
	if (status == GAME_STATUS_PLAYING || status == GAME_STATUS_TURN_MSG || status == GAME_STATUS_PASS_MSG)
	{
		rbDrawRankBadgeSmall(PANEL_X, 480);
	}

	//1.6.0: ランクアップ/降格演出 (FINISHED の上にフルスクリーンオーバーレイ)
	if (status == GAME_STATUS_RANK_UP)
	{
		rbDrawRankUpAnimation(rankUpFrame, resultOldTier, resultNewTier);
	}
	else if (status == GAME_STATUS_DEMOTED)
	{
		rbDrawDemoteAnimation(rankUpFrame, resultOldTier, resultNewTier);
	}

	//1.6.1: 中断ガイド (PLAYING/TURN_MSG/PASS_MSG + ラウンド遷移待ち中も active) + 確認モード中央オーバーレイ
	//Game2 ラウンド 1 FINISHED 時は finish msg (Y=410..530 の 3 行予告文) と被るので guideY=350 に上げる
	//(Round 表示 Y=240+40=280 と finish msg 開始 Y=410 の間に挟む、ガイド Y=350..378 で双方と 30px 程度の gap 確保)
	bool abortActive =
		(status == GAME_STATUS_PLAYING || status == GAME_STATUS_TURN_MSG || status == GAME_STATUS_PASS_MSG) ||
		(status == GAME_STATUS_FINISHED && CurrentRound == GAME_ROUND_FIRST);
	int abortGuideY = (status == GAME_STATUS_FINISHED && CurrentRound == GAME_ROUND_FIRST) ? 350 : 420;
	rbDrawAbortGuide(abortConfirmTimer, abortActive, abortGuideY);
}

//	シーン終了時の後処理 (シーン退場時に毎回呼ばれる、γ-1 でリソース解放を追加 + γ-2 副次解消で DxLib_End 撤去)
void releaseGame2Scene(void)
{
	//BGM 停止 (メニュー復帰時に Game2 BGM が鳴り続けないように)
	StopSoundFile();

	//画像ハンドル解放 (再入場時の二重ロードを防ぎリーク解消、δ-1 前倒し)
	if (pieces[0] != -1)
	{
		DeleteGraph(pieces[0]);
		DeleteGraph(pieces[1]);
		pieces[0] = pieces[1] = -1;
	}
}

// 当り判定コールバック 　　　ここでは要素を削除しないこと！！
void  Game2SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}
