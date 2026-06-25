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

//シーンを変更する関数
void changeScene(SCENE_NO no);

#endif