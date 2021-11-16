#ifndef __GAMESCENEMAIN_H_
#define __GAMESCENEMAIN_H_

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
	SCENE_GAME1,		// ゲーム１シーンの番号
	SCENE_GAME2,		// ゲーム２シーンの番号
	SCENE_GAME3,		// ゲーム３シーンの番号
	SCENE_GAME4,		// ゲーム４シーンの番号
	SCENE_GAME5,		// ゲーム５シーンの番号
	SCENE_MAX			// シーン番号の上限。必ず書く
} SCENE_NO;

//シーンを変更する関数
void changeScene(SCENE_NO no);

#endif