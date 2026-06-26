#include "GameSceneMain.h"
#include <assert.h>		//changeScene 範囲外チェックのアサート用 (Debug ビルドでのみ発火)
#include <stdlib.h>		//rb* 関数で GetRand と組み合わせ + 将来の汎用ユーティリティ用

//全てのシーンのヘッダファイルをインクルードする
#include "Game1Scene.h"
#include "Game2Scene.h"
#include "Game3Scene.h"
#include "Game4Scene.h"
#include "MenuScene.h"

//このファイル内だけで使用する関数のプロトタイプ宣言
//現在のシーンの初期化処理 (BOOL: 成功 TRUE / 失敗 FALSE、FrameMove 内のフォールバック判定で参照)
BOOL initCurrentScene(void);
//現在のシーンのフレーム処理
void moveCurrentScene();
//現在のシーンのレンダリング処理
void renderCurrentScene(void);
//現在のシーンの削除処理
void releaseCurrentScene(void);

//このファイル内だけで使用する変数の宣言（staticをつけて宣言する）
static SCENE_NO sceneNo = SCENE_NONE;	// 現在のシーン番号（必ず初期値はSCENE_NONE）
static SCENE_NO nextScene = SCENE_NONE;	// 次のシーン番号（必ず初期値はSCENE_NONE）

//シーンディスパッチャ用の関数ポインタ束 (全シーン共通インターフェース)
//新規シーン追加時は (1) GameSceneMain.h の SCENE_NO に 1 値追加、(2) 下の sceneTable に 1 行追加で完了
typedef struct _SCENE_HANDLERS {
	BOOL (*init)(void);						// initXXXScene
	void (*move)(void);						// moveXXXScene
	void (*render)(void);					// renderXXXScene
	void (*release)(void);					// releaseXXXScene
	void (*collide)(int, int, int);			// XXXSceneCollideCallback
} SCENE_HANDLERS;

//シーン番号順に並べたディスパッチテーブル (SCENE_NO の値がそのまま添字)
static const SCENE_HANDLERS sceneTable[SCENE_MAX] = {
	/* [SCENE_MENU]  */ { initMenuScene,  moveMenuScene,  renderMenuScene,  releaseMenuScene,  MenuSceneCollideCallback  },
	/* [SCENE_GAME1] */ { initGame1Scene, moveGame1Scene, renderGame1Scene, releaseGame1Scene, Game1SceneCollideCallback },
	/* [SCENE_GAME2] */ { initGame2Scene, moveGame2Scene, renderGame2Scene, releaseGame2Scene, Game2SceneCollideCallback },
	/* [SCENE_GAME3] */ { initGame3Scene, moveGame3Scene, renderGame3Scene, releaseGame3Scene, Game3SceneCollideCallback },
	/* [SCENE_GAME4] */ { initGame4Scene, moveGame4Scene, renderGame4Scene, releaseGame4Scene, Game4SceneCollideCallback },
};

//sceneNo が sceneTable の有効インデックス範囲内か判定するヘルパ
static BOOL isValidSceneIndex(SCENE_NO no) {
	return (no > SCENE_NONE && no < SCENE_MAX);
}

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
		//新しいシーンの初期化処理 (失敗時は SCENE_MENU にフォールバック、それも失敗なら諦める)
		if (!initCurrentScene()) {
			MyOutputDebugString(_T("initCurrentScene failed for scene %d, falling back to SCENE_MENU\n"), (int)sceneNo);
			if (sceneNo != SCENE_MENU) {
				//通常シーン失敗 → SCENE_MENU を試す
				sceneNo = SCENE_MENU;
				if (!initCurrentScene()) {
					//SCENE_MENU も失敗 → 諦める (sceneNo = SCENE_NONE で以後 move/render 無効化)
					MyOutputDebugString(_T("SCENE_MENU init also failed, giving up\n"));
					sceneNo = SCENE_NONE;
				}
			} else {
				//SCENE_MENU 自体が失敗 → 無限再試行を避け諦める
				sceneNo = SCENE_NONE;
			}
			//無限ループ防止: nextScene を sceneNo に合わせる (sceneNo != nextScene の再侵入を防ぐ)
			nextScene = sceneNo;
		}
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
	if (isValidSceneIndex(sceneNo)) sceneTable[sceneNo].collide(nSrc, nTarget, nCollideID);
}

