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

//ランク章用 拡張 6 色 (1.5.9.1 で追加、B2 プレイヤーランクシステム 10 ティア対応の余裕枠)
//Iron=Bronze 下位エントリー、Diamond/Emerald/Ruby/Sapphire/Amethyst=Platinum 上位の宝石・称号系
unsigned int ColorIron      = GetColor(110, 120, 140);  //鋼鉄グレー (Bronze 下位のエントリーティア、背景黒上で読める明度)
unsigned int ColorDiamond   = GetColor(130, 240, 255);  //シアン水色 (Sky 純シアン/Hover 淡水色と段階分離)
unsigned int ColorEmerald   = GetColor(50, 200, 130);   //ミントグリーン寄り (盤面色より明るく、ColorSuccess 純緑と用途分離)
unsigned int ColorRuby      = GetColor(224, 50, 95);    //深紅ピンク寄り (ColorRed=純赤=「選択中」用と区別)
unsigned int ColorSapphire  = GetColor(40, 90, 200);    //深サファイア青 (Sky シアン/Info 明るい青と段階分離)
unsigned int ColorAmethyst  = GetColor(170, 100, 200);  //アメジスト紫 (既存色に紫系なし、唯一性高)

//汎用 UI 拡張 4 色 (1.5.9.1 で追加、テーマ化伏線、状態通知の用途分離)
unsigned int ColorSuccess   = GetColor(40, 180, 60);    //成功・確定 (Emerald と用途分離、純緑寄り)
unsigned int ColorError     = GetColor(255, 80, 80);    //エラー警報 (ColorRed=純赤は「選択中」用、用途分離)
unsigned int ColorInfo      = GetColor(120, 180, 240);  //情報通知 (Sapphire 深青/Hover 淡水色と段階分離)
unsigned int ColorAccent    = GetColor(255, 130, 200);  //汎用アクセント (マゼンタ寄り、既存にピンク系なし)

//Game3 オプション設定 (1.5.7 で導入、デフォルト 4 トグル全 ON + boardSize=12、settings.ini から起動時に上書き)
//1.5.8 で boardSize 追加 (6/8/10/12 の 4 段階、初期 12 = 現状互換)
ReversiOptions g_game3Options = { true, true, true, true, 12 };

//B2 プレイヤーランクシステム (1.6.0 で導入、全モード共通)
//初期値: NOVICE / XP 0 / 0 試合 / 0 勝、settings.ini に新 4 キーが無ければこの値で起動 (後方互換)
PlayerStats g_playerStats = { 0, 0, 0, 0 };

//1.6.1 で導入したランクアップ SE 専用ハンドル定義 (g_rankUpSeHandle) は 1.6.3 で撤去。
//詳細経緯は GameMain.h のコメント参照

//ティアテーブル (10 段、1.6.0 で導入)
//名前は冒険感のある英語 NOVICE → ETERNAL の 10 段、Game3 のプレイヤー名 nameTmp とは独立した実力称号
const char* TIER_NAMES[TIER_COUNT] = {
	"NOVICE", "APPRENTICE", "ADEPT", "EXPERT", "MASTER",
	"GRANDMASTER", "SAGE", "LEGEND", "MYTHIC", "ETERNAL"
};
//各ティアに到達するための累積 XP 閾値 (Game1 標準勝利 30〜50 XP なら ETERNAL 到達は約 230 勝)
const int TIER_THRESHOLDS[TIER_COUNT] = {
	0, 50, 150, 350, 700, 1200, 2000, 3200, 5000, 8000
};
//各ティアの色 (1.5.9.1 で追加した 10 ランク色を順番に割当)
//注: ファイルスコープ初期化は上から下、ColorXxx は本 TU で先に初期化済みなので参照可能
const unsigned int TIER_COLORS[TIER_COUNT] = {
	ColorIron,      //0 NOVICE
	ColorBronze,    //1 APPRENTICE
	ColorSilver,    //2 ADEPT
	ColorGold,      //3 EXPERT
	ColorPlatinum,  //4 MASTER
	ColorDiamond,   //5 GRANDMASTER
	ColorEmerald,   //6 SAGE
	ColorRuby,      //7 LEGEND
	ColorSapphire,  //8 MYTHIC
	ColorAmethyst   //9 ETERNAL
};

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
		else if (sscanf_s(line, "boardSize=%d", &v) == 1)
		{
			//1.5.8 で導入。有効値 (6/8/10/12) のみ受理、他はデフォルト 12 維持
			if (v == 6 || v == 8 || v == 10 || v == 12) g_game3Options.boardSize = v;
		}
		//B2 ランクシステム (1.6.0 で導入)、不在時は g_playerStats のデフォルト初期値 (全 0、NOVICE) を維持
		else if (sscanf_s(line, "totalXp=%d", &v) == 1)
		{
			if (v >= 0 && v <= MAX_XP) g_playerStats.totalXp = v;
		}
		else if (sscanf_s(line, "currentTier=%d", &v) == 1)
		{
			if (v >= 0 && v < TIER_COUNT) g_playerStats.currentTier = v;
		}
		else if (sscanf_s(line, "totalGames=%d", &v) == 1)
		{
			if (v >= 0) g_playerStats.totalGames = v;
		}
		else if (sscanf_s(line, "totalWins=%d", &v) == 1)
		{
			if (v >= 0) g_playerStats.totalWins = v;
		}
	}
	fclose(fp);
}

//settings.ini にオプション書込 (OptionsScene でトグル変更ごとに呼ぶ)
void saveOptions(void)
{
	FILE* fp = nullptr;
	if (fopen_s(&fp, "settings.ini", "w") != 0 || !fp) return;
	fprintf(fp, "# まきもどリバーシ オプション設定 (1.5.7+、1.5.8 で boardSize 追加、1.6.0 で B2 ランク 4 キー追加)\n");
	fprintf(fp, "showHints=%d\n", g_game3Options.showHints  ? 1 : 0);
	fprintf(fp, "showGain=%d\n",  g_game3Options.showGain   ? 1 : 0);
	fprintf(fp, "weakCpu=%d\n",   g_game3Options.weakCpu    ? 1 : 0);
	fprintf(fp, "allowUndo=%d\n", g_game3Options.allowUndo  ? 1 : 0);
	fprintf(fp, "boardSize=%d\n", g_game3Options.boardSize);
	//B2 ランクシステム (1.6.0)
	fprintf(fp, "totalXp=%d\n",      g_playerStats.totalXp);
	fprintf(fp, "currentTier=%d\n",  g_playerStats.currentTier);
	fprintf(fp, "totalGames=%d\n",   g_playerStats.totalGames);
	fprintf(fp, "totalWins=%d\n",    g_playerStats.totalWins);
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
	SetMainWindowText("Reverse Reversi 1.6.3");

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

	//1.6.3: ランクアップ SE のロード (LoadSoundMem) は撤去 (詳細経緯は GameMain.h のコメント参照)
	//将来専用 SE wav を追加する場合は本コメント位置に LoadSoundMem を復活

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