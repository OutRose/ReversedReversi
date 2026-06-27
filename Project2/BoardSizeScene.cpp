#include "GameMain.h"
#include "GameSceneMain.h"
#include "BoardSizeScene.h"

//盤面サイズ設定シーン (1.6.1 で導入、項目 6(a))
//OPTIONS の 5 行目「盤面サイズ設定 →」から Enter で起動、本シーンで ←→/Enter で循環、X で OPTIONS 復帰
//1.5.8 で OptionsScene に組み込んでいた renderBoardPreview + cycleBoardSize を本ファイルへ移植

//外部定義 (GameMain.cpp にて宣言)
extern int Input, EdgeInput;

//BOARD_SIZE_CHOICES の中で current の次/前のサイズを返す (step=+1 で次、-1 で前)
//該当値が見つからなければ 12 (= index 3) を返す安全策。1.6.1 で OptionsScene から移植
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

//盤面プレビューを右側に描画 (320×320 領域、現在選択中のサイズの盤面 + 初期 4 駒)
//1.5.8 で OptionsScene に導入、1.6.1 で本シーンへ移植
static void renderBoardPreview(int boardSize)
{
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

//シーン開始前の初期化 (盤面サイズ自体は g_game3Options.boardSize を共有、リセット不要)
BOOL initBoardSizeScene(void)
{
	ChangeFont("ＭＳ 明朝");
	return TRUE;
}

//フレーム処理 (←→ サイズ循環 / Enter・Space で次のサイズへ / X キーで OPTIONS 復帰)
void moveBoardSizeScene()
{
	//←キー: 前のサイズ (12 → 6 → 8 → 10 → 12)、保存も即時
	if (EdgeInput & PAD_INPUT_LEFT)
	{
		g_game3Options.boardSize = cycleBoardSize(g_game3Options.boardSize, +1);
		saveOptions();
	}
	//→キー: 次のサイズ (12 → 10 → 8 → 6 → 12)、保存も即時
	if (EdgeInput & PAD_INPUT_RIGHT)
	{
		g_game3Options.boardSize = cycleBoardSize(g_game3Options.boardSize, -1);
		saveOptions();
	}
	//Enter / Space / ボタン 1: 次のサイズへ循環 (1.5.8 OPTIONS 時の互換動作を維持)
	if (EdgeInput & PAD_INPUT_1)
	{
		g_game3Options.boardSize = cycleBoardSize(g_game3Options.boardSize, -1);
		saveOptions();
	}
	//X キー: OPTIONS シーンへ復帰 (本シーンは OPTIONS から呼ばれる前提でハードコード)
	//1.6.1 polish: isXKeyJustPressed でエッジ検出、押しっぱなしでの OPTIONS → MENU 連鎖を防止
	if (isXKeyJustPressed()) changeScene(SCENE_OPTIONS);
}

//レンダリング処理 (タイトル + 現在サイズ + プレビュー + 操作ガイド)
void renderBoardSizeScene(void)
{
	int sz = g_game3Options.boardSize;

	//タイトル (フォント 50、OptionsScene と同じスタイル)
	SetFontSize(50);
	SetFontThickness(9);
	DrawString(180, 40, "盤面サイズ設定", ColorWhite);

	//現在のサイズ表示 (フォント 40、左寄せ)
	SetFontSize(FONT_SIZE_DEFAULT);
	SetFontThickness(5);
	DrawFormatString(180, 140, ColorWhite, "現在のサイズ:");
	DrawFormatString(180, 200, ColorSky,    "%d × %d", sz, sz);

	//盤面プレビュー (右側、320×320、現在選択中のサイズ反映)
	renderBoardPreview(sz);

	//操作ガイド (画面下、フォント 28 で 4 行、768px 高に収める)
	SetFontSize(28);
	DrawString(180, 580, "←→: 前/次のサイズ\nEnter/Space: 次のサイズへ循環\nX: OPTIONS へ戻る", ColorSky);
}

//シーン終了時の後処理 (リソースなし)
void releaseBoardSizeScene(void)
{
}

//当たり判定コールバック (要素削除禁止)
void BoardSizeSceneCollideCallback(int nSrc, int nTarget, int nCollideID)
{
}