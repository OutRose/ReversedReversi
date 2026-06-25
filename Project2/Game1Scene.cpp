#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game1Scene.h"
#include <stdlib.h>		//_itoa_s 用

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

//	レンダリング処理 (γ-1 で move から分離、毎フレーム描画専用)
void renderGame1Scene(void)
{
	//プログラムで格子を描く（上ではうまく行かない）
	//第1段階：四方の枠線とフィールド緑化 (Game1 は暗緑、Game2 は明緑)
	DrawBox(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_END_PX, BOARD_END_PX, GetColor(0, 100, 20), TRUE);
	int i;
	for (i = 0; i < 3; i++)
	{
		DrawLine(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_END_PX, BOARD_ORIGIN_Y, ColorWhite);
		DrawLine(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_ORIGIN_X, BOARD_END_PX, ColorWhite);
		DrawLine(BOARD_END_PX, BOARD_ORIGIN_Y, BOARD_END_PX, BOARD_END_PX, ColorWhite);
		DrawLine(BOARD_ORIGIN_X, BOARD_END_PX, BOARD_END_PX, BOARD_END_PX, ColorWhite);
	}

	//第2段階：縦横の格子
	int j;
	int DrawX = BOARD_ORIGIN_X, DrawY = BOARD_ORIGIN_Y;
	for (i = 0; i < BOARD_SIZE_MAX; i++)
	{
		DrawX = DrawX + CELL_PX;
		DrawLine(DrawX, BOARD_ORIGIN_Y, DrawX, BOARD_END_PX, ColorWhite);
	}
	for (j = 0; j < BOARD_SIZE_MAX; j++)
	{
		DrawY = DrawY + CELL_PX;
		DrawLine(BOARD_ORIGIN_X, DrawY, BOARD_END_PX, DrawY, ColorWhite);
	}

	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++)
	{
		//コマ表示部
		if (state.board[y][x]) DrawGraph(x * CELL_PX + BOARD_ORIGIN_X, y * CELL_PX + BOARD_ORIGIN_Y, pieces[state.board[y][x] - 1], TRUE);
	}

	if (status > 1)
	{
		//ターンメッセージ表示部 (PLAYING 以外の状態でメッセージ箱を表示)
		int mw = GetDrawStringWidth(state.msg.c_str(), state.msg.size());

		DrawBox(MSG_BOX_CENTER_X - mw / 2 - MSG_BOX_PADDING_X, MSG_BOX_Y_TOP, MSG_BOX_CENTER_X + mw / 2 + MSG_BOX_PADDING_X, MSG_BOX_Y_BOTTOM, GetColor(150, 150, 150), TRUE);
		DrawString(MSG_BOX_CENTER_X - mw / 2, MSG_TEXT_Y, state.msg.c_str(), ColorWhite);
	}

	//白と黒のコマ数を数え、両者の取得数を表示する（ついでに優勢な方を赤く）

	int pcnum[2] = {};
	char blackC[32], whiteC[32];
	rbCountPieces(&state, pcnum);
	_itoa_s(pcnum[0], blackC, sizeof(blackC), 10);
	_itoa_s(pcnum[1], whiteC, sizeof(whiteC), 10);

	//優勢判定
	int winning = 0;

	if (pcnum[0] > pcnum[1]) winning = 1;
	else if (pcnum[0] < pcnum[1]) winning = 2;
	else winning = 3;

	//優勢な方は文字が赤くなる
	DrawString(PANEL_X, PANEL_BLACK_LABEL_Y, "BLACK", ColorWhite);
	if (winning == 1)
	{
		DrawString(PANEL_X, PANEL_BLACK_VALUE_Y, blackC, ColorRed);
	}
	else if (winning == 2 || winning == 3)
	{
		DrawString(PANEL_X, PANEL_BLACK_VALUE_Y, blackC, ColorWhite);
	}

	DrawString(PANEL_X, PANEL_WHITE_LABEL_Y, "WHITE", ColorWhite);
	if (winning == 2)
	{
		DrawString(PANEL_X, PANEL_WHITE_VALUE_Y, whiteC, ColorRed);
	}
	else if (winning == 1 || winning == 3)
	{
		DrawString(PANEL_X, PANEL_WHITE_VALUE_Y, whiteC, ColorWhite);
	}

	//誰のターンかを表示する
	if (turn == GAME_TURN_BLACK)
	{
		DrawString(PANEL_TURN_X, PANEL_TURN_BLACK_Y, "← Now", ColorSky);
	}
	else if (turn == GAME_TURN_WHITE)
	{
		DrawString(PANEL_TURN_X, PANEL_TURN_WHITE_Y, "← Now", ColorSky);
	}

	//ゲーム終了時のメニュー復帰ガイド (γ-1 で X キー復帰経路追加)
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