//シーンを変更する関数
void changeScene(SCENE_NO no) {
	// 現在のシーンと同じときは何もしない
	if (sceneNo == no)return;
	// 正しくないシーン番号は無視 (Debug はログ + assert で停止、Release は無視継続)
	if (no >= SCENE_MAX || no <= SCENE_NONE) {
		MyOutputDebugString(_T("changeScene: invalid scene number %d (ignored)\n"), (int)no);
		assert(0 && "changeScene called with invalid scene number");
		return;
	}
	// シーンを変更する
	nextScene = no;
}

//３(3) 現在のシーンの初期化処理 (各シーンの init を呼び、BOOL 戻り値をそのまま返す)
BOOL initCurrentScene(void) {
	if (isValidSceneIndex(sceneNo)) return sceneTable[sceneNo].init();
	return FALSE;	//無効インデックス時は失敗扱い (FrameMove 側でフォールバックされる)
}
//３(4) 現在のシーンのフレーム処理
void moveCurrentScene() {
	if (isValidSceneIndex(sceneNo)) sceneTable[sceneNo].move();
}
//３(5) 現在のシーンのレンダリング処理
void renderCurrentScene(void) {
	if (isValidSceneIndex(sceneNo)) sceneTable[sceneNo].render();
}
//３(6) 現在のシーンの削除処理
void releaseCurrentScene(void) {
	if (isValidSceneIndex(sceneNo)) sceneTable[sceneNo].release();
}

//=========================================================================
//盤面ロジック (Game1Scene/Game2Scene 共有、β-D-5 でクローン統合)
//=========================================================================

//盤面初期化 (ゼロクリア + 中央 4 駒配置 + メッセージリセット)
//シーンの move 関数冒頭で 1 度呼ばれる想定 (静的ゼロ初期化と組み合わせる)
void rbInit(ReversiBoard* state) {
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			state->board[y][x] = 0;
		}
	}
	//初期コマを盤上中央に 4 つセット (左上/右下=黒、右上/左下=白の対角配置)
	state->board[BOARD_CENTER_LOW][BOARD_CENTER_LOW]   = GAME_TURN_BLACK;
	state->board[BOARD_CENTER_HIGH][BOARD_CENTER_HIGH] = GAME_TURN_BLACK;
	state->board[BOARD_CENTER_HIGH][BOARD_CENTER_LOW]  = GAME_TURN_WHITE;
	state->board[BOARD_CENTER_LOW][BOARD_CENTER_HIGH]  = GAME_TURN_WHITE;
	state->msg.clear();
	state->msg_wait = 0;
}

//指定位置にコマを置く (put_flag=false ならシミュレーションのみ、戻り値: 裏返ったコマ数)
//x, y = コマを置く座標、turn = 順番 (1=黒、2=白)
int rbPutPiece(ReversiBoard* state, int x, int y, int turn, bool put_flag) {
	int sum = 0;

	//既にコマが置かれているマスには置けない
	if (state->board[y][x] > 0) return 0;

	//置いた場所から上下左右、斜めの 8 方向に盤面をチェック (dx, dy で方向を示す)
	for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++) {
		//裏返すことができる敵コマの位置を一時格納しておく配列 (lnt-uninitialized-local 抑制で zero-init)
		int wx[BOARD_SIZE] = {}, wy[BOARD_SIZE] = {};

		for (int wn = 0;; wn++) {
			//kx, ky でチェックする場所を示す
			int kx = x + dx * (wn + 1);
			int ky = y + dy * (wn + 1);

			//チェック位置が盤面外か空きマスなら裏返せないのでループ脱出
			if (kx < 0 || kx > BOARD_SIZE_MAX || ky < 0 || ky > BOARD_SIZE_MAX || state->board[ky][kx] == 0) break;

			//間に挟まれたコマを実際に裏返す (put_flag=true 時のみ書き換え、sum には wn を加算)
			if (state->board[ky][kx] == turn) {
				if (put_flag) for (int i = 0; i < wn; i++) state->board[wy[i]][wx[i]] = turn;
				sum += wn;
				break;
			}

			//敵コマならば裏返せるので、一時的に位置を格納しておく
			wx[wn] = kx; wy[wn] = ky;
		}
	}

	//1 つでも裏返せて、かつ実際に置く指示なら、置いたマスにコマを書き込む
	if (sum > 0 && put_flag) state->board[y][x] = turn;

	return sum;
}

//パスチェック (ターン実行中のプレイヤーが置けるマスが無ければ true)
bool rbIsPass(ReversiBoard* state, int turn) {
	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		if (rbPutPiece(state, x, y, turn, false)) return false;	//1 か所でも置けるならパス不要
	}
	return true;
}

