#include "GameMain.h"
#include "GameSceneMain.h"
#include "OptionsScene.h"

//Game3 オプション設定画面 (1.5.7 で導入、1.5.8 で 5 行目「盤面サイズ」+ プレビュー追加)
//5 項目: ヒント表示 / 取得コマ数表示 / 弱い CPU / 待った機能 / 盤面サイズ
//1〜4 は ON/OFF トグル、5 は 4 段階サイズ循環 (6/8/10/12)
//変更時に即 settings.ini に保存 (saveOptions 呼出)、メニュー復帰は X キー

#define OPTION_COUNT 5	//1.5.8 で 4 → 5 (盤面サイズ追加)

//ファイルスコープ static (シーン入場時に init で初期化)
static int selected = 0;	//現在選択中のオプションインデックス 0..OPTION_COUNT-1

//外部定義 (GameMain.cpp にて宣言)
extern int Input, EdgeInput;

//BOARD_SIZE_CHOICES の中で current の次/前のサイズを返す (step=+1 で次、-1 で前)
//該当値が見つからなければ 12 (= index 3) を返す安全策
static int cycleBoardSize(int current, int step)
{
	int idx = BOARD_SIZE_CHOICE_COUNT - 1;	//デフォルトインデックス (= 12)
	for (int i = 0; i < BOARD_SIZE_CHOICE_COUNT; i++)
	{
		if (BOARD_SIZE_CHOICES[i] == current) { idx = i; break; }
	}
	idx = (idx + step + BOARD_SIZE_CHOICE_COUNT) % BOARD_SIZE_CHOICE_COUNT;
	return BOARD_SIZE_CHOICES[idx];
}

//盤面プレビューを OPTIONS 右側に描画 (320×320 領域、現在選択中のサイズの盤面 + 初期 4 駒)
//1.5.8 で導入 (320×320)、1.5.9 一時 500×500 → 768px 化で 320×320 (小ぶり) に再調整
//Game3 選択時の盤面イメージを直感的に確認できる
static void renderBoardPreview(int boardSize)
{
	//プレビュー領域 (右側、320×320、1.5.9 最終で位置 (810, 200))
	const int previewX  = 810;
	const int previewY  = 200;
	const int previewSz = 320;

	//ラベル (領域上部)
	SetFontSize(22);
	SetFontThickness(3);
	DrawString(previewX, previewY - 32, "プレビュー (現在のサイズ)", ColorWhite);

	//セル幅 = 320 / size (端数は余白として中央寄せ)
	int cellPx     = previewSz / boardSize;
	int totalPx    = cellPx * boardSize;
	int marginX    = (previewSz - totalPx) / 2;
	int marginY    = (previewSz - totalPx) / 2;
	int boardLeft  = previewX + marginX;
	int boardTop   = previewY + marginY;
	int boardRight = boardLeft + totalPx;
	int boardBot   = boardTop  + totalPx;

	//盤面背景 (暗緑) + 枠線
	DrawBox(boardLeft, boardTop, boardRight, boardBot, GetColor(0, 100, 20), TRUE);
	DrawLine(boardLeft,  boardTop, boardRight, boardTop, ColorWhite);
	DrawLine(boardLeft,  boardTop, boardLeft,  boardBot, ColorWhite);
	DrawLine(boardRight, boardTop, boardRight, boardBot, ColorWhite);
	DrawLine(boardLeft,  boardBot, boardRight, boardBot, ColorWhite);

	//グリッド線 (内側、size-1 本ずつ)
	for (int i = 1; i < boardSize; i++)
	{
		int gx = boardLeft + i * cellPx;
		int gy = boardTop  + i * cellPx;
		DrawLine(gx, boardTop,  gx, boardBot, ColorWhite);
		DrawLine(boardLeft, gy, boardRight, gy, ColorWhite);
	}

	//初期 4 駒 (中央 2×2 の対角配置、piece.png は使わず単純化、DrawCircle)
	int centerLow  = boardSize / 2 - 1;
	int centerHigh = boardSize / 2;
	int radius     = cellPx / 2 - 2;
	int blackColor = GetColor(0, 0, 0);
	int whiteColor = GetColor(255, 255, 255);
	struct { int bx; int by; int color; } pieces[4] = {
		{ centerLow,  centerLow,  blackColor },
		{ centerHigh, centerHigh, blackColor },
		{ centerHigh, centerLow,  whiteColor },
		{ centerLow,  centerHigh, whiteColor },
	};
	for (int i = 0; i < 4; i++)
	{
		int cx = boardLeft + pieces[i].bx * cellPx + cellPx / 2;
		int cy = boardTop  + pieces[i].by * cellPx + cellPx / 2;
		DrawCircle(cx, cy, radius, pieces[i].color, TRUE);
	}
}

