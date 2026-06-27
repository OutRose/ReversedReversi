#include "GameMain.h"
#include "GameSceneMain.h"
#include "OptionsScene.h"

//Game3 オプション設定画面 (1.5.7 で導入)
//4 つのトグル: ヒント表示 / 取得コマ数表示 / 弱い CPU / 待った機能
//変更時に即 settings.ini に保存 (saveOptions 呼出)、メニュー復帰は X キー

#define OPTION_COUNT 4

//ファイルスコープ static (シーン入場時に init で初期化)
static int selected = 0;	//現在選択中のオプションインデックス 0..OPTION_COUNT-1

//外部定義 (GameMain.cpp にて宣言)
extern int Input, EdgeInput;

//シーン開始前の初期化を行う
BOOL initOptionsScene(void)
{
	selected = 0;	//常に先頭から
	ChangeFont("ＭＳ 明朝");
	return TRUE;
}

//フレーム処理 (キー入力でカーソル移動 / トグル反転 / メニュー復帰)
void moveOptionsScene()
{
	//上下キーで選択項目移動 (リング状)
	if (EdgeInput & PAD_INPUT_UP)   selected = (selected + OPTION_COUNT - 1) % OPTION_COUNT;
	if (EdgeInput & PAD_INPUT_DOWN) selected = (selected + 1) % OPTION_COUNT;

	//Enter / Space / ボタン 1 で選択中トグルを反転 + settings.ini に即保存
	if (EdgeInput & PAD_INPUT_1)
	{
		bool* targets[OPTION_COUNT] = {
			&g_game3Options.showHints,
			&g_game3Options.showGain,
			&g_game3Options.weakCpu,
			&g_game3Options.allowUndo,
		};
		*targets[selected] = !*targets[selected];
		saveOptions();
	}

	//X キーでメニュー復帰
	if (CheckHitKey(KEY_INPUT_X) == 1) changeScene(SCENE_MENU);
}

//レンダリング処理 (タイトル + 4 トグル + 操作ガイド)
void renderOptionsScene(void)
{
	//タイトル
	SetFontSize(45);
	SetFontThickness(9);
	DrawString(220, 50, "OPTIONS (Game3)", ColorWhite);

	//トグル一覧
	SetFontSize(FONT_SIZE_DEFAULT);
	SetFontThickness(5);

	const char* labels[OPTION_COUNT] = {
		"ヒント表示       ",
		"取得コマ数表示   ",
		"弱いCPU         ",
		"待った機能      ",
	};
	bool values[OPTION_COUNT] = {
		g_game3Options.showHints,
		g_game3Options.showGain,
		g_game3Options.weakCpu,
		g_game3Options.allowUndo,
	};

	int x = 130, y = 180, gapY = 60;
	for (int i = 0; i < OPTION_COUNT; i++, y += gapY)
	{
		//選択中は赤、それ以外は白 (MenuScene と同じ慣習)
		unsigned int color = (i == selected) ? ColorRed : ColorWhite;
		DrawFormatString(x, y, color, "%s%s", labels[i], values[i] ? "[ ON ]" : "[ OFF ]");
	}

	//操作ガイド
	DrawString(130, 500, "↑↓: 選択\nEnter/Space: 切替\nX: メニューに戻る", ColorSky);
}

//シーン終了時の後処理 (リソースなし、何もしない)
void releaseOptionsScene(void)
{
}

//当たり判定コールバック (要素削除禁止)
void OptionsSceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}
