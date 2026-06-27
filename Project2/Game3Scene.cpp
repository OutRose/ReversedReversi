#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game3Scene.h"

//Game3 (あまちゃん次元) 内部フェーズ — 名前入力 → リバーシ対局の 2 段構造
//γ-3 (2026-06-26) で導入、TwistTimeStopper の TIMER_STATUS 流の状態管理
typedef enum _GAME3_PHASE {
	GAME3_PHASE_NAME_ENTRY = 0,	//名前入力中 (KeyInputSingleCharString)
	GAME3_PHASE_PLAYING			//リバーシ対局中 (Game1 ベース + 弱い CPU)
} GAME3_PHASE;

//ファイルスコープ static (Game1/Game2 と同じパターン、シーン入場/再入場時に init で初期化)
static GAME3_PHASE	phase		= GAME3_PHASE_NAME_ENTRY;
static ReversiBoard	state		= {};
static GAME_STATUS	status		= GAME_STATUS_TURN_MSG;
static GAME_TURN	turn		= GAME_TURN_BLACK;
static int			pieces[2]	= { -1, -1 };	//LoadDivGraph ハンドル (-1 = 未読込)

//「待った」機能用の前手スナップショット (Game3 専用、あまちゃん向け 1 手戻し、1.5.6 で追加)
//プレイヤーが思考確定 (コマを置いた) 瞬間に「その直前の盤面状態」を保存し、
//R キー押下で state/status/turn を一括復元する。CPU の応手も巻き戻る (1 ターン = プレイヤー手 → CPU 手 のセット単位)
//終局状態 (FINISHED) からは巻き戻し不可 — 勝敗を尊重してプレイを区切る方針
static ReversiBoard	prevState		= {};
static GAME_STATUS	prevStatus		= GAME_STATUS_TURN_MSG;
static GAME_TURN	prevTurn		= GAME_TURN_BLACK;
static bool			undoAvailable	= false;	//true = R で復元可能、開始直後 / undo 直後は false

//外部定義 (GameMain.cpp にて宣言、Game3 で入力 → Game3 対局画面で表示)
extern int Input, EdgeInput;
extern char nameTmp[12];

//シーン開始前の初期化を行う (シーン入場/再入場時に毎回呼ばれる)
BOOL initGame3Scene(void)
{
	//フェーズ・状態リセット (再入場時の前回状態クリアに必須)
	phase		= GAME3_PHASE_NAME_ENTRY;
	nameTmp[0]	= '\0';	//名前入力バッファをクリア (元コードのリセット欠落も同時解消)

	//対局フェーズ用の状態リセット (1.5.8 で OPTIONS の boardSize を反映、init 時に確定)
	status		= GAME_STATUS_TURN_MSG;
	turn		= GAME_TURN_BLACK;
	rbInit(&state, g_game3Options.boardSize);
	rbSetMsg(&state, turn, 0);	//先頭ターンの "BLACK TURN" メッセージをセット

	//「待った」スナップショットもリセット (再入場時の前回値残留防止、ゲーム開始直後は undo 不可)
	prevState		= {};
	prevStatus		= GAME_STATUS_TURN_MSG;
	prevTurn		= GAME_TURN_BLACK;
	undoAvailable	= false;

	//フォント設定 (init で 1 回、move/render では呼ばない)
	ChangeFont("ＭＳ 明朝");

	//リソース読込 (PHASE_PLAYING で使用、init で 1 回ロード)
	LoadDivGraph("res/piece.png", 2, 2, 1, PIECE_SIZE_PX, PIECE_SIZE_PX, pieces);

	//ループ BGM 再生 (Game1 と同じ loop_95)
	PlaySoundFile("res/loop_95.wav", DX_PLAYTYPE_LOOP);

	return TRUE;
}

