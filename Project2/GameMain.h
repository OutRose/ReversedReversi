#pragma once
//ゲームの初期化
#include "DxLib.h"
#include "GameStatus.h"

//MyOutputDebugString 用 (Debug ビルド時の OutputDebugString ラッパ)
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

//デバッグ文字列出力マクロ (Debug ビルドのみ実体化、Release は空展開)
//使用方法: MyOutputDebugString(_T("出力したい文字 %s"), 指定する対応変数);
//※_T は変数を混ぜたい場合に使用 (ワイド/マルチバイト両対応)
#ifdef _DEBUG
#   define MyOutputDebugString( str, ... ) \
      { \
        TCHAR c[256]; \
        _stprintf_s( c, sizeof(c) / sizeof(TCHAR), str, __VA_ARGS__ ); \
        OutputDebugString( c ); \
      }
#else
#    define MyOutputDebugString( str, ... ) // 空実装
#endif

//色変数 (定義は GameMain.cpp、全シーン共通)
extern unsigned int ColorWhite, ColorRed, ColorSky;

//ランク章用 (1.5.6.1 で追加、B2 ランクシステム伏線)
extern unsigned int ColorBronze, ColorSilver, ColorGold, ColorPlatinum;

//汎用 UI 用 (1.5.6.1 で追加、B1 オプショントグル + テーマ化伏線)
extern unsigned int ColorWarn, ColorOverlay, ColorHover;

//ランク章用 拡張 6 色 (1.5.9.1 で追加、B2 ランクシステム 10 ティア対応)
extern unsigned int ColorIron, ColorDiamond, ColorEmerald, ColorRuby, ColorSapphire, ColorAmethyst;

//汎用 UI 拡張 4 色 (1.5.9.1 で追加、テーマ化伏線)
extern unsigned int ColorSuccess, ColorError, ColorInfo, ColorAccent;

//オプション設定の永続化 (1.5.7 で導入、Game3 専用)
//loadOptions: WinMain で InitGame() 直後に 1 回呼ぶ、settings.ini からの読込み
//saveOptions: OptionsScene でトグル変更ごとに呼ぶ、settings.ini への書込み
//1.6.0 で B2 プレイヤーランクシステムの永続化 (totalXp/currentTier/totalGames/totalWins) も同関数経由に統合
void loadOptions(void);
void saveOptions(void);

//B2 プレイヤーランクシステム (1.6.0 で導入)
//PlayerStats: 全モード共通の累積統計、settings.ini に 4 キー (totalXp/currentTier/totalGames/totalWins) で永続化
typedef struct _PlayerStats {
	int totalXp;        //累積 XP (0〜MAX_XP、降格圧力で減算もあり 0 下限ガード)
	int currentTier;    //現在ティア (0=NOVICE 〜 9=ETERNAL、TIER_COUNT-1 上限)
	int totalGames;     //累計対局数 (Game1/2/3 共通)
	int totalWins;      //累計勝利数 (引分は勝利に含めない)
} PlayerStats;
extern PlayerStats g_playerStats;

//ランクシステム関連定数 (1.6.0 で追加)
#define TIER_COUNT  10                  //10 段ティア
#define MAX_XP      99999               //永続化の上限ガード (sscanf_s 検証用)
#define RANK_UP_DURATION_FRAMES   240   //ランクアップ演出 4 秒
#define DEMOTE_DURATION_FRAMES    120   //降格演出 2 秒
#define DEMOTE_BUFFER_XP          50    //降格バッファ (閾値 -50 XP を下回ったら降格)
#define MODE_GAME1  1                   //ふつう次元
#define MODE_GAME2  2                   //まきもどり次元 (2 ラウンド制)
#define MODE_GAME3  3                   //あまちゃん次元

//ティアテーブル (定義は GameMain.cpp、全シーン共通参照)
extern const char* TIER_NAMES[TIER_COUNT];           //"NOVICE" .. "ETERNAL"
extern const int   TIER_THRESHOLDS[TIER_COUNT];      //各ティア到達 XP 閾値
extern const unsigned int TIER_COLORS[TIER_COUNT];   //各ティア色 (ColorIron .. ColorAmethyst)

//ランクアップ SE 専用ハンドル (1.6.1 で追加、BGM と別チャンネルで SE 再生して BGM 中断を防止)
//WinMain で LoadSoundMem("res/loop_68.wav") → 各 Game シーンの RANK_UP 突入時に PlaySoundMem
//GameRelease で DeleteSoundMem、-1 = 未ロード/ロード失敗 (呼出側で != -1 ガード必須)
extern int g_rankUpSeHandle;

//画面設定 (WinMain で SetGraphMode に渡す)
//1.5.9 で 800×700 → 1280×768 に拡張 (1.6× / 1.097×、Windows DPI 125% 1080p ディスプレイでも収まる)
//Y 方向は控えめ拡張 (タイトルバー + タスクバー差し引いた可視領域に収まる優先)
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 768
#define SCREEN_BPP    16            //ReversedReversi は 16bit (TwistTimeStopper は 24)

