#include "GameMain.h"
#include "GameSceneMain.h"
#include "GameSceneTemplate.h"

//空シーン雛形 (1.5.8 で旧 Game4Scene から固定名にリネーム、ユーザー指示)
//本ファイルをコピーして新シーンを追加する。menu[] には載せない。

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

// シーン開始前の初期化を行う
BOOL initGameSceneTemplate(void)
{
	return TRUE;
}

//	フレーム処理
void moveGameSceneTemplate() {
	if ((EdgeInput & PAD_INPUT_1))
	{
		changeScene(SCENE_MENU);
	}
}

//	レンダリング処理 (1.5.9 で 1280×768 画面に合わせて余白を取った座標に調整)
void renderGameSceneTemplate(void)
{
	DrawString(60, 100, "Template Scene", GetColor(255, 255, 255));
	DrawString(60, 200, "ボタン１でタイトルに戻る", GetColor(255, 255, 255));
}

//	シーン終了時の後処理
void releaseGameSceneTemplate(void)
{
}

// 当り判定コールバック 　　　ここでは要素を削除しないこと！！
void  GameSceneTemplateCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}