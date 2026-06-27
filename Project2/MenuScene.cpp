#include "GameMain.h"
#include "GameSceneMain.h"
#include "MenuScene.h"

//メニュー項目のシーン番号の配列 (1.5.7 で「オプション設定」を追加、MENU_MAX 3 → 4)
#define MENU_MAX 4
SCENE_NO menu[MENU_MAX] = { SCENE_GAME1, SCENE_GAME3, SCENE_GAME2, SCENE_OPTIONS };
char* menuList[MENU_MAX] = { "ふつうの次元：頭をほどよく使う", "あまちゃん次元：頭をあまり使わない",  "まきもどり次元：頭をかなり使う", "オプション設定" };
//選択されたゲームを表すメニュー番号の初期化（menuの添え字）
static int selectedGame = 0;

//外部定義(GameMain.cppにて宣言)
extern int Input, EdgeInput;

//シーン開始前の初期化を行う
BOOL initMenuScene(void)
{
	SetFontSize(FONT_SIZE_DEFAULT);
	ChangeFontType(DX_FONTTYPE_ANTIALIASING_EDGE_8X8);

	selectedGame = 0;

	//無論ここにもBGMを。
	PlaySoundFile("res/ambnience_jazz_22.mp3", DX_PLAYTYPE_LOOP);

	return TRUE;
}

//フレーム処理
void moveMenuScene()
{
	//７メニュー項目の選択
	//７(1) ①新たに↑が押されたら、
	if ((EdgeInput & PAD_INPUT_UP))
	{
		//７(1) ②１つ上のメニュー項目が選択されたとする。
		//      　ただし、それより上のメニュー項目がないときは、最下段のメニュー項目が選択されたとする
		if (--selectedGame < 0)
		{
			selectedGame = MENU_MAX - 1;
		}
	}

	//７(2) ①新たに↓が押されたら、
	if ((EdgeInput & PAD_INPUT_DOWN))
	{
		//７(2) ②１つ下のメニュー項目が選択されたとする。。
		//      　ただし、それより下のメニュー項目がないときは、最上段のメニュー項目が選択されたとする
		if (++selectedGame >= MENU_MAX)
		{
			selectedGame = 0;
		}
	}

	//７(3) 新たにボタン１が押されたら選択されているシーンへ
	if ((EdgeInput & PAD_INPUT_1))
	{
		changeScene(menu[selectedGame]);
	}
}

//レンダリング処理 (1.5.9 で 1280×768 用に再配置: タイトル / メニュー項目 / Credits)
void renderMenuScene(void)
{
	ChangeFont("ＭＳ 明朝");
	//タイトル: フォント 40、中央上寄り
	DrawString(280, 60, "まきもどリバーシ Ver 1.5.9.2", GetColor(255, 255, 255));

	//６(2) メニュー項目の表示 (1.5.9 で開始 y=170、gapY=90 で 4 項目を 440 まで)
	int x = 260, y = 170, gapY = 90;	//（x,y)：表示開始座標　gapY：行の高さ
	for (int i = 0; i < MENU_MAX; i++, y += gapY)
	{
		//６(2) ①選択された項目の表示
		if (i == selectedGame)
		{
			DrawString(x, y, menuList[i], GetColor(255, 0, 0));
			//６(2) ②選択されていない項目の表示
		}
		else
		{
			DrawString(x, y, menuList[i], GetColor(255, 255, 255));
		}
	}

	//Credits: 画面右下に配置、フォント 22 で 5 行 (5×22=110、y=580 から 690 まで、768 内に収まる)
	//1.5.9.2 でフォント 28 → 22 に下げて URL 末尾 (".com/") の見切れを解消 (1280×768 で右端まで 460px、URL 31 文字を収める)
	int oldFontSize = GetFontSize();
	SetFontSize(22);
	DrawString(820, 580, "Made with DX-Library 3.24f\n\nBGM: hitoshi & ambnience\nby Senses Circuit\nhttps://www.senses-circuit.com/",
		GetColor(255, 255, 255));
	SetFontSize(oldFontSize);
}

//シーン終了時の後処理
void releaseMenuScene(void)
{
}

//当り判定コールバック 　　　ここでは要素を削除しないこと！！
void  MenuSceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}