//シーン開始前の初期化を行う
BOOL initOptionsScene(void)
{
	selected = 0;	//常に先頭から
	ChangeFont("ＭＳ 明朝");
	return TRUE;
}

//フレーム処理 (キー入力でカーソル移動 / トグル反転 / サイズ変更 / メニュー復帰)
void moveOptionsScene()
{
	//上下キーで選択項目移動 (リング状)
	if (EdgeInput & PAD_INPUT_UP)   selected = (selected + OPTION_COUNT - 1) % OPTION_COUNT;
	if (EdgeInput & PAD_INPUT_DOWN) selected = (selected + 1) % OPTION_COUNT;

	//Enter / Space / ボタン 1 で選択中項目を変更 + settings.ini に即保存
	//1〜4 行目: ON/OFF トグル反転、5 行目 (盤面サイズ): 次のサイズへ循環
	if (EdgeInput & PAD_INPUT_1)
	{
		if (selected < 4)
		{
			bool* targets[4] = {
				&g_game3Options.showHints,
				&g_game3Options.showGain,
				&g_game3Options.weakCpu,
				&g_game3Options.allowUndo,
			};
			*targets[selected] = !*targets[selected];
		}
		else
		{
			//盤面サイズ行: 次のサイズへ (12 → 10 → 8 → 6 → 12 → ...)
			g_game3Options.boardSize = cycleBoardSize(g_game3Options.boardSize, -1);
		}
		saveOptions();
	}

	//左右キーで盤面サイズ変更 (5 行目選択時のみ有効、1.5.8 で導入)
	//左 = 前のサイズ (12 → 6 → 8 → 10 → 12)、右 = 次のサイズ (12 → 10 → 8 → 6 → 12)
	if (selected == 4)
	{
		if (EdgeInput & PAD_INPUT_LEFT)
		{
			g_game3Options.boardSize = cycleBoardSize(g_game3Options.boardSize, +1);
			saveOptions();
		}
		if (EdgeInput & PAD_INPUT_RIGHT)
		{
			g_game3Options.boardSize = cycleBoardSize(g_game3Options.boardSize, -1);
			saveOptions();
		}
	}

	//X キーでメニュー復帰
	if (CheckHitKey(KEY_INPUT_X) == 1) changeScene(SCENE_MENU);
}

//レンダリング処理 (タイトル + 5 項目 + プレビュー + 操作ガイド)
//1.5.9 で 1280×768 用に再配置、フォント拡張 + プレビュー 320×320
void renderOptionsScene(void)
{
	//タイトル (1.5.9 でフォント 50 / 位置 (180,40) に拡張、768px 化で y を 60→40 に詰め)
	SetFontSize(50);
	SetFontThickness(9);
	DrawString(180, 40, "OPTIONS (Game3)", ColorWhite);

	//項目一覧 (1.5.9 で開始 y=140 / gapY=70 で 5 項目を 460 まで)
	SetFontSize(FONT_SIZE_DEFAULT);
	SetFontThickness(5);

	const char* labels[OPTION_COUNT] = {
		"ヒント表示       ",
		"取得コマ数表示   ",
		"弱いCPU         ",
		"待った機能      ",
		"盤面サイズ       ",	//1.5.8 で追加
	};

	int x = 130, y = 140, gapY = 70;
	for (int i = 0; i < OPTION_COUNT; i++, y += gapY)
	{
		//選択中は赤、それ以外は白 (MenuScene と同じ慣習)
		unsigned int color = (i == selected) ? ColorRed : ColorWhite;
		if (i < 4)
		{
			//1〜4 行目: ON/OFF 表示
			bool values[4] = {
				g_game3Options.showHints,
				g_game3Options.showGain,
				g_game3Options.weakCpu,
				g_game3Options.allowUndo,
			};
			DrawFormatString(x, y, color, "%s%s", labels[i], values[i] ? "[ ON ]" : "[ OFF ]");
		}
		else
		{
			//5 行目: 現在の盤面サイズ表示
			DrawFormatString(x, y, color, "%s%d×%d", labels[i], g_game3Options.boardSize, g_game3Options.boardSize);
		}
	}

	//盤面プレビュー (右側、現在選択中のサイズ反映、1.5.8 で導入、1.5.9 最終で 320×320)
	renderBoardPreview(g_game3Options.boardSize);

	//操作ガイド (1.5.8 で 3 行 → 4 行に拡張、768px 化で y=570, フォント 28 で 4×28=112 ends 682)
	int oldFontSize = GetFontSize();
	SetFontSize(28);
	DrawString(130, 570, "↑↓: 選択\nEnter/Space: ON/OFF・サイズ循環\n←→: サイズ変更 (5 行目)\nX: 戻る", ColorSky);
	SetFontSize(oldFontSize);
}

//シーン終了時の後処理 (リソースなし、何もしない)
void releaseOptionsScene(void)
{
}

//当たり判定コールバック (要素削除禁止)
void OptionsSceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}
