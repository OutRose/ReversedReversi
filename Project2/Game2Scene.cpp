#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game2Scene.h"
#include <stdlib.h>		//_itoa_s 用

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
	rbInit(&state);
	rbSetMsg(&state, turn, 0);	//先頭ターンの "BLACK TURN" メッセージをセット

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
			if (CheckHitKey(KEY_INPUT_X) == 1) changeScene(SCENE_MENU);
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

//	レンダリング処理 (γ-1 で move から分離、毎フレーム描画専用)
void renderGame2Scene(void)
{
	//プログラムで格子を描く（上ではうまく行かない）
	//第1段階：四方の枠線とフィールド緑化 (Game1 は暗緑、Game2 は明緑)
	{
		DrawBox(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_END_PX, BOARD_END_PX, GetColor(0, 140, 20), TRUE);
		int i;
		for (i = 0; i < 3; i++)
		{
			DrawLine(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_END_PX, BOARD_ORIGIN_Y, ColorWhite);
			DrawLine(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_ORIGIN_X, BOARD_END_PX, ColorWhite);
			DrawLine(BOARD_END_PX, BOARD_ORIGIN_Y, BOARD_END_PX, BOARD_END_PX, ColorWhite);
			DrawLine(BOARD_ORIGIN_X, BOARD_END_PX, BOARD_END_PX, BOARD_END_PX, ColorWhite);
		}
	}

	//第2段階：縦横の格子
	int i, j;
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
	{
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
	}

	{
		//誰のターンかを表示する
		if (turn == GAME_TURN_BLACK)
		{
			DrawString(PANEL_TURN_X, PANEL_TURN_BLACK_Y, "← Now", ColorSky);
		}
		else if (turn == GAME_TURN_WHITE)
		{
			DrawString(PANEL_TURN_X, PANEL_TURN_WHITE_Y, "← Now", ColorSky);
		}
	}

	//現在のラウンド数を表示
	DrawString(PANEL_X, PANEL_ROUND_LABEL_Y, "Round: ", ColorWhite);
	if (CurrentRound == GAME_ROUND_SECOND)
	{
		DrawFormatString(PANEL_ROUND_VALUE_X, PANEL_ROUND_LABEL_Y, ColorRed, "%d", CurrentRound);
	}
	else if (CurrentRound == GAME_ROUND_FIRST)
	{
		DrawFormatString(PANEL_ROUND_VALUE_X, PANEL_ROUND_LABEL_Y, ColorWhite, "%d", CurrentRound);
	}

	//ゲーム終了時のメニュー復帰ガイド (γ-1 で X キー復帰経路追加)
	if (status == GAME_STATUS_FINISHED && CurrentRound == GAME_ROUND_SECOND)
	{
		DrawString(PANEL_X, PANEL_END_MSG_Y, "Xキーで\nメニュー\nESCで終了", ColorWhite);
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