//プレイヤー思考 (マウス左クリック、戻り値: 実際にコマを置いたら true)
//mouse_flag は関数内 static として保持し、ボタン押下→離す→再押下のエッジ検知をする
bool rbThinkPlayer(ReversiBoard* state, int turn) {
	static bool mouse_flag = false;	//クリックされているかのフラグ (連打防止)

	if (GetMouseInput() & MOUSE_INPUT_LEFT) {
		if (!mouse_flag) {
			mouse_flag = true;	//フラグを立てる (押下中)
			int mx, my;
			GetMousePoint(&mx, &my);	//マウスポインタの場所を取得

			//δ-2: 盤面範囲外クリックでの配列アクセス UB を防止
			//負値ガード: C++ の整数除算はゼロ向き切り捨てなので mx=-10 → bx=0 になり単純な上限チェックでは捕捉不能
			if (mx < 0 || my < 0) return false;
			int bx = mx / CELL_PX;
			int by = my / CELL_PX;
			//上限ガード: 右パネル領域 (x>=576) や下部メッセージ領域 (y>=576) クリックを除外
			if (bx >= BOARD_SIZE || by >= BOARD_SIZE) return false;

			//ポインタのある場所のマスに置く (置けたら true 返し、ターン進行)
			if (rbPutPiece(state, bx, by, turn, true)) return true;
		}
	} else {
		mouse_flag = false;	//ボタンが離されたらフラグを下ろす
	}
	return false;
}

//CPU 思考 (最も多く取れるマスに置く、等値時は 1/2 確率で入れ替え)
bool rbThinkCpu(ReversiBoard* state, int turn) {
	int max = 0, wx = 0, wy = 0;	//最大取得数と置く座標 (未初期化警告を避けるため 0 初期化)

	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		//盤面を順にチェックして、取得可能なコマ数を得る
		int num = rbPutPiece(state, x, y, turn, false);

		//現在の最大を超えれば無条件入れ替え。等値なら 1/2 確率で入れ替え (バリエーション)
		if (max < num || (max == num && GetRand(1) == 0)) {
			max = num; wx = x; wy = y;
		}
	}
	rbPutPiece(state, wx, wy, turn, true);	//確定したマスに置く
	return true;
}

//CPU 思考 弱 (置ける場所からランダム選択、Game3 あまちゃん用、戻り値: 置けたら true)
//rbThinkCpu (貪欲: 最多取得) より明確に弱く、初心者でも勝てる難易度を提供する
//rbIsPass が事前に呼ばれて false (置ける場所あり) を確認している前提だが、念のため候補 0 件のガードあり
bool rbThinkRandom(ReversiBoard* state, int turn) {
	int candX[BOARD_SIZE * BOARD_SIZE] = {};	//置ける候補の x 座標 (lnt-uninitialized-local 抑制で zero-init)
	int candY[BOARD_SIZE * BOARD_SIZE] = {};	//置ける候補の y 座標 (lnt-uninitialized-local 抑制で zero-init)
	int candCount = 0;

	//置ける場所を全マス走査して候補リストに集める
	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		if (rbPutPiece(state, x, y, turn, false)) {
			candX[candCount] = x;
			candY[candCount] = y;
			candCount++;
		}
	}

	if (candCount == 0) return false;	//置ける場所なし (rbIsPass 側で検出されるが念のため)

	//候補からランダム選択 (GetRand(n) は 0〜n を返すので n-1 を渡す)
	int idx = GetRand(candCount - 1);
	rbPutPiece(state, candX[idx], candY[idx], turn, true);
	return true;
}

//メッセージセット (turn: 1=BLACK 2=WHITE 3=DRAW、type: 0=TURN 1=PASS 2=WINS!)
void rbSetMsg(ReversiBoard* state, int turn, int type) {
	std::string turn_str[] = { "BLACK", "WHITE", "DRAW" };
	std::string type_str[] = { "TURN", "PASS", "WINS!" };
	state->msg = turn_str[turn - 1];
	if (turn != 3) state->msg += " " + type_str[type];	//引き分け時 (turn=3) は "DRAW" のみ
	state->msg_wait = MSG_WAIT_FRAMES;
}

//勝敗チェック (戻り値: 0=継続 / 1=黒勝 / 2=白勝 / 3=引分)
//両プレイヤーがパスで詰みなら終了、コマ数で勝者を判定
int rbCheckResult(ReversiBoard* state) {
	int pnum[2] = {};	//pnum[0]=黒、pnum[1]=白
	int result = 0;

	//盤面のコマ数を数える
	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		if (state->board[y][x] > 0) pnum[state->board[y][x] - 1]++;
	}

	//双方ともパス＝終了の合図。勝敗チェックが始まる。
	if (rbIsPass(state, GAME_TURN_BLACK) && rbIsPass(state, GAME_TURN_WHITE)) {
		if (pnum[0] > pnum[1]) result = 1;
		else if (pnum[0] < pnum[1]) result = 2;
		else result = 3;
	}

	if (result) rbSetMsg(state, result, 2);	//勝者メッセージをセット
	return result;
}

