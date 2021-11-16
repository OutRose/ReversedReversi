#include "GameSceneMain.h"

//全てのシーンのヘッダファイルをインクルードする
#include "Game1Scene.h"
#include "Game2Scene.h"
#include "Game3Scene.h"
#include "Game4Scene.h"
#include "Game5Scene.h"
#include "MenuScene.h"

//このファイル内だけで使用する関数のプロトタイプ宣言
//現在のシーンの初期化処理
void initCurrentScene(void);
//現在のシーンのフレーム処理
void moveCurrentScene();
//現在のシーンのレンダリング処理
void renderCurrentScene(void);
//現在のシーンの削除処理
void releaseCurrentScene(void);

//このファイル内だけで使用する変数の宣言（staticをつけて宣言する）
static SCENE_NO sceneNo = SCENE_NONE;	// 現在のシーン番号（必ず初期値はSCENE_NONE）
static SCENE_NO prevScene = SCENE_NONE;	// 1つ前のシーン番号（必ず初期値はSCENE_NONE）
static SCENE_NO nextScene = SCENE_NONE;	// 次のシーン番号（必ず初期値はSCENE_NONE）

//３ゲーム開始前の初期化を行う
BOOL InitGame(void) {
	// 全てのシーンで共有するモノを初期化する

	//３(1) 初めのシーン番号の設定
	changeScene(SCENE_MENU);
	return TRUE;
}

//フレーム処理
void FrameMove() {
	// 次のシーンに変更するかどうか判断する
	if (sceneNo != nextScene) {
		//現在のシーンの削除処理
		releaseCurrentScene();
		//現在のシーンを新規シーンに変更する
		sceneNo = nextScene;
		//新しいシーンの初期化処理
		initCurrentScene();
	}

	//現在のシーンのフレーム処理
	moveCurrentScene();
}

//レンダリング処理
void RenderScene() {
	//現在のシーンのレンダリング処理
	renderCurrentScene();
}

//ゲーム終了時の後処理
void GameRelease(void) {
	//現在のシーンの削除処理
	releaseCurrentScene();
	// 全てのシーンで共有するモノの削除処理をする
}

//３(2) 当り判定コールバック 　　　ここでは要素を削除しないこと！！
void  CollideCallback(int nSrc, int nTarget, int nCollideID) {
	switch (sceneNo) {
	case SCENE_NONE:	break;
	case SCENE_MENU:	MenuSceneCollideCallback(nSrc, nTarget, nCollideID);		break;
	case SCENE_GAME1:	Game1SceneCollideCallback(nSrc, nTarget, nCollideID);		break;
	case SCENE_GAME2:	Game2SceneCollideCallback(nSrc, nTarget, nCollideID);		break;
	case SCENE_GAME3:	Game3SceneCollideCallback(nSrc, nTarget, nCollideID);		break;
	case SCENE_GAME4:	Game4SceneCollideCallback(nSrc, nTarget, nCollideID);		break;
	case SCENE_GAME5:   Game5SceneCollideCallback(nSrc, nTarget, nCollideID);		break;
	case SCENE_MAX:		break;
	default:
		MessageBox(NULL, "まだそのシーンはない", "作成途中", 0);
		changeScene(prevScene);
	}
}

//シーンを変更する関数
void changeScene(SCENE_NO no) {
	// 現在のシーンと同じときは何もしない
	if (sceneNo == no)return;
	// 正しくないシーン番号の時は無視する
	if (no >= SCENE_MAX) return;
	if (no <= SCENE_NONE) return;
	// シーンを変更する
	prevScene = sceneNo;
	nextScene = no;
}

//３(3) 現在のシーンの初期化処理
void initCurrentScene(void) {
	switch (sceneNo) {
	case SCENE_NONE:	break;
	case SCENE_MENU:	initMenuScene();	break;
	case SCENE_GAME1:	initGame1Scene();	break;
	case SCENE_GAME2:	initGame2Scene();	break;
	case SCENE_GAME3:	initGame3Scene();	break;
	case SCENE_GAME4:	initGame4Scene();	break;
	case SCENE_GAME5:   initGame5Scene();	break;
	case SCENE_MAX:		break;
	default:
		MessageBox(NULL, "まだそのシーンはない", "作成途中", 0);
		changeScene(prevScene);
	}
}
//３(4) 現在のシーンのフレーム処理
void moveCurrentScene() {
	switch (sceneNo) {
	case SCENE_NONE:	break;
	case SCENE_MENU:	moveMenuScene();		break;
	case SCENE_GAME1:	moveGame1Scene();		break;
	case SCENE_GAME2:	moveGame2Scene();		break;
	case SCENE_GAME3:	moveGame3Scene();		break;
	case SCENE_GAME4:	moveGame4Scene();		break;
	case SCENE_GAME5:   moveGame5Scene();		break;
	case SCENE_MAX:		break;
	default:
		MessageBox(NULL, "まだそのシーンはない", "作成途中", 0);
		changeScene(prevScene);
	}
}
//３(5) 現在のシーンのレンダリング処理
void renderCurrentScene(void) {
	switch (sceneNo) {
	case SCENE_NONE:	break;
	case SCENE_MENU:	renderMenuScene();		break;
	case SCENE_GAME1:	renderGame1Scene();		break;
	case SCENE_GAME2:	renderGame2Scene();		break;
	case SCENE_GAME3:	renderGame3Scene();		break;
	case SCENE_GAME4:	renderGame4Scene();		break;
	case SCENE_GAME5:   renderGame5Scene();		break;
	case SCENE_MAX:		break;
	default:
		MessageBox(NULL, "まだそのシーンはない", "作成途中", 0);
		changeScene(prevScene);
	}
}
//３(6) 現在のシーンの削除処理
void releaseCurrentScene(void) {
	switch (sceneNo) {
	case SCENE_NONE:	break;
	case SCENE_MENU:	releaseMenuScene();		break;
	case SCENE_GAME1:	releaseGame1Scene();	break;
	case SCENE_GAME2:	releaseGame2Scene();	break;
	case SCENE_GAME3:	releaseGame3Scene();	break;
	case SCENE_GAME4:	releaseGame4Scene();	break;
	case SCENE_GAME5:   releaseGame5Scene();	break;
	case SCENE_MAX:		break;
	default:
		MessageBox(NULL, "まだそのシーンはない", "作成途中", 0);
		changeScene(prevScene);
	}
}