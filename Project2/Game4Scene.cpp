#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game4Scene.h"
#include <math.h>
#include <stdio.h>
#include <string>

int SqBoardA[12][12];  //フィールドは12×12マス
//なお、盤面上のデータは：0=置かれていない　1=黒　2=白

std::string msg2;	  //メッセージ格納用変数
int msg_wait2;		  //メッセージ表示の時間

int CurrentRound = 1;     //ラウンド管理用変数
int excuted = 0;        //BGMループ対策用変数

//各種フォント設定
unsigned int ColorWhite2 = GetColor(255, 255, 255);
unsigned int ColorRed2 = GetColor(255, 0, 0);
unsigned int ColorSky2 = GetColor(40, 235, 255);

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

// シーン開始前の初期化を行う
BOOL initGame4Scene(void)
{
	return TRUE;
}

// 指定した位置にコマを置く
int putPiece2(int x, int y, int turn, bool put_flag)
{
	//X,Y＝コマを置く座標　TURN＝順番（1は黒、2は白）
	//PUT_FLAG＝置けるかの確認中はFALSE、置くならばTRUE

	//なお、戻り値は裏返ったコマの数。
	//↓で宣言と初期化を行う
	int sum = 0;

	//置ける場所にコマがあれば、この時点で戻る
	if (SqBoardA[y][x] > 0) return 0;

	//置いた場所から上下左右、斜めの8方向に盤面をチェックしていく
	//dx, dyはチェックする方向を示す
	for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++)
	{
		//裏返すことができる敵コマの位置を一時格納しておく配列
		int wx[12] = { 0 }, wy[12] = { 0 };

		for (int wn = 0;; wn++)
		{
			//kx, kyでチェックする場所を示す
			int kx = x + dx * (wn + 1); int ky = y + dy * (wn + 1);

			//チェック位置が盤面からはみ出したり、空き状態の場合は裏返せないのでループ脱出
			if (kx < 0 || kx > 11 || ky < 0 || ky > 11 || SqBoardA[ky][kx] == 0) break;

			//間に挟まれたコマを実際に裏返す
			//裏返った数の合計はSUMに加算される
			if (SqBoardA[ky][kx] == turn)
			{
				if (put_flag) for (int i = 0; i < wn; i++) SqBoardA[wy[i]][wx[i]] = turn;

				sum += wn;
				break;
			}

			//敵コマならば裏返せるので、一時的に位置を格納しておく
			wx[wn] = kx; wy[wn] = ky;
		}
	}

	if (sum > 0 && put_flag) SqBoardA[y][x] = turn;

	return sum;
}

// パスチェックを行う
bool isPass2(int turn)
//ターン実行中のプレイヤーに応じて対応コマをチェックする：1=黒、2=白
{
	for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++) //置けるマスがあるかどうかを検証
	{
		if (putPiece2(x, y, turn, false)) return false;      //パス不要ならばFALSEが返される
	}
	return true;  //パスならばTRUEが返される
}

/*

思考ルーチンについての共通項

引数：TURNが1ならば黒、2ならば白のターンになる。
戻り値：コマを置いたならばTRUE、コマを置いていなければFALSE

*/

// 思考ルーチン1 プレイヤーの操作
bool think01(int turn)
{
	static bool mouse_flag = false;         //クリックされているかのフラグ

	if (GetMouseInput() & MOUSE_INPUT_LEFT) //マウス左クリックを検知した場合
	{
		if (!mouse_flag)
		{
			mouse_flag = true;   //フラグを立てる

			int mx, my;

			GetMousePoint(&mx, &my);  //マウスポインターの場所を取得

			//ポインターのある場所のマスに置く
			if (putPiece2(mx / 48, my / 48, turn, true)) return true;
		}
	}
	else mouse_flag = false;  //そうでなければフラグはしまう

	return false;
}

