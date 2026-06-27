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
	SCENE_TEMPLATE,		// 空シーン雛形（新シーン追加時のコピー元、menu[] 非掲載。1.5.8 で旧 SCENE_GAME4 から固定名に改名）
	SCENE_OPTIONS,		// オプション設定画面（1.5.7 で導入、Game3 のトグル設定）
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

//Game3 (あまちゃん) のオプション設定 (1.5.7 で導入、settings.ini で永続化)
//5 トグル: ヒント表示 / 取得コマ数表示 / 弱い CPU / 待った機能 / 盤面サイズ (1.5.8 で boardSize 追加)
//デフォルト: 4 トグルすべて true + boardSize=12 (現状の Game3 動作と一致)
typedef struct _ReversiOptions {
	bool showHints;		//置けるマスのオレンジ丸ハイライト
	bool showGain;		//ヒント丸内の取得コマ数表示 (showHints=true 時のみ意味あり)
	bool weakCpu;		//true=rbThinkRandom (弱) / false=rbThinkCpu (貪欲、強)
	bool allowUndo;		//R キーでの 1 手戻し
	int  boardSize;		//盤面サイズ 6/8/10/12 (1.5.8 で追加、Game3 専用、デフォルト 12)
} ReversiOptions;

//Game3 オプション設定のグローバル状態 (定義は GameMain.cpp、起動時に settings.ini から読込)
extern ReversiOptions g_game3Options;

//盤面サイズ選択肢 (1.5.8 で導入、OPTIONS シーンの ←→ サイクル + load 時の検証で参照)
//6×6 = 超短時間、8×8 = 標準リバーシ、10×10 = 中間、12×12 = 本作デフォルト
//1.5.9 で BOARD_TARGET_PX=720 に拡張、全選択肢が綺麗に割り切れる (120/90/72/60)
#define BOARD_SIZE_CHOICE_COUNT 4
extern const int BOARD_SIZE_CHOICES[BOARD_SIZE_CHOICE_COUNT];

//盤面状態をまとめた構造体 (Game1Scene/Game2Scene/Game3Scene 共有、TwistTimeStopper TIMER_STATE と同方式)
//静的初期化でゼロクリアされ、rbInit(state, size) で初期 4 駒配置 + メッセージリセット + サイズ依存メトリクスを設定。
//ストレージ board[][] は常に 12×12 で確保し、有効領域は左上 size×size のみ使用 (1.5.8 で動的サイズ対応)
//1.5.9 で BOARD_TARGET_PX=720 に拡張、全サイズが綺麗に割り切れるため originX/Y は BOARD_ORIGIN_X/Y 固定 (中央寄せ補正不要)
typedef struct _ReversiBoard {
	int board[BOARD_SIZE][BOARD_SIZE];	//最大 12×12 ストレージ (0=空き / 1=黒 / 2=白)、有効領域は左上 size×size のみ
	int size;							//有効盤面サイズ 6/8/10/12 (1.5.8 で追加、rbInit で設定)
	int cellPx;							//有効セル幅 (1.5.9 で 120/90/72/60、BOARD_TARGET_PX / size、全サイズ綺麗に割り切れる)
	int originX;						//有効盤面 X 原点 (1.5.9 では BOARD_ORIGIN_X 固定、補正不要)
	int originY;						//有効盤面 Y 原点 (同上)
	std::string msg;					//メッセージ文字列 ("BLACK TURN" 等)
	int msg_wait;						//メッセージ表示残フレーム (60FPS で 1 フレーム = 1 カウント)
} ReversiBoard;

//盤面初期化 (ゼロクリア + 中央 4 駒配置 + メッセージリセット + サイズメトリクス設定)
//size は BOARD_SIZE_CHOICES の値 (6/8/10/12) を渡す、想定外値は 12 にフォールバック
void rbInit(ReversiBoard* state, int size);

//指定位置にコマを置く (put_flag=false ならシミュレーションのみ、戻り値: 裏返った数)
int rbPutPiece(ReversiBoard* state, int x, int y, int turn, bool put_flag);

//パスチェック (置けるマスが無ければ true)
bool rbIsPass(ReversiBoard* state, int turn);

//プレイヤー思考 (マウス入力、戻り値: コマを置いたら true)
bool rbThinkPlayer(ReversiBoard* state, int turn);

//CPU 思考 (最多取得マス選択、戻り値: 常に true)
bool rbThinkCpu(ReversiBoard* state, int turn);

//CPU 思考 弱 (置ける場所からランダム選択、Game3 あまちゃん次元用、戻り値: 置けたら true)
bool rbThinkRandom(ReversiBoard* state, int turn);

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
//1.5.8 で state を受け取り、動的サイズ (state->originX/Y/size/cellPx) で描画
void rbDrawBoard(ReversiBoard* state, int boardBgColor);

//縦横の格子線を描画 (色は ColorWhite 固定、1.5.8 で state ベースに動的化)
void rbDrawGrid(ReversiBoard* state);

//コマを描画 (pieces[0]=黒、pieces[1]=白 のハンドル配列)
void rbDrawPieces(ReversiBoard* state, int pieces[2]);

//ターン/パス/勝者メッセージ箱を描画 (status > GAME_STATUS_PLAYING の時のみ表示)
void rbDrawMsg(ReversiBoard* state, int status);

//右パネルの BLACK/WHITE カウントと優勢赤表示
void rbDrawCountPanel(ReversiBoard* state);

//誰のターンかを示す "← Now" インジケータを描画
void rbDrawTurnIndicator(int turn);

//置ける場所を半透明丸でハイライト (Game3 あまちゃん用、初心者向けヒント表示)
//プレイヤー手番中のみ呼び出す想定 (CPU 手番中は混乱を招くため非表示推奨)
//showGain=true でヒント丸の中に取得コマ数を白で重ね描き、false ならオレンジ丸のみ (1.5.7 でトグル化)
//1.5.8 で state->cellPx に追従 (丸半径 = cellPx/3、取得数フォントサイズ = cellPx*5/12)
//1.5.9 で BOARD_TARGET_PX=720 に拡張、フォントサイズ = 12→25 / 10→30 / 8→37 / 6→50 に自然スケール
void rbDrawHints(ReversiBoard* state, int turn, bool showGain);

//シーンを変更する関数
void changeScene(SCENE_NO no);

#endif