//黒白コマ数カウント (pcnum[0]=黒、pcnum[1]=白)
//画面表示用、勝敗判定とは別経路 (rbCheckResult 内のカウントは関数内で完結)
void rbCountPieces(ReversiBoard* state, int pcnum[2]) {
	pcnum[0] = pcnum[1] = 0;
	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		if (state->board[y][x] > 0) pcnum[state->board[y][x] - 1]++;
	}
}

//ランダム削除 (Game2 まきもどり用、count マスをゼロにする)
//重複削除が発生するため実削除数は count を下回る可能性があるが、元実装の挙動を維持
void rbRemovePieces(ReversiBoard* state, int count) {
	for (int j = 0; j < count; j++) {
		int rmx = GetRand(BOARD_SIZE_MAX);
		int rmy = GetRand(BOARD_SIZE_MAX);
		state->board[rmy][rmx] = 0;
	}
}

//=========================================================================
//盤面描画ヘルパ (Game1Scene/Game2Scene 共有、δ-4 で renderXxxScene のクローン解消)
//=========================================================================

//盤面背景 + 四辺枠線を描画 (色は引数差し替え、Game1=暗緑/Game2=明緑)
void rbDrawBoard(int boardBgColor) {
	DrawBox(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_END_PX, BOARD_END_PX, boardBgColor, TRUE);
	//四辺の枠線 (3 重描画は元コード由来、線幅強調)
	for (int i = 0; i < 3; i++) {
		DrawLine(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_END_PX,    BOARD_ORIGIN_Y, ColorWhite);
		DrawLine(BOARD_ORIGIN_X, BOARD_ORIGIN_Y, BOARD_ORIGIN_X,  BOARD_END_PX,   ColorWhite);
		DrawLine(BOARD_END_PX,   BOARD_ORIGIN_Y, BOARD_END_PX,    BOARD_END_PX,   ColorWhite);
		DrawLine(BOARD_ORIGIN_X, BOARD_END_PX,   BOARD_END_PX,    BOARD_END_PX,   ColorWhite);
	}
}

//縦横の格子線を描画 (盤面 12×12 の内側仕切り、BOARD_SIZE_MAX=11 本ずつ)
void rbDrawGrid(void) {
	int DrawX = BOARD_ORIGIN_X, DrawY = BOARD_ORIGIN_Y;
	for (int i = 0; i < BOARD_SIZE_MAX; i++) {
		DrawX += CELL_PX;
		DrawLine(DrawX, BOARD_ORIGIN_Y, DrawX, BOARD_END_PX, ColorWhite);
	}
	for (int j = 0; j < BOARD_SIZE_MAX; j++) {
		DrawY += CELL_PX;
		DrawLine(BOARD_ORIGIN_X, DrawY, BOARD_END_PX, DrawY, ColorWhite);
	}
}

//コマを描画 (pieces[0]=黒画像ハンドル、pieces[1]=白画像ハンドル)
void rbDrawPieces(ReversiBoard* state, int pieces[2]) {
	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		if (state->board[y][x]) {
			DrawGraph(x * CELL_PX + BOARD_ORIGIN_X, y * CELL_PX + BOARD_ORIGIN_Y, pieces[state->board[y][x] - 1], TRUE);
		}
	}
}

//ターン/パス/勝者メッセージ箱を盤面下に描画 (PLAYING 中=1 は非表示)
//status > 1 (TURN_MSG/PASS_MSG/FINISHED) の時に灰色箱の中に state->msg を表示
void rbDrawMsg(ReversiBoard* state, int status) {
	if (status <= GAME_STATUS_PLAYING) return;
	//δ-3: x64 ビルド時の C4267 (size_t→int 暗黙縮小) を回避。メッセージは数文字なので int で十分
	int mw = GetDrawStringWidth(state->msg.c_str(), (int)state->msg.size());
	DrawBox(MSG_BOX_CENTER_X - mw / 2 - MSG_BOX_PADDING_X, MSG_BOX_Y_TOP,
	        MSG_BOX_CENTER_X + mw / 2 + MSG_BOX_PADDING_X, MSG_BOX_Y_BOTTOM, GetColor(150, 150, 150), TRUE);
	DrawString(MSG_BOX_CENTER_X - mw / 2, MSG_TEXT_Y, state->msg.c_str(), ColorWhite);
}