// 思考ルーチン2 最も多く取れるところに置く
bool think02(int turn)
{
	//MAX＝取得できるコマの最大数　wx, wy＝一番多く取れるコマを置く場所
	int max = 0, wx, wy;

	for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
	{
		//盤面を順にチェックして、取得可能なコマ数を得る
		int num = putPiece2(x, y, turn, false);

		//取れるコマが現在の最大よりも大きければ無条件入れ替え。
		//等しければ、1/2の確率で入れ替える。
		//これの繰り返しで、一番多く取れる場所を絞り込んでいく。
		if (max < num || (max == num && GetRand(1) == 0))
		{
			max = num; wx = x; wy = y;
		}
	}
	putPiece2(wx, wy, turn, true);
	return true;
}

// メッセージセット処理

// turn ... 1:BLACK 2:WHITE 3:DRAW　1で黒に対する、2で白に対するメッセージ
// type ... 0:TURN 1:PASS 2:WIN!    0でターン開始、1でパス、2で勝利メッセージ
void setMsg2(int turn, int type)
{
	std::string turn_str[] = { "BLACK", "WHITE", "DRAW" };
	std::string type_str[] = { "TURN", "PASS", "WINS!" };
	msg2 = turn_str[turn - 1];
	if (turn != 3) msg2 += " " + type_str[type];
	msg_wait2 = 60;
}

// 勝敗チェック
int checkResult2()
{
	//両プレイヤーの所持コマ数を格納する変数と、
	//勝者を判定する変数を宣言する
	int pnum[2] = {};
	int result = 0;

	//盤面のコマ数を数える。黒のコマをpnum[0]に、白をpnum[1]にセットする。
	for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
	{
		if (SqBoardA[y][x] > 0) pnum[SqBoardA[y][x] - 1]++;
	}

	//双方ともパス＝終了の合図。　勝敗チェックが始まる。
	if (isPass2(1) && isPass2(2))
	{
		//Result=1ならば黒の勝利、2ならば白の勝利、3ならば引き分けとなる。
		if (pnum[0] > pnum[1]) result = 1;
		else if (pnum[0] < pnum[1]) result = 2;
		else result = 3;
	}

	if (result) setMsg2(result, 2);
	return result;
}

//盤面を3分の2抹消し、第2ラウンドへ向かう
void removePiece()
{
	for (int j = 0; j < 96; j++)
	{
		int rmx, rmy;
		rmx = GetRand(11);
		rmy = GetRand(11);

		SqBoardA[rmy][rmx] = 0;
	}
}

//BGM変更関数
void changeBGM()
{
	StopSoundFile();
	PlaySoundFile("res/loop_68.wav", DX_PLAYTYPE_LOOP);
}

