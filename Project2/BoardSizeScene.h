#ifndef BOARDSIZESCENE_H_
#define BOARDSIZESCENE_H_

//盤面サイズ設定シーン (1.6.1 で導入、項目 6(a))
//OPTIONS の 5 行目「盤面サイズ設定」から Enter で起動、←→/Enter/Space でサイズ循環、X で OPTIONS 復帰
//1.5.8 で OPTIONS 内に組み込んでいた盤面サイズ + 320×320 プレビューを本シーンに分離し、
//OPTIONS のトグル列とプレビュー描画の被り問題を根本解消する設計

BOOL initBoardSizeScene(void);
void moveBoardSizeScene();
void renderBoardSizeScene(void);
void releaseBoardSizeScene(void);
void BoardSizeSceneCollideCallback(int nSrc, int nTarget, int nCollideID);

#endif
