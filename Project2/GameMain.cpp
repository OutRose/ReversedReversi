//メイン関数
#include "GameMain.h"
#include "GameSceneMain.h"

//グローバル変数
int FrameStartTime;        // ６０ＦＰＳ固定用、時間保存用変数

//ネームエントリー変数：プレイヤー側
char nameTmp[12];

//入力状態 Input:押しっぱなし
int Input;
//入力状態 EdgeInput:1回のみ
int EdgeInput;

//色変数 (GameMain.h で extern 宣言、全シーン共通)
//注意: GetColor() の戻り値は SetGraphMode 設定後の bit 深度に依存する。
//      ファイルスコープでの動的初期化は SetGraphMode 前に走るが、現状の RGB 値ならば
//      bit 深度に依らず一意の結果を返すので動作不変 (16/24/32 bit 全てで同じ)。
unsigned int ColorWhite = GetColor(255, 255, 255);
unsigned int ColorRed   = GetColor(255, 0, 0);
unsigned int ColorSky   = GetColor(40, 235, 255);

//ランク章用 4 色 (1.5.6.1 で追加、B2 プレイヤーランクシステム伏線、CSS/Material/メダル色配色に準拠)
//Bronze=銅、Silver=銀、Gold=金、Platinum=白に近い淡銀 (銀と差別化)
unsigned int ColorBronze    = GetColor(205, 127, 50);
unsigned int ColorSilver    = GetColor(192, 192, 192);
unsigned int ColorGold      = GetColor(255, 215, 0);
unsigned int ColorPlatinum  = GetColor(229, 228, 226);

//汎用 UI 色 (1.5.6.1 で追加、B1 オプショントグル + テーマ化伏線)
unsigned int ColorWarn      = GetColor(255, 235, 0);    //警告系黄色 (純黄より少しオレンジ寄りで視認性高)
unsigned int ColorOverlay   = GetColor(128, 128, 128);  //半透明オーバーレイ用中間グレー (DX_BLENDMODE_ALPHA 128 想定)
unsigned int ColorHover     = GetColor(180, 220, 255);  //ホバー強調 (ColorSky より淡い水色、選択中赤と区別)

//Game3 オプション設定 (1.5.7 で導入、デフォルト全 ON、settings.ini から起動時に上書き)
ReversiOptions g_game3Options = { true, true, true, true };

//settings.ini からオプション読込 (WinMain で InitGame() 後に呼ぶ、ファイル無ければデフォルト維持)
void loadOptions(void)
{
	FILE* fp = nullptr;
	if (fopen_s(&fp, "settings.ini", "r") != 0 || !fp) return;	//ファイル無し → デフォルト維持
	char line[128];
	while (fgets(line, sizeof(line), fp))
	{
		int v;
		if (sscanf_s(line, "showHints=%d", &v) == 1) g_game3Options.showHints = (v != 0);
		else if (sscanf_s(line, "showGain=%d",  &v) == 1) g_game3Options.showGain  = (v != 0);
		else if (sscanf_s(line, "weakCpu=%d",   &v) == 1) g_game3Options.weakCpu   = (v != 0);
		else if (sscanf_s(line, "allowUndo=%d", &v) == 1) g_game3Options.allowUndo = (v != 0);
	}
	fclose(fp);
}

//settings.ini にオプション書込 (OptionsScene でトグル変更ごとに呼ぶ)
void saveOptions(void)
{
	FILE* fp = nullptr;
	if (fopen_s(&fp, "settings.ini", "w") != 0 || !fp) return;
	fprintf(fp, "# まきもどリバーシ Game3 オプション設定 (1.5.7+)\n");
	fprintf(fp, "showHints=%d\n", g_game3Options.showHints  ? 1 : 0);
	fprintf(fp, "showGain=%d\n",  g_game3Options.showGain   ? 1 : 0);
	fprintf(fp, "weakCpu=%d\n",   g_game3Options.weakCpu    ? 1 : 0);
	fprintf(fp, "allowUndo=%d\n", g_game3Options.allowUndo  ? 1 : 0);
	fclose(fp);
}

//WinMain関数
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_  LPSTR lpCmdLine, _In_ int nCmdShow)
{
	SetOutApplicationLogValidFlag(true);

	//ウィンドウの設定
	//ウィンドウモードの設定 false:全画面設定
	ChangeWindowMode(true);

	//ウィンドウのリサイズ
	//Check:実行中に画面の大きさが変更可能か
	SetWindowSizeChangeEnableFlag(true);

	//ウィンドウサイズをセット
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

	//ウィンドウタイトル
	SetMainWindowText("Reverse Reversi 1.5.7");

	//背景色の設定
	SetBackgroundColor(0, 0, 0);
	//DXライブラリ初期化処理
	//check:-1(例外処理)が来た場合は、セットアップに失敗する
	//パソコン本体の機器の確認をしてもらう(音声デバイスがない等。)
	if (DxLib_Init() == -1)return -1;

	//描画先を一番後ろにする
	SetDrawScreen(DX_SCREEN_BACK);

	//音楽再生の初期化
	InitSoundMem();	//Dxlib

	// ゲームループ前初期化
	InitGame();

	// オプション設定読込 (settings.ini から、1.5.7 で導入)
	loadOptions();

	//メインループ
	while (1)
	{
		// 画面に描かれているものを一回全部消す
		ClearDrawScreen();

		// １/６０秒立つまで待つ
		while (GetNowCount() - FrameStartTime < MS_PER_SEC / FPS) {}
		// 現在のカウント値を保存
		FrameStartTime = GetNowCount();
		// 入力状態を更新
		{
			int i;
			// パッド１とキーボードから入力を得る
			i = GetJoypadInputState(DX_INPUT_KEY_PAD1);
			// エッジを取った入力をセット
			EdgeInput = i & ~Input;
			// 入力状態の保存
			Input = i;
		}

		//ゲームの更新処理
		FrameMove();
		//ゲームの描画処理
		RenderScene();

		//画面の表示順を一番手前から反映する
		ScreenFlip();

		//windowsシステムから来る情報を処理する(例外処理等)
		if (ProcessMessage() == -1)break;

		//ESCキーが押されたらループから抜ける
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)break;
	}

	//Dxlibの開放
	DxLib_End();

	// ゲーム終了時の初期化
	GameRelease();

	//ゲームを終了する
	return 0;
}