//右パネルの BLACK/WHITE カウントを描画 (優勢な方は数値が赤、引分時はどちらも白)
void rbDrawCountPanel(ReversiBoard* state) {
	int pcnum[2] = {};
	char blackC[32], whiteC[32];
	rbCountPieces(state, pcnum);
	_itoa_s(pcnum[0], blackC, sizeof(blackC), 10);
	_itoa_s(pcnum[1], whiteC, sizeof(whiteC), 10);

	//優勢判定 (1=黒優勢 / 2=白優勢 / 3=引分)
	int winning = 0;
	if (pcnum[0] > pcnum[1]) winning = 1;
	else if (pcnum[0] < pcnum[1]) winning = 2;
	else winning = 3;

	//BLACK ラベル + 数値 (優勢時のみ赤)
	DrawString(PANEL_X, PANEL_BLACK_LABEL_Y, "BLACK", ColorWhite);
	DrawString(PANEL_X, PANEL_BLACK_VALUE_Y, blackC, (winning == 1) ? ColorRed : ColorWhite);

	//WHITE ラベル + 数値 (優勢時のみ赤)
	DrawString(PANEL_X, PANEL_WHITE_LABEL_Y, "WHITE", ColorWhite);
	DrawString(PANEL_X, PANEL_WHITE_VALUE_Y, whiteC, (winning == 2) ? ColorRed : ColorWhite);
}

//"← Now" 矢印で現在の手番を表示 (黒なら BLACK 行に、白なら WHITE 行に)
void rbDrawTurnIndicator(int turn) {
	if (turn == GAME_TURN_BLACK) {
		DrawString(PANEL_TURN_X, PANEL_TURN_BLACK_Y, "← Now", ColorSky);
	} else if (turn == GAME_TURN_WHITE) {
		DrawString(PANEL_TURN_X, PANEL_TURN_WHITE_Y, "← Now", ColorSky);
	}
}

//置ける場所を半透明丸でハイライト + 取得コマ数を重ね描き (Game3 あまちゃん用、初心者向けヒント表示)
//全マス走査で rbPutPiece の put_flag=false シミュレーションを使って置けるマスと裏返るコマ数を取得。
//2 パス構成: パス 1 = 半透明オレンジ丸、パス 2 = オレンジの上に白で取得コマ数を中央寄せ表示。
//色 (255, 165, 0) はオレンジ。黒コマ/白コマ/暗緑盤面のいずれとも混同しにくく、ヒントらしい暖色
void rbDrawHints(ReversiBoard* state, int turn) {
	//ヒント色 (関数冒頭で 1 度だけ計算、144 マス走査での GetColor 呼び出しを回避)
	int hintColor = GetColor(255, 165, 0);
	//呼び出し元のフォントサイズを退避 (パス 2 で小さくするため、関数末尾で復元)
	int oldFontSize = GetFontSize();

	//パス 1: 半透明オレンジ丸を描画 (255 段階で 128 = 50% アルファ)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		if (rbPutPiece(state, x, y, turn, false)) {
			//マス中央に塗り潰し丸を描画 (半径はマスの 1/3、視認性と被らなさのバランス)
			int cx = x * CELL_PX + BOARD_ORIGIN_X + CELL_PX / 2;
			int cy = y * CELL_PX + BOARD_ORIGIN_Y + CELL_PX / 2;
			DrawCircle(cx, cy, CELL_PX / 3, hintColor, TRUE);
		}
	}
	//ブレンドモードを元に戻す (パス 2 のテキストは不透明、以降の描画に影響しないように必須)
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//パス 2: 取得コマ数を白で中央寄せ重ね描き (オレンジ円の中で白が最も視認性高い)
	SetFontSize(HINT_GAIN_FONT_SIZE);
	for (int y = 0; y < BOARD_SIZE; y++) for (int x = 0; x < BOARD_SIZE; x++) {
		int gain = rbPutPiece(state, x, y, turn, false);
		if (gain) {
			int cx = x * CELL_PX + BOARD_ORIGIN_X + CELL_PX / 2;
			int cy = y * CELL_PX + BOARD_ORIGIN_Y + CELL_PX / 2;
			//桁数によらず中央配置するため、実描画幅を取得して半分ずらす
			int textW = GetDrawFormatStringWidth("%d", gain);
			DrawFormatString(cx - textW / 2, cy - HINT_GAIN_FONT_SIZE / 2, ColorWhite, "%d", gain);
		}
	}

	//フォントサイズを呼び出し前の値に戻す (後続の rbDrawMsg/CountPanel/TurnIndicator に影響させない)
	SetFontSize(oldFontSize);
}
