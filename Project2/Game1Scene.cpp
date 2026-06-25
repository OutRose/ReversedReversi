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

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

// シーン開始前の初期化を行う (シーン入場/再入場時に毎回呼ばれる)
BOOL initGame1Scene(void)
{
	//状態リセット (再入場時の前回状態クリアに必須、γ-1 で導入)
	status	= GAME_STATUS_TURN_MSG;
	turn	= GAME_TURN_BLACK;
	rbInit(&state);
	rbSetMsg(&state, turn, 0);	//先頭ターンの "BLACK TURN" メッセージをセット

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

		if (rbCheckResult(&state)) status = GAME_STATUS_FINISHED;
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
		if (CheckHitKey(KEY_INPUT_X) == 1) changeScene(SCENE_MENU);
		break;
	}
}

//	レンダリング処理 (δ-4 で共通描画ヘルパに圧縮、Game1 専用は終了メッセージのみ)
void renderGame1Scene(void)
{
	//盤面背景 (暗緑) + 枠線 + 格子線
	rbDrawBoard(GetColor(0, 100, 20));
	rbDrawGrid();

	//コマ・メッセージ箱・カウントパネル・ターンインジケータ
	rbDrawPieces(&state, pieces);
	rbDrawMsg(&state, status);
	rbDrawCountPanel(&state);
	rbDrawTurnIndicator(turn);

	//Game1 専用: ゲーム終了時のメニュー復帰ガイド (γ-1 で X キー復帰経路追加)
	if (status == GAME_STATUS_FINISHED)
	{
		DrawString(PANEL_X, PANEL_END_MSG_GAME1_Y, "Xキーでメニュー\nESCキーで終了", ColorWhite);
	}
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
