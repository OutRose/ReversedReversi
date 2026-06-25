#ifndef GAMESCENEMAIN_H_
#define GAMESCENEMAIN_H_

#include "GameMain.h"
#include <string>		//ReversiBoard.msg 用 (std::string)

// GameMain.cppファイル内の関数のうち、他のファイルから呼び出される関数のプロトタイプ宣言を記述する
BOOL InitGame(void);
void FrameMove();
void RenderScene(void);
void GameRelease(void);
void CollideCallback(int nSrc, int nTarget, int nCollideID);

//１シーン番号を管理する列挙体
//SCENE_NONEとSCENE_MAXの間に、必要なシーン番号を設定する
typedef enum _SCENE_NO {
	SCENE_NONE = -1,		// シーン番号の下限。必ず書く
	SCENE_MENU,			// メニューシーンの番号
	SCENE_GAME1,		// ふつうの次元（12×12 リバーシ本体）
	SCENE_GAME2,		// まきもどり次元（2 ラウンド制）
	SCENE_GAME3,		// あまちゃん次元（名前入力 → Game2 へ遷移）
	SCENE_GAME4,		// 空シーン雛形（新シーン追加時のコピー元、menu[] 非掲載）
	SCENE_MAX			// シーン番号の上限。必ず書く
} SCENE_NO;

//ゲーム進行状態 (Game1Scene/Game2Scene 共通、moveXxxScene の status 変数で使用)
//数値 1 から開始するのは既存実装互換のため (リテラル整数からの段階移行)
typedef enum _GAME_STATUS {
	GAME_STATUS_PLAYING = 1,	//プレイ中
	GAME_STATUS_TURN_MSG,		//TURN メッセージ表示中
	GAME_STATUS_PASS_MSG,		//PASS メッセージ表示中
	GAME_STATUS_FINISHED		//ゲーム終了 (Game2 ではラウンド遷移待ち)
} GAME_STATUS;

//手番 (Game1Scene/Game2Scene 共通)
//数値 1=BLACK / 2=WHITE で、turn = 3 - turn のような bit 反転算術が成立する設計
typedef enum _GAME_TURN {
	GAME_TURN_BLACK = 1,		//黒の手番
	GAME_TURN_WHITE = 2			//白の手番
} GAME_TURN;

//ラウンド (Game2Scene 専用、まきもどり 2 ラウンド制)
typedef enum _GAME_ROUND {
	GAME_ROUND_FIRST = 1,		//第 1 ラウンド
	GAME_ROUND_SECOND = 2		//第 2 ラウンド (ラウンド間に removePiece で 96 マス削除)
} GAME_ROUND;

//盤面状態をまとめた構造体 (Game1Scene/Game2Scene 共有、TwistTimeStopper TIMER_STATE と同方式)
//静的初期化でゼロクリアされ、rbInit() で初期 4 駒配置 + メッセージリセットされる。
//シーンの move 関数冒頭で 1 度呼ばれる想定 (β-D-5 時点ではシーン再入場時のリセットは未実装、γ-1 で対応)
typedef struct _ReversiBoard {
	int board[BOARD_SIZE][BOARD_SIZE];	//12×12 盤面 (0=空き / 1=黒 / 2=白)
	std::string msg;					//メッセージ文字列 ("BLACK TURN" 等)
	int msg_wait;						//メッセージ表示残フレーム (60FPS で 1 フレーム = 1 カウント)
} ReversiBoard;

//盤面初期化 (ゼロクリア + 中央 4 駒配置 + メッセージリセット)
void rbInit(ReversiBoard* state);

//指定位置にコマを置く (put_flag=false ならシミュレーションのみ、戻り値: 裏返った数)
int rbPutPiece(ReversiBoard* state, int x, int y, int turn, bool put_flag);

//パスチェック (置けるマスが無ければ true)
bool rbIsPass(ReversiBoard* state, int turn);

//プレイヤー思考 (マウス入力、戻り値: コマを置いたら true)
bool rbThinkPlayer(ReversiBoard* state, int turn);

//CPU 思考 (最多取得マス選択、戻り値: 常に true)
bool rbThinkCpu(ReversiBoard* state, int turn);

//メッセージセット (turn: 1=BLACK 2=WHITE 3=DRAW、type: 0=TURN 1=PASS 2=WINS!)
void rbSetMsg(ReversiBoard* state, int turn, int type);

//勝敗チェック (戻り値: 0=継続 / 1=黒勝 / 2=白勝 / 3=引分)
int rbCheckResult(ReversiBoard* state);

//黒白コマ数カウント (pcnum[0]=黒、pcnum[1]=白)
void rbCountPieces(ReversiBoard* state, int pcnum[2]);

//ランダム削除 (Game2 まきもどり用、count マスをゼロにする)
void rbRemovePieces(ReversiBoard* state, int count);

//=========================================================================
//盤面描画ヘルパ (Game1Scene/Game2Scene 共有、γ-1 後の δ-4 で renderXxxScene のクローン解消)
//=========================================================================

//盤面背景 + 四辺枠線を描画 (boardBgColor: 盤面緑、Game1=暗緑/Game2=明緑で引数差し替え)
void rbDrawBoard(int boardBgColor);

//縦横の格子線を描画 (色は ColorWhite 固定)
void rbDrawGrid(void);

//コマを描画 (pieces[0]=黒、pieces[1]=白 のハンドル配列)
void rbDrawPieces(ReversiBoard* state, int pieces[2]);

//ターン/パス/勝者メッセージ箱を描画 (status > GAME_STATUS_PLAYING の時のみ表示)
void rbDrawMsg(ReversiBoard* state, int status);

//右パネルの BLACK/WHITE カウントと優勢赤表示
void rbDrawCountPanel(ReversiBoard* state);

//誰のターンかを示す "← Now" インジケータを描画
void rbDrawTurnIndicator(int turn);

//シーンを変更する関数
void changeScene(SCENE_NO no);

#endif