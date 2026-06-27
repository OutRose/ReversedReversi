#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game1Scene.h"

//盤面ステート + 状態変数 (ファイルスコープ static、シーン入場/再入場時に init で初期化)
//詳細は GameSceneMain.h の ReversiBoard / GAME_STATUS / GAME_TURN を参照
static ReversiBoard	state		= {};
static GAME_STATUS	status		= GAME_STATUS_TURN_MSG;
static GAME_TURN	turn		= GAME_TURN_BLACK;

//画像ハンドル (LoadDivGraph の戻り値、-1 = 未読込)
//γ-1 で init 読込 + release 解放に整理 (δ-1 のリソースリーク解消も同時達成)
static int			pieces[2]	= { -1, -1 };

//B2 ランクシステム関連 (1.6.0 で追加、終局時の XP 計算結果 + 演出フレームカウンタ)
static int			resultXpGained	= 0;	//今回獲得 XP (FINISHED 状態のオーバーレイ表示用)
static int			resultOldTier	= 0;	//XP 加算前のティア (演出用)
static int			resultNewTier	= 0;	//XP 加算後のティア (演出 + 状態振り分け判定用)
static int			rankUpFrame		= 0;	//RANK_UP/DEMOTED 演出のフレームカウンタ (0..RANK_UP_DURATION-1)
static bool			xpApplied		= false;	//FINISHED 突入時に XP 加算済かのフラグ (1 試合 1 回保証)

//1.6.1 で追加 (項目 4 中断機能): Q キー 2 段階確認タイマー (0=待機、>0=確認モード残フレーム)
static int			abortConfirmTimer	= 0;

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

// シーン開始前の初期化を行う (シーン入場/再入場時に毎回呼ばれる)
BOOL initGame1Scene(void)
{
	//状態リセット (再入場時の前回状態クリアに必須、γ-1 で導入)
	status	= GAME_STATUS_TURN_MSG;
	turn	= GAME_TURN_BLACK;
	rbInit(&state, 12);	//Game1 (ふつう) は常に 12×12 固定 (1.5.8 で size 引数明示)
	rbSetMsg(&state, turn, 0);	//先頭ターンの "BLACK TURN" メッセージをセット

	//1.6.0 ランクシステム状態リセット (再入場時に前回試合の演出が残らないように)
	resultXpGained	= 0;
	resultOldTier	= 0;
	resultNewTier	= 0;
	rankUpFrame		= 0;
	xpApplied		= false;

	//1.6.1 中断確認タイマーリセット (前回試合の確認モード残留を防止)
	abortConfirmTimer	= 0;

	//フォント設定 (init で 1 回、move/render では呼ばない)
	ChangeFont("ＭＳ 明朝");

	//リソース読込 (コマ画像 1 枚を 2x2 分割で 2 ハンドル取得: 0=黒 1=白)
	LoadDivGraph("res/piece.png", 2, 2, 1, PIECE_SIZE_PX, PIECE_SIZE_PX, pieces);

	//ループ BGM 再生
	PlaySoundFile("res/loop_95.wav", DX_PLAYTYPE_LOOP);

	return TRUE;
}

