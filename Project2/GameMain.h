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

//オプション設定の永続化 (1.5.7 で導入、Game3 専用)
//loadOptions: WinMain で InitGame() 直後に 1 回呼ぶ、settings.ini からの読込み
//saveOptions: OptionsScene でトグル変更ごとに呼ぶ、settings.ini への書込み
void loadOptions(void);
void saveOptions(void);

//画面設定 (WinMain で SetGraphMode に渡す)
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 700
#define SCREEN_BPP    16            //ReversedReversi は 16bit (TwistTimeStopper は 24)

//FPS関連 (フレーム周期管理、メインループ + 各シーンの時間換算で共通使用)
#define FPS         60               //60FPS固定
#define MS_PER_SEC  1000             //ミリ秒/秒

//盤面関連 (Game1Scene/Game2Scene 共通、12×12 リバーシ盤面)
#define BOARD_SIZE       12          //12×12 マス (盤面の縦横セル数)
#define BOARD_SIZE_MAX   11          //添字の最大値 (BOARD_SIZE - 1)
#define CELL_PX          48          //セル 1 個のピクセル幅
#define BOARD_ORIGIN_X   5           //盤面左上 X 座標
#define BOARD_ORIGIN_Y   5           //盤面左上 Y 座標
#define BOARD_END_PX     580         //盤面終端ピクセル (ORIGIN + BOARD_SIZE * CELL_PX - 4)
#define PIECE_SIZE_PX    47          //駒画像のサイズ (CELL_PX - 1)
#define BOARD_CENTER_LOW 5           //初期 4 駒の左上座標
#define BOARD_CENTER_HIGH 6          //初期 4 駒の右下座標

//描画レイアウト座標 (右パネル: 黒白カウント、ターン表示、ラウンド表示、終了メッセージ)
#define PANEL_X            590       //右パネル X 位置 (BLACK/WHITE 等のラベルとカウント)
#define PANEL_BLACK_LABEL_Y 5
#define PANEL_BLACK_VALUE_Y 40
#define PANEL_WHITE_LABEL_Y 90
#define PANEL_WHITE_VALUE_Y 125
#define PANEL_TURN_X       680       //ターンインジケータ ("← Now") の X
#define PANEL_TURN_BLACK_Y 40
#define PANEL_TURN_WHITE_Y 125
#define PANEL_ROUND_LABEL_Y 220
#define PANEL_ROUND_VALUE_X 710
#define PANEL_END_MSG_Y     330      //ESC で終了メッセージなどの Y
#define PANEL_END_MSG_GAME1_Y 150    //Game1 専用の終了メッセージ Y

//メッセージ表示 (盤面下のターン/PASS 通知)
#define MSG_BOX_CENTER_X    192      //メッセージ箱の水平中心
#define MSG_BOX_Y_TOP       630
#define MSG_BOX_Y_BOTTOM    655
#define MSG_BOX_PADDING_X   30
#define MSG_TEXT_Y          620
#define MSG_WAIT_FRAMES     60       //setMsg で msg_wait に設定する値 (60FPS で 1 秒)

//フォント設定 (MenuScene 初期化時のサイズ)
#define FONT_SIZE_DEFAULT   32
#define HINT_GAIN_FONT_SIZE 20       //ヒントマス内の取得コマ数表示 (CELL_PX=48 に収まる小さめサイズ)