//FPS関連 (フレーム周期管理、メインループ + 各シーンの時間換算で共通使用)
#define FPS         60               //60FPS固定
#define MS_PER_SEC  1000             //ミリ秒/秒

//盤面関連 (Game1Scene/Game2Scene/Game3Scene 共通、12×12 リバーシ盤面が最大)
//1.5.9 で 576px → 720px ターゲット領域に拡張、CELL_PX=60 (= 720/12)
//6/8/10/12 すべて綺麗に割り切れる (120/90/72/60) ため、1.5.8 の 10×10 中央寄せ補正が不要に
#define BOARD_SIZE        12         //12×12 マス (ストレージ最大次元、Game1/Game2 の固定サイズ)
#define BOARD_SIZE_MAX    11         //添字の最大値 (BOARD_SIZE - 1)
#define BOARD_TARGET_PX   720        //盤面描画ターゲット領域 (px) — state->cellPx = BOARD_TARGET_PX / size
#define CELL_PX           60         //セル 1 個のピクセル幅 (12×12 デフォルト時、720/12)
#define BOARD_ORIGIN_X    5          //盤面左上 X 座標
#define BOARD_ORIGIN_Y    5          //盤面左上 Y 座標 (1.5.9 768px 化に伴い 10 → 5 に詰める)
#define BOARD_END_PX      (BOARD_ORIGIN_X + BOARD_TARGET_PX)  //盤面終端ピクセル = 5 + 720 = 725
#define PIECE_SIZE_PX     47         //駒画像のソースサイズ (piece.png は 47×47)、表示は state->cellPx に DrawExtendGraph で拡大
#define BOARD_CENTER_LOW   5         //初期 4 駒の左上座標 (12×12 の場合、rbInit で動的計算)
#define BOARD_CENTER_HIGH  6         //初期 4 駒の右下座標 (12×12 の場合、rbInit で動的計算)

//描画レイアウト座標 (右パネル: 黒白カウント、ターン表示、ラウンド表示、終了メッセージ)
//1.5.9 で 1280×768 用に再配置、PANEL_X=745 (盤面右端 725 + 20px 余白)
#define PANEL_X            745       //右パネル X 位置 (BLACK/WHITE 等のラベルとカウント)
#define PANEL_BLACK_LABEL_Y 20
#define PANEL_BLACK_VALUE_Y 65
#define PANEL_WHITE_LABEL_Y 130
#define PANEL_WHITE_VALUE_Y 175
#define PANEL_TURN_X       900       //ターンインジケータ ("← Now") の X (PANEL_X + ラベル幅余裕)
#define PANEL_TURN_BLACK_Y 65
#define PANEL_TURN_WHITE_Y 175
#define PANEL_ROUND_LABEL_Y 240      //Game2 Round / Game3 PLAYER ラベル (1.5.9 で 290→240 に上げ、後続要素のスペース確保)
#define PANEL_ROUND_VALUE_X 925
#define PANEL_END_MSG_Y     410      //ESC で終了メッセージなどの Y (Game2/Game3、Game3 R:待ったガイド 350..390 と分離)
#define PANEL_END_MSG_GAME1_Y 240    //Game1 専用の終了メッセージ Y (1.5.9 で WHITE_VALUE 175..215 との 25px gap 確保、200→240)

//メッセージ表示 (盤面下のターン/PASS 通知、盤面の水平中心に揃える)
//盤面中心 X = BOARD_ORIGIN_X + BOARD_TARGET_PX/2 = 5 + 360 = 365
//1.5.9 768px 化で MSG_BOX を盤面下端 725 直下に詰める。msg は専用の小さめフォント MSG_FONT_SIZE で描画
#define MSG_BOX_CENTER_X    365      //メッセージ箱の水平中心 (1.5.9 で盤面拡張に追従)
#define MSG_BOX_Y_TOP       730      //盤面下端 725 + 5px 余白
#define MSG_BOX_Y_BOTTOM    762      //画面下端 768 から 6px 余白
#define MSG_BOX_PADDING_X   40
#define MSG_TEXT_Y          728      //文字上端、msg フォント 28 で下端 756
#define MSG_WAIT_FRAMES     60       //setMsg で msg_wait に設定する値 (60FPS で 1 秒)

//フォント設定 (MenuScene 初期化時のサイズ)
//1.5.9 で 32 → 40 (1.25× 拡張、画面解像度の拡張に伴う)
#define FONT_SIZE_DEFAULT   40
#define MSG_FONT_SIZE       28       //rbDrawMsg 専用の小さめフォント (768px 高に msg を収めるため)
#define HINT_GAIN_FONT_SIZE 24       //ヒントマス内の取得コマ数表示の固定上限値 (実際は state->cellPx*5/12 で動的算出)
