#ifndef GAMESCENEMAIN_H_
#define GAMESCENEMAIN_H_

#include "GameMain.h"

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

//シーンを変更する関数
void changeScene(SCENE_NO no);

#endif