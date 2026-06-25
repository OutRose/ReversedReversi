#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game4Scene.h"

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

// シーン開始前の初期化を行う
BOOL initGame4Scene(void)
{
	return TRUE;
}

//	フレーム処理
void moveGame4Scene() {
	if ((EdgeInput & PAD_INPUT_1))
	{
		changeScene(SCENE_MENU);
	}
}

//	レンダリング処理
void renderGame4Scene(void)
{
	DrawString(30, 50, "ゲーム画面４です", GetColor(255, 255, 255));
	DrawString(30, 100, "ボタン１でタイトルに戻る", GetColor(255, 255, 255));
}

//	シーン終了時の後処理
void releaseGame4Scene(void)
{
}

// 当り判定コールバック 　　　ここでは要素を削除しないこと！！
void  Game4SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}