//	フレーム処理 (1 フレーム分の状態遷移のみ、γ-1 で独自 while ループ撤去)
void moveGame1Scene()
{
	//1.6.1: 対局中の Q キー中断機能 (PLAYING/TURN_MSG/PASS_MSG 中のみ active、XP 計上なし)
	//FINISHED/RANK_UP/DEMOTED 中は非アクティブ (X キーでメニュー復帰の通常経路を使う)
	bool abortActive = (status == GAME_STATUS_PLAYING || status == GAME_STATUS_TURN_MSG || status == GAME_STATUS_PASS_MSG);
	if (tryAbortMidGame(&abortConfirmTimer, abortActive))
	{
		changeScene(SCENE_MENU);	//XP 計算 / totalGames++ は走らない、release で state クリア
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
			//1.6.0: 終局時に XP 計算 + ティア再判定 + 演出状態振り分け (1 試合 1 回保証)
			if (!xpApplied) {
				int pcnum[2]; rbCountPieces(&state, pcnum);
				bool won = (pcnum[0] > pcnum[1]);	//プレイヤー=黒、コマ数多い方が勝ち
				int diff = pcnum[0] - pcnum[1]; if (diff < 0) diff = -diff;
				bool perfect = won && pcnum[1] <= 5;
				resultXpGained = calcXpGain(MODE_GAME1, won, diff, state.moveCount, perfect,
					false, false, false, false);
				applyXpAndCheck(resultXpGained, won, &resultOldTier, &resultNewTier);
				g_playerStats.totalGames++;
				if (won) g_playerStats.totalWins++;
				saveOptions();	//即時永続化 (settings.ini に書込)
				xpApplied = true;
				//演出状態振り分け
				if (resultNewTier > resultOldTier) {
					status = GAME_STATUS_RANK_UP;
					rankUpFrame = 0;
					//1.6.3: SE 再生を撤去 (BGM = loop_95.wav LOOP を中断せず継続、演出は視覚のみ)
					//1.6.1 で loop_68.wav を SE 流用したが BGM と長尺重複、1.6.2 で StopSoundMem 制限したが同曲被りが残存
					//根本対策として「演出＋無音」設計に変更。将来専用 SE wav 追加時はここに PlaySoundMem 復活で対応
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
		else status = GAME_STATUS_PLAYING;  //カウントが終了したらプレイに移行する
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

	case GAME_STATUS_FINISHED: //ゲーム終了
		//X キーでメニューに復帰 (TwistTimeStopper 流、ESC = プロセス終了は WinMain 側で処理)
		//1.6.1 polish: isXKeyJustPressed でエッジ検出、RANK_UP/DEMOTED スキップからの押しっぱなし連鎖を防止
		if (isXKeyJustPressed()) changeScene(SCENE_MENU);
		break;

	case GAME_STATUS_RANK_UP:	//1.6.0 ランクアップ演出 (240f アニメ、X キーまたはボタン 1 でスキップ可)
		rankUpFrame++;
		if (rankUpFrame >= RANK_UP_DURATION_FRAMES || (EdgeInput & PAD_INPUT_1) || isXKeyJustPressed()) {
			status = GAME_STATUS_FINISHED;	//演出終了で通常 FINISHED に戻る (X キーガイド表示)
			//1.6.3: SE 撤去により StopSoundMem も不要、BGM は元から触っていないため自然継続
		}
		break;

	case GAME_STATUS_DEMOTED:	//1.6.0 降格演出 (120f アニメ、同じくスキップ可)
		rankUpFrame++;
		if (rankUpFrame >= DEMOTE_DURATION_FRAMES || (EdgeInput & PAD_INPUT_1) || isXKeyJustPressed()) {
			status = GAME_STATUS_FINISHED;
		}
		break;
	}
}

//	レンダリング処理 (δ-4 で共通描画ヘルパに圧縮、Game1 専用は終了メッセージのみ)
void renderGame1Scene(void)
{
	//盤面背景 (暗緑) + 枠線 + 格子線
	rbDrawBoard(&state, GetColor(0, 100, 20));
	rbDrawGrid(&state);

	//コマ・メッセージ箱・カウントパネル・ターンインジケータ
	rbDrawPieces(&state, pieces);
	rbDrawMsg(&state, status);
	rbDrawCountPanel(&state);
	rbDrawTurnIndicator(turn);

	//1.6.0: 対局中の右パネル下部に小ランク章を表示 (FINISHED/RANK_UP/DEMOTED 中はオーバーレイで隠れるので非表示)
	if (status == GAME_STATUS_PLAYING || status == GAME_STATUS_TURN_MSG || status == GAME_STATUS_PASS_MSG)
	{
		rbDrawRankBadgeSmall(PANEL_X, 480);
	}

	//Game1 専用: ゲーム終了時のメニュー復帰ガイド (γ-1 で X キー復帰経路追加、3 行で画面内に収める)
	if (status == GAME_STATUS_FINISHED)
	{
		DrawString(PANEL_X, PANEL_END_MSG_GAME1_Y, "Xキーで\nメニュー\nESCで終了", ColorWhite);
		//1.6.0: 獲得 XP オーバーレイ (半透明背景 + +XP + TOTAL/NEXT 表示)
		rbDrawResultOverlay(resultXpGained, resultOldTier, resultNewTier);
	}
	//1.6.0: ランクアップ/降格演出 (FINISHED の上にフルスクリーンオーバーレイ)
	else if (status == GAME_STATUS_RANK_UP)
	{
		rbDrawRankUpAnimation(rankUpFrame, resultOldTier, resultNewTier);
	}
	else if (status == GAME_STATUS_DEMOTED)
	{
		rbDrawDemoteAnimation(rankUpFrame, resultOldTier, resultNewTier);
	}

	//1.6.1: 中断ガイド (PLAYING/TURN_MSG/PASS_MSG 中のみ) + 確認モード中央オーバーレイ
	bool abortActive = (status == GAME_STATUS_PLAYING || status == GAME_STATUS_TURN_MSG || status == GAME_STATUS_PASS_MSG);
	rbDrawAbortGuide(abortConfirmTimer, abortActive);
}

//	シーン終了時の後処理 (シーン退場時に毎回呼ばれる、γ-1 でリソース解放を追加)
void releaseGame1Scene(void)
{
	//BGM 停止 (メニュー復帰時に Game1 BGM が鳴り続けないように)
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
void  Game1SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}
