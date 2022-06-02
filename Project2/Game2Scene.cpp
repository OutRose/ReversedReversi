#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game2Scene.h"

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

// シーン開始前の初期化を行う
BOOL initGame2Scene(void) 
{
	return TRUE;
}

//	フレーム処理
void moveGame2Scene() {
	if ((EdgeInput & PAD_INPUT_1)) 
	{
		changeScene(SCENE_MENU);
	}
}

//	レンダリング処理
void renderGame2Scene(void) 
{
	DrawString(30, 50, "ゲーム画面２です", GetColor(255, 255, 255));
	DrawString(30, 100, "ボタン１でタイトルに戻る", GetColor(255, 255, 255));
}

//	シーン終了時の後処理
void releaseGame2Scene(void) 
{

}

// 当り判定コールバック 　　　ここでは要素を削除しないこと！！
void  Game2SceneCollideCallback(int nSrc, int nTarget, int nCollideID) 
{

}