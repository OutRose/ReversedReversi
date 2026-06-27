#ifndef OPTIONSSCENE_H_
#define OPTIONSSCENE_H_

// OptionsScene.cpp ファイル内の関数のうち、他のファイルから呼び出される関数のプロトタイプ宣言
// SCENE_OPTIONS (1.5.7 で導入、Game3 のオプション設定画面)

BOOL initOptionsScene(void);
void moveOptionsScene();
void renderOptionsScene(void);
void releaseOptionsScene(void);
void OptionsSceneCollideCallback(int nSrc, int nTarget, int nCollideID);

#endif
