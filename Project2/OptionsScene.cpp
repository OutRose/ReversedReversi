#include "GameMain.h"
#include "GameSceneMain.h"
#include "OptionsScene.h"

//Game3 オプション設定画面 (1.5.7 で導入、1.5.8 で 5 行目「盤面サイズ」+ プレビュー追加)
//5 項目: ヒント表示 / 取得コマ数表示 / 弱い CPU / 待った機能 / 盤面サイズ
//1〜4 は ON/OFF トグル、5 は 4 段階サイズ循環 (6/8/10/12)
//変更時に即 settings.ini に保存 (saveOptions 呼出)、メニュー復帰は X キー

#define OPTION_COUNT 6	//1.5.8 で 4 → 5 (盤面サイズ追加)、1.6.0 で 5 → 6 (ランクをリセット追加)

//ファイルスコープ static (シーン入場時に init で初期化)
static int selected = 0;	//現在選択中のオプションインデックス 0..OPTION_COUNT-1

//1.6.0: ランクリセット確認用タイマー (180f 以内に Enter 2 回目で確定、それ以外で解除)
//Enter 1 回目で 180 セット、毎フレーム --、0 になるとリセット確認モード解除
static int resetConfirmTimer = 0;
static int resetCompleteTimer = 0;	//リセット完了メッセージ表示残フレーム (60f)

//外部定義 (GameMain.cpp にて宣言)
extern int Input, EdgeInput;

//1.6.1: 旧 cycleBoardSize / renderBoardPreview は BoardSizeScene.cpp へ移動済 (項目 6(a))
//OPTIONS は ON/OFF トグル列と「ランクをリセット」「盤面サイズ設定 →」のみ担当、プレビュー描画なし

//シーン開始前の初期化を行う
BOOL initOptionsScene(void)
{
	selected = 0;	//常に先頭から
	//1.6.0: リセット確認/完了タイマーもクリア (再入場時に前回状態が残らないように)
	resetConfirmTimer = 0;
	resetCompleteTimer = 0;
	ChangeFont("ＭＳ 明朝");
	return TRUE;
}

//フレーム処理 (キー入力でカーソル移動 / トグル反転 / サイズ変更 / ランクリセット / メニュー復帰)
void moveOptionsScene()
{
	//1.6.0: リセット確認/完了タイマーの減衰
	if (resetConfirmTimer > 0) resetConfirmTimer--;
	if (resetCompleteTimer > 0) resetCompleteTimer--;

	//上下キーで選択項目移動 (リング状)
	//1.6.0: 項目移動で確認モード解除 (誤操作防止)
	if (EdgeInput & PAD_INPUT_UP)   { selected = (selected + OPTION_COUNT - 1) % OPTION_COUNT; resetConfirmTimer = 0; }
	if (EdgeInput & PAD_INPUT_DOWN) { selected = (selected + 1) % OPTION_COUNT; resetConfirmTimer = 0; }

	//Enter / Space / ボタン 1 で選択中項目を変更 + settings.ini に即保存
	//1〜4 行目: ON/OFF トグル反転、5 行目 (盤面サイズ): 次のサイズへ循環、6 行目 (ランクリセット): 2 連打で確定 (1.6.0)
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
			saveOptions();
		}
		else if (selected == 4)
		{
			//1.6.1: 盤面サイズ設定行は専用シーンへ遷移 (←→ サイズ循環は BoardSizeScene 側で実施)
			changeScene(SCENE_BOARD_SIZE);
		}
		else if (selected == 5)
		{
			//1.6.0: ランクリセット行
			if (resetConfirmTimer > 0)
			{
				//2 回目 Enter (180f 以内) → リセット実行
				g_playerStats.totalXp     = 0;
				g_playerStats.currentTier = 0;
				g_playerStats.totalGames  = 0;
				g_playerStats.totalWins   = 0;
				saveOptions();
				resetConfirmTimer = 0;
				resetCompleteTimer = 60;	//「リセット完了」を 60f 表示
			}
			else
			{
				//1 回目 Enter → 確認モードに入る (180f = 3 秒)
				resetConfirmTimer = 180;
			}
		}
	}

	//1.6.1: ←→ キーの盤面サイズ循環は BoardSizeScene へ移管 (本シーンでは未使用)

	//X キーでメニュー復帰
	//1.6.1 polish: isXKeyJustPressed でエッジ検出、BoardSizeScene からの押しっぱなし連鎖を防止
	if (isXKeyJustPressed()) changeScene(SCENE_MENU);
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

	//1.6.1: labels[] の空白パディング撤去 (DrawString 分離方式でアラインメント実現、項目 6(b))
	const char* labels[OPTION_COUNT] = {
		"ヒント表示",
		"取得コマ数表示",
		"弱いCPU",
		"待った機能",
		"盤面サイズ設定",	//1.5.8 で追加、1.6.1 で「→」付きリンク化 (BoardSizeScene へ遷移)
		"ランクをリセット",	//1.6.0 で追加 (Enter 2 連打で確定、B2 ランクシステム)
	};

	//1.6.1: ラベル描画と値 (ON/OFF・サイズ・XP) 描画を別 DrawString に分離して TOGGLE_X=500 で揃え
	const int LABEL_X  = 130;
	const int TOGGLE_X = 500;
	int y = 140;
	const int gapY = 70;
	for (int i = 0; i < OPTION_COUNT; i++, y += gapY)
	{
		//選択中は赤、それ以外は白 (MenuScene と同じ慣習)
		unsigned int color = (i == selected) ? ColorRed : ColorWhite;
		//ラベル単独描画 (日本語のみ、幅は labels[i] の文字数に依存)
		DrawString(LABEL_X, y, labels[i], color);

		if (i < 4)
		{
			//1〜4 行目: 固定 X で ON/OFF (TOGGLE_X=500 揃え)
			bool values[4] = {
				g_game3Options.showHints,
				g_game3Options.showGain,
				g_game3Options.weakCpu,
				g_game3Options.allowUndo,
			};
			DrawString(TOGGLE_X, y, values[i] ? "[ ON ]" : "[ OFF ]", color);
		}
		else if (i == 4)
		{
			//5 行目: 現在サイズ + 「→」(BoardSizeScene への遷移リンク)
			DrawFormatString(TOGGLE_X, y, color, "%d×%d  →", g_game3Options.boardSize, g_game3Options.boardSize);
		}
		else if (i == 5)
		{
			//6 行目: ランクリセット (確認/完了モード中は表示変更、通常時はティア + XP)
			if (resetConfirmTimer > 0)
			{
				DrawString(TOGGLE_X, y, "もう一度 Enter", ColorWarn);
			}
			else if (resetCompleteTimer > 0)
			{
				DrawString(TOGGLE_X, y, "完了!", ColorSuccess);
			}
			else
			{
				DrawFormatString(TOGGLE_X, y, color, "[%s %d XP]",
					getTierName(g_playerStats.currentTier), g_playerStats.totalXp);
			}
		}
	}

	//1.6.1: 盤面プレビューは BoardSizeScene 側で描画 (本シーンからは撤去、項目 6(a))

	//操作ガイド (1.6.1 で 4 行 → 3 行に縮小、←→ サイズ変更は BoardSizeScene へ移管したため)
	//確認/完了モードの文言は 6 行目 TOGGLE_X 列でインライン表示済 (本ガイドは常時表示)
	int oldFontSize = GetFontSize();
	SetFontSize(28);
	DrawString(130, 570, "↑↓: 選択\nEnter/Space: 決定\nX: 戻る", ColorSky);
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