//	フレーム処理
void moveGame4Scene()
{
	int pieces[2];  //画像保存用変数　0=黒　1=白
	//int back;
	int status = 2; // 1:プレイ中 2:TURNメッセージ中 3:パスメッセージ中 4:終了
	int turn = 1;   // 1:黒ターン 2:白ターン

	//画面描画の準備
	SetDrawScreen(DX_SCREEN_BACK);
	ChangeFont("ＭＳ 明朝");

	LoadDivGraph("res/piece.png", 2, 2, 1, 47, 47, pieces); //画像読み込み：コマ（1つを分割）
	//back = LoadGraph("res/board1212.png");  //背景画像読み込み

	 //SetDrawBlendMode(DX_BLENDMODE_PMA_INVSRC, 255);  //ボード画像反転の準備

	SqBoardA[5][5] = SqBoardA[6][6] = 1;      //初期コマを盤上にセット
	SqBoardA[6][5] = SqBoardA[5][6] = 2;

	setMsg2(turn, 0);

	//ループBGMをセットする（状況に応じて）
	PlaySoundFile("res/loop_95.wav", DX_PLAYTYPE_LOOP);

	while (!ProcessMessage())
	{
		ClearDrawScreen();

		switch (status)
		{
		case 1:               //プレイモードに入る
			if (isPass2(turn)) //パスと判断された場合
			{
				setMsg2(turn, 1); //その時点のプレイヤーにパスを宣告する
				status = 3;      //パスフェイズ移行
			}
			else
			{
				/*

				思考ルーチンのthink？の組み合わせを変えると、違った形式が楽しめる！
				例：Think1同士でプレイヤー同士の対戦
				　　Think2,3を入れてCPU同士の対戦

				*/
				bool (*think[])(int) = { think01, think02 };  //思考ルーチン選択の準備

				if ((*think[turn - 1])(turn))//思考ルーチンを呼び出す
				{
					turn = 3 - turn; status = 2; //黒、白のターンを入れ替えて次へ
					setMsg2(turn, 0);
				}
			}

			if (checkResult2()) status = 4;
			break;

		case 2:     //TURNメッセージ表示中の場合
			if (msg_wait2 > 0) msg_wait2--;   //msg_waitの分だけカウントする
			else status = 1;                //カウントが終了したらプレイに移行する
			break;

		case 3:     //PASSと判定され、メッセージが表示される
			if (msg_wait2 > 0) msg_wait2--;   //msg_waitの分だけカウントする
			else
			{
				turn = 3 - turn; status = 2;//カウントが終了したらターンを移行、次へ
				setMsg2(turn, 0);
			}
			break;
		case 4:
			if (CurrentRound == 1)
			{
				WaitTimer(4000); //処理を待つ(ミリ秒)
				//ラウンド移行処理を行う
				removePiece();	//ピースを消し
				CurrentRound++;	//ラウンドカウントアップ
				status = 2;		//ステータスをプレイ中扱いに戻す
			}
			else if (CurrentRound == 2)
			{
				//ラウンド2終了時
				//終了待機状態へ飛ぶ
				releaseGame4Scene();
			}
			break;
		}

		//ESCキーが押されたらループから抜ける
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)break;

		//DrawGraph(100, 0, back, FALSE);

		//プログラムで格子を描く（上ではうまく行かない）
		//第1段階：四方の枠線とフィールド緑化
		{
			DrawBox(5, 5, 580, 580, GetColor(0, 140, 20), TRUE);
			int i;
			for (i = 0; i < 3; i++)
			{
				DrawLine(5, 5, 580, 5, ColorWhite2);
				DrawLine(5, 5, 5, 580, ColorWhite2);
				DrawLine(580, 5, 580, 580, ColorWhite2);
				DrawLine(5, 580, 580, 580, ColorWhite2);
			}
		}

		//第2段階：縦横の格子
		int i, j;
		int DrawGap = 48;   //描画間隔は50
		int DrawX = 5, DrawY = 5;
		for (i = 0; i < 11; i++)
		{
			DrawX = DrawX + DrawGap;
			DrawLine(DrawX, 5, DrawX, 580, ColorWhite2);
		}
		for (j = 0; j < 11; j++)
		{
			DrawY = DrawY + DrawGap;
			DrawLine(5, DrawY, 580, DrawY, ColorWhite2);
		}

		/*テスト用：マウス座標取得
		int mcx, mcy;
		char StrBuf[128], StrBuf2[32];
		int calcx, calcy;
		char StrBuf3[128], StrBuf4[32];
		GetMousePoint(&mcx, &mcy);
		calcx = (mcx / 48);
		calcy = (mcy / 48);
		{
			lstrcpy(StrBuf, "座標 Ｘ"); // 文字列"座標 Ｘ"をStrBufにコピー
			itoa(mcx, StrBuf2, 10); // MouseXの値を文字列にしてStrBuf2に格納
			lstrcat(StrBuf, StrBuf2); // StrBufの内容にStrBuf2の内容を付け足す
			lstrcat(StrBuf, "　Ｙ "); // StrBufの内容に文字列"Ｙ"を付け足す
			itoa(mcy, StrBuf2, 10); // MouseYの値を文字列にしてStrBuf2に格納
			lstrcat(StrBuf, StrBuf2); // StrBufの内容にStrBuf2の内容を付け足す

			lstrcpy(StrBuf3, "算出値 X");
			itoa(calcx, StrBuf4, 10);
			lstrcat(StrBuf3, StrBuf4);
			lstrcat(StrBuf3, " Y ");
			itoa(calcy, StrBuf4, 10);
			lstrcat(StrBuf3, StrBuf4);
		}
		DrawString(0, 0, StrBuf, GetColor(255, 255, 255));
		DrawString(0, 40, StrBuf3, GetColor(255, 255, 255));
		//テスト用ここまで*/

		for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
		{
			//コマ表示部
			if (SqBoardA[y][x]) DrawGraph(x * 48 + 5, y * 48 + 5, pieces[SqBoardA[y][x] - 1], TRUE);
		}

		if (status > 1)
		{
			//ターンメッセージ表示部
			int mw = GetDrawStringWidth(msg2.c_str(), msg2.size());

			DrawBox(192 - mw / 2 - 30, 630, 192 + mw / 2 + 30, 655, GetColor(150, 150, 150), TRUE);
			DrawString(192 - mw / 2, 620, msg2.c_str(), ColorWhite2);
		}

		//白と黒のコマ数を数え、両者の取得数を表示する（ついでに優勢な方を赤く）
		{
			int pcnum[2] = {};
			char blackC[32], whiteC[32];
			for (int y = 0; y < 12; y++) for (int x = 0; x < 12; x++)
			{
				if (SqBoardA[y][x] > 0) pcnum[SqBoardA[y][x] - 1]++;
			}
			itoa(pcnum[0], blackC, 10);
			itoa(pcnum[1], whiteC, 10);

			//優勢判定
			int winning = 0;

			if (pcnum[0] > pcnum[1]) winning = 1;
			else if (pcnum[0] < pcnum[1]) winning = 2;
			else winning = 3;

			//優勢な方は文字が赤くなる
			DrawString(590, 5, "BLACK", ColorWhite2);
			if (winning == 1)
			{
				DrawString(590, 40, blackC, ColorRed2);
			}
			else if (winning == 2 || winning == 3)
			{
				DrawString(590, 40, blackC, ColorWhite2);
			}

			DrawString(590, 90, "WHITE", ColorWhite2);
			if (winning == 2)
			{
				DrawString(590, 125, whiteC, ColorRed2);
			}
			else if (winning == 1 || winning == 3)
			{
				DrawString(590, 125, whiteC, ColorWhite2);
			}
		}

		{
			//誰のターンかを表示する
			if (turn == 1)
			{
				DrawString(680, 40, "← Now", ColorSky2);
			}
			else if (turn == 2)
			{
				DrawString(680, 125, "← Now", ColorSky2);
			}
		}

		//現在のラウンド数を表示
		DrawString(590, 220, "Round: ", ColorWhite2);
		if (CurrentRound == 2)
		{
			DrawFormatString(710, 220, ColorRed2, "%d", CurrentRound);
		}
		else if (CurrentRound == 1)
		{
			DrawFormatString(710, 220, ColorWhite2, "%d", CurrentRound);
		}

		//ゲーム終了時、閉じてもらうよう要請する
		if (status == 4 && CurrentRound == 2)
		{
			DrawString(590, 330, "ESCキーを\n押して終了\nしてください", ColorWhite2);
		}
		else if (status == 4 && CurrentRound == 1)
		{
			//表示するメッセージは乱数で決定する
			int msgRand = GetRand(2);
			if (msgRand == 0)
			{
				DrawString(590, 330, "まだだ……！\nまだ終わらん\nよ！", ColorWhite2);
			}
			else if (msgRand == 1)
			{
				DrawString(590, 330, "ここまでか？\n……いや！\nまだだ！", ColorWhite2);
			}
			else if (msgRand == 2)
			{
				DrawString(590, 330, "ノーカウント\nノーカウント\nなんだ…！！", ColorWhite2);
			}
		}

		//BGMを変更する
		if (CurrentRound == 2)
		{
			if (excuted == 0)
			{
				changeBGM();
				excuted++;
			}
			else if (excuted >= 1)
			{
			}
		}

		ScreenFlip();
	}
}

//	レンダリング処理
void renderGame4Scene(void)
{
}

//	シーン終了時の後処理
void releaseGame4Scene(void)
{
	//ESCキーを押したら終了する
	if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)DxLib_End();
}

// 当り判定コールバック 　　　ここでは要素を削除しないこと！！
void  Game4SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}