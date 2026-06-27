#ifndef GAMESCENETEMPLATE_H_
#define GAMESCENETEMPLATE_H_

//空シーン雛形 (1.5.8 で旧 Game4Scene から固定名にリネーム、ユーザー指示)
//新規シーン追加時は本ファイル + GameSceneTemplate.cpp をコピーして識別子置換し、
//SCENE_NO enum と sceneTable[] に 1 行ずつ追加する。雛形自身は menu[] 非掲載で残置。

BOOL initGameSceneTemplate(void);
void moveGameSceneTemplate();
void renderGameSceneTemplate(void);
void releaseGameSceneTemplate(void);
void GameSceneTemplateCollideCallback(int nSrc, int nTarget, int nCollideID);

#endif