//	フレーム処理 (1 フレーム分の状態遷移のみ、フレーム駆動モデル)
void moveGame3Scene()
{
	switch (phase)
	{
	case GAME3_PHASE_NAME_ENTRY:
		//名前入力フェーズ: Enter キーで対局へ、X キーでメニュー復帰
		//KeyInputSingleCharString は render 側で呼ぶ (入力受付 + 描画を兼ねるため)
		if (CheckHitKey(KEY_INPUT_RETURN) == 1 && nameTmp[0] != '\0')
		{
			phase = GAME3_PHASE_PLAYING;
		}
		if (CheckHitKey(KEY_INPUT_X) == 1) changeScene(SCENE_MENU);
		break;

	case GAME3_PHASE_PLAYING:
		//R キーで「待った」 (1.5.6 で導入、1.5.7 で allowUndo ゲート追加)
		//PLAYING/TURN_MSG/PASS_MSG 中で有効、FINISHED 中は無効 (勝敗を尊重)、allowUndo=false なら無効
		if (g_game3Options.allowUndo && status != GAME_STATUS_FINISHED && undoAvailable && CheckHitKey(KEY_INPUT_R) == 1)
		{
			state			= prevState;
			status			= prevStatus;
			turn			= prevTurn;
			undoAvailable	= false;	//1 回使ったら次のプレイヤー手まで使えない
		}

		//対局フェーズ: Game1 と同じ進行、思考テーブルは weakCpu トグルで切替 (1.5.7 で導入)
		switch (status)
		{
		case GAME_STATUS_PLAYING:
			if (rbIsPass(&state, turn))
			{
				rbSetMsg(&state, turn, 1);
				status = GAME_STATUS_PASS_MSG;
			}
			else
			{
				//weakCpu=true なら rbThinkRandom (弱、置けるマスからランダム選択)、false なら rbThinkCpu (貪欲、最多取得)
				bool (*think[])(ReversiBoard*, int) = { rbThinkPlayer, g_game3Options.weakCpu ? rbThinkRandom : rbThinkCpu };

				//「待った」用スナップショット (1.5.6 で導入、1.5.7 で allowUndo ゲート追加): プレイヤー手番のみ思考前に保持
				//思考が確定 (think が true 返却) した瞬間に prev* に persist し undo を有効化
				ReversiBoard	preMoveState	= state;
				GAME_STATUS		preMoveStatus	= status;
				GAME_TURN		preMoveTurn		= turn;
				bool isPlayerTurn = (turn == GAME_TURN_BLACK);

				if ((*think[turn - 1])(&state, turn))
				{
					//プレイヤーがコマを置いた瞬間に prev* を更新 (CPU の手では保存しない、CPU 応手後の盤面から戻れる設計)
					//allowUndo=false ならスナップショット save も skip (パフォーマンスより意図の明示性優先)
					if (g_game3Options.allowUndo && isPlayerTurn)
					{
						prevState		= preMoveState;
						prevStatus		= preMoveStatus;
						prevTurn		= preMoveTurn;
						undoAvailable	= true;
					}
					turn = (GAME_TURN)(3 - (int)turn);
					status = GAME_STATUS_TURN_MSG;
					rbSetMsg(&state, turn, 0);
				}
			}

			if (rbCheckResult(&state)) status = GAME_STATUS_FINISHED;
			break;

		case GAME_STATUS_TURN_MSG:
			if (state.msg_wait > 0) state.msg_wait--;
			else status = GAME_STATUS_PLAYING;
			break;

		case GAME_STATUS_PASS_MSG:
			if (state.msg_wait > 0) state.msg_wait--;
			else
			{
				turn = (GAME_TURN)(3 - (int)turn);
				status = GAME_STATUS_TURN_MSG;
				rbSetMsg(&state, turn, 0);
			}
			break;

		case GAME_STATUS_FINISHED:
			//X キーでメニュー復帰 (Game1/Game2 と同じ TwistTimeStopper 流)
			if (CheckHitKey(KEY_INPUT_X) == 1) changeScene(SCENE_MENU);
			break;
		}
		break;
	}
}

