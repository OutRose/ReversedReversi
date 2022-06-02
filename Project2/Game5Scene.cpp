#include "GameMain.h"
#include "GameSceneMain.h"
#include "Game5Scene.h"
#include <string>

//外部定義（GameMain.cppにて宣言）
extern int Input, EdgeInput;
extern char nameTmp[12];

//シーン開始前の初期化を行う
BOOL initGame5Scene(void)
{
	return TRUE;
}

//各種フレーム処理
void moveGame5Scene()
{
	ClearDrawScreen();

	//表題・指示を描画する
	ChangeFont("ＭＳ 明朝");

	SetFontSize(45);
	SetFontThickness(9);
	DrawString(280, 50, "NAME ENTRY", GetColor(255, 255, 255));

	SetFontSize(30);
	SetFontThickness(5);
	DrawString(160, 100, "アルファベットで名前を入力してください\n入力が終わったら、Enterキーを\n押してください", GetColor(255, 255, 255));

	//入力中の名前を表示する
	SetFontSize(40);
	SetFontThickness(6);
	DrawString(270, 280, nameTmp, GetColor(255, 255, 255));
	//名前下に線を表示（11文字分程度,Xの長さ=250）
	DrawLine(265, 330, 515, 330, GetColor(255, 255, 255));

	//入力を確認したら、Enterキーでゲームスタート
	//ワープさせたい。例えば：1=通常モード　4=まきもどりモード
	if (nameTmp[0] != NULL)
	{
		SetFontSize(30);
		DrawString(50, 370, "入力完了。\nEnterキーを押して対局開始！", GetColor(255, 255, 255));
		ScreenFlip();
		//問題はここよここ
		changeScene(SCENE_GAME4);
	}

	//入力を検知・代入
	KeyInputSingleCharString(270, 280, 32, nameTmp, FALSE);
}

//画面描画処理
void renderGame5Scene(void)
{
}

//シーン終了時の後処理
void releaseGame5Scene(void)
{
}

//当たり判定コールバック（要素削除禁止）
void Game5SceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}