//	レンダリング処理 (フェーズに応じて名前入力画面 or 対局画面を描画)
void renderGame3Scene(void)
{
	switch (phase)
	{
	case GAME3_PHASE_NAME_ENTRY:
		//表題・指示を描画する (1.5.9 で 1280×768 用に再配置、フォント拡張に伴い座標も調整)
		SetFontSize(55);
		SetFontThickness(11);
		DrawString(480, 80, "NAME ENTRY", ColorWhite);

		SetFontSize(36);
		SetFontThickness(5);
		DrawString(280, 180, "アルファベットで名前を入力してください\n入力が終わったら、Enterキーを\n押してください", ColorWhite);

		//入力中の名前を表示する (内側で KeyInputSingleCharString が描画も担うので併用、太字で強調)
		SetFontSize(50);
		SetFontThickness(7);
		//名前下に線を表示（11文字分程度、X の長さ = 310 で拡張）
		DrawLine(485, 470, 795, 470, ColorWhite);

		//入力を検知・代入 (FALSE = 確定処理しない、毎フレーム継続入力)
		//γ-3 で move → render に移動 (KeyInputSingleCharString は描画も兼ねるため)
		KeyInputSingleCharString(490, 410, 40, nameTmp, FALSE);

		//入力完了の案内 (1 文字でも入力されたら Enter 待ち、γ-3 で即座遷移バグ解消)
		if (nameTmp[0] != '\0')
		{
			SetFontSize(36);
			DrawString(120, 540, "入力完了。\nEnterキーを押して対局開始！", ColorWhite);
		}

		//X キーでメニュー復帰の案内 (1.5.9 768px 化で y を 780→700 に上げ、画面内に収める)
		SetFontSize(FONT_SIZE_DEFAULT);
		DrawString(120, 700, "Xキーでメニューに戻る", ColorSky);
		break;

	case GAME3_PHASE_PLAYING:
		//盤面背景 (Game1 と同じ暗緑) + 共通描画ヘルパ (1.5.8 で state ベースに動的化)
		rbDrawBoard(&state, GetColor(0, 100, 20));
		rbDrawGrid(&state);
		rbDrawPieces(&state, pieces);

		//Game3 専用: プレイヤー手番中のみヒント表示 (1.5.7 で showHints / showGain トグル参照)
		//PLAYING 中以外 (TURN_MSG/PASS_MSG/FINISHED) は混乱を招くため非表示、CPU 手番も同様
		//showHints=false なら非表示、showHints=true でも showGain=false ならオレンジ丸のみで数字なし
		if (g_game3Options.showHints && status == GAME_STATUS_PLAYING && turn == GAME_TURN_BLACK)
		{
			rbDrawHints(&state, turn, g_game3Options.showGain);
		}

		rbDrawMsg(&state, status);
		rbDrawCountPanel(&state);
		rbDrawTurnIndicator(turn);

		//Game3 専用: 右パネルに PLAYER 名を表示 (名前入力フェーズで入力した nameTmp)
		//line spacing は font 40 高さに合わせて +50 (1.5.9 で +35 → +50、5px overlap 解消)
		SetFontSize(FONT_SIZE_DEFAULT);
		DrawString(PANEL_X, PANEL_ROUND_LABEL_Y, "PLAYER:", ColorWhite);
		DrawString(PANEL_X, PANEL_ROUND_LABEL_Y + 50, nameTmp, ColorSky);

		//Game3 専用:「待った」可用時のガイド (1.5.6 で導入、1.5.7 で allowUndo ゲート追加)
		//allowUndo=false / FINISHED 中は R が無効化されているため非表示にして誤解防止
		//テキストは「R: 待った」に短縮 (フォント 40 で約 180px)、PANEL_X=745 から余裕あり
		//y は PANEL_ROUND_LABEL_Y + 110 (1.5.9 で +70 → +110、nameTmp y=290..330 との 5px overlap 解消 + PANEL_END_MSG_Y=410 とも独立)
		if (g_game3Options.allowUndo && undoAvailable && status != GAME_STATUS_FINISHED)
		{
			DrawString(PANEL_X, PANEL_ROUND_LABEL_Y + 110, "R: 待った", ColorSky);
		}

		//ゲーム終了時のメニュー復帰ガイド (3 行で画面内に収める)
		if (status == GAME_STATUS_FINISHED)
		{
			DrawString(PANEL_X, PANEL_END_MSG_Y, "Xキーで\nメニュー\nESCで終了", ColorWhite);
		}
		break;
	}
}

//シーン終了時の後処理 (Game1/Game2 と同じパターン、リソース解放)
void releaseGame3Scene(void)
{
	//BGM 停止 (メニュー復帰時に Game3 BGM が鳴り続けないように)
	StopSoundFile();

	//画像ハンドル解放 (再入場時の二重ロード防止)
	if (pieces[0] != -1)
	{
		DeleteGraph(pieces[0]);
		DeleteGraph(pieces[1]);
		pieces[0] = pieces[1] = -1;
	}
}

//当たり判定コールバック（要素削除禁止）
void Game3SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}
