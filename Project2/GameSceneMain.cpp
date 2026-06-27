#include "GameSceneMain.h"
#include <assert.h>		//changeScene 範囲外チェックのアサート用 (Debug ビルドでのみ発火)
#include <stdlib.h>		//rb* 関数で GetRand と組み合わせ + 将来の汎用ユーティリティ用
#include <math.h>		//1.6.0 ランクアップ演出の sinf/cosf 用 (1.6.1 で揺れ動き撤去により本リリースでは未参照、将来の演出強化用に保持)
#include <string.h>		//1.6.0 ランク演出で strlen (ティア名長取得) 用

//全てのシーンのヘッダファイルをインクルードする
#include "Game1Scene.h"
#include "Game2Scene.h"
#include "Game3Scene.h"
#include "GameSceneTemplate.h"	//1.5.8 で旧 Game4Scene.h からリネーム (固定名の空シーン雛形)
#include "MenuScene.h"
#include "OptionsScene.h"	//1.5.7 で導入
#include "BoardSizeScene.h"	//1.6.1 で導入 (盤面サイズ設定、OPTIONS から遷移)

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
	/* [SCENE_TEMPLATE] */ { initGameSceneTemplate, moveGameSceneTemplate, renderGameSceneTemplate, releaseGameSceneTemplate, GameSceneTemplateCollideCallback },
	/* [SCENE_OPTIONS] */ { initOptionsScene, moveOptionsScene, renderOptionsScene, releaseOptionsScene, OptionsSceneCollideCallback },
	/* [SCENE_BOARD_SIZE] */ { initBoardSizeScene, moveBoardSizeScene, renderBoardSizeScene, releaseBoardSizeScene, BoardSizeSceneCollideCallback },
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
	//1.6.3: ランクアップ SE ハンドル解放 (DeleteSoundMem) は撤去 (詳細経緯は GameMain.h のコメント参照)
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

//盤面サイズ選択肢の定義 (宣言は GameSceneMain.h)
const int BOARD_SIZE_CHOICES[BOARD_SIZE_CHOICE_COUNT] = { 6, 8, 10, 12 };

//盤面初期化 (ゼロクリア + 中央 4 駒配置 + メッセージリセット + サイズメトリクス設定)
//シーンの init 関数で呼ばれる想定 (静的ゼロ初期化 + 再入場時のリセットを兼ねる)
//size は BOARD_SIZE_CHOICES の値、想定外なら 12 にフォールバック (異常時の安全策)
//1.5.8 で size 引数追加、cellPx/originX/originY を size から自動計算
//1.5.9 で BOARD_TARGET_PX=720 に拡張、全サイズが綺麗に割り切れる (120/90/72/60) ため特殊ケース撤廃
void rbInit(ReversiBoard* state, int size) {
	//size 値検証 (BOARD_SIZE_CHOICES に含まれない値は 12 にフォールバック)
	bool valid = false;
	for (int i = 0; i < BOARD_SIZE_CHOICE_COUNT; i++) if (BOARD_SIZE_CHOICES[i] == size) { valid = true; break; }
	if (!valid) size = 12;

	state->size = size;
	//cellPx を BOARD_TARGET_PX (720) / size で計算 — 6/8/10/12 すべて綺麗に割り切れる
	state->cellPx = BOARD_TARGET_PX / size;
	//全サイズで中央寄せ補正不要 (BOARD_TARGET_PX = size × cellPx ぴったり)
	state->originX = BOARD_ORIGIN_X;
	state->originY = BOARD_ORIGIN_Y;

	//ストレージ全域 (12×12) をゼロクリア (未使用領域も常に 0 に保つ)
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++) {
			state->board[y][x] = 0;
		}
	}
	//初期コマを盤上中央に 4 つセット (左上/右下=黒、右上/左下=白の対角配置)
	//位置は state->size から動的計算 (6=2,3 / 8=3,4 / 10=4,5 / 12=5,6)
	int centerLow  = size / 2 - 1;
	int centerHigh = size / 2;
	state->board[centerLow ][centerLow ] = GAME_TURN_BLACK;
	state->board[centerHigh][centerHigh] = GAME_TURN_BLACK;
	state->board[centerHigh][centerLow ] = GAME_TURN_WHITE;
	state->board[centerLow ][centerHigh] = GAME_TURN_WHITE;
	state->msg.clear();
	state->msg_wait = 0;
	//1.6.0 で追加: ランクシステム用カウンタを初期化 (XP 短手数ボーナス判定 + 将来のパスなし勝利フック用)
	state->moveCount = 0;
	state->passCount = 0;
}

//指定位置にコマを置く (put_flag=false ならシミュレーションのみ、戻り値: 裏返ったコマ数)
//x, y = コマを置く座標、turn = 順番 (1=黒、2=白)
//1.5.8 で境界チェックを state->size に動的化 (一時格納配列は最大値 BOARD_SIZE で確保継続)
int rbPutPiece(ReversiBoard* state, int x, int y, int turn, bool put_flag) {
	int sum = 0;

	//既にコマが置かれているマスには置けない
	if (state->board[y][x] > 0) return 0;

	//置いた場所から上下左右、斜めの 8 方向に盤面をチェック (dx, dy で方向を示す)
	for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++) {
		//裏返すことができる敵コマの位置を一時格納しておく配列 (ストレージ最大の BOARD_SIZE で確保、zero-init)
		int wx[BOARD_SIZE] = {}, wy[BOARD_SIZE] = {};

		for (int wn = 0;; wn++) {
			//kx, ky でチェックする場所を示す
			int kx = x + dx * (wn + 1);
			int ky = y + dy * (wn + 1);

			//チェック位置が盤面外 (有効サイズ超え) か空きマスなら裏返せないのでループ脱出
			if (kx < 0 || kx >= state->size || ky < 0 || ky >= state->size || state->board[ky][kx] == 0) break;

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
	if (sum > 0 && put_flag) {
		state->board[y][x] = turn;
		//1.6.0: 実際に手が確定したら moveCount をインクリメント (XP 短手数ボーナス判定用)
		state->moveCount++;
	}

	return sum;
}

//パスチェック (ターン実行中のプレイヤーが置けるマスが無ければ true)
bool rbIsPass(ReversiBoard* state, int turn) {
	for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
		if (rbPutPiece(state, x, y, turn, false)) return false;	//1 か所でも置けるならパス不要
	}
	return true;
}

//プレイヤー思考 (マウス左クリック、戻り値: 実際にコマを置いたら true)
//mouse_flag は関数内 static として保持し、ボタン押下→離す→再押下のエッジ検知をする
//1.5.8 でマウス座標を state->originX/Y/cellPx に動的化、小盤面 (中央寄せ) でも正しく判定
bool rbThinkPlayer(ReversiBoard* state, int turn) {
	static bool mouse_flag = false;	//クリックされているかのフラグ (連打防止)

	if (GetMouseInput() & MOUSE_INPUT_LEFT) {
		if (!mouse_flag) {
			mouse_flag = true;	//フラグを立てる (押下中)
			int mx, my;
			GetMousePoint(&mx, &my);	//マウスポインタの場所を取得

			//δ-2: 盤面範囲外クリックでの配列アクセス UB を防止 (1.5.8 で originX/Y を引いてから判定)
			//負値ガード: C++ の整数除算はゼロ向き切り捨てなので originX 未満でも bx=0 にマップされ単純な上限チェックでは捕捉不能
			if (mx < state->originX || my < state->originY) return false;
			int bx = (mx - state->originX) / state->cellPx;
			int by = (my - state->originY) / state->cellPx;
			//上限ガード: 右パネル領域や下部メッセージ領域、盤面右下の余白を除外
			if (bx >= state->size || by >= state->size) return false;

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

	for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
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
	int candX[BOARD_SIZE * BOARD_SIZE] = {};	//置ける候補の x 座標 (ストレージ最大の BOARD_SIZE^2 で確保、zero-init)
	int candY[BOARD_SIZE * BOARD_SIZE] = {};	//置ける候補の y 座標 (同上)
	int candCount = 0;

	//置ける場所を全マス走査して候補リストに集める
	for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
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

	//盤面のコマ数を数える (有効領域のみ走査)
	for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
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
	for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
		if (state->board[y][x] > 0) pcnum[state->board[y][x] - 1]++;
	}
}

//ランダム削除 (Game2 まきもどり用、count マスをゼロにする)
//重複削除が発生するため実削除数は count を下回る可能性があるが、元実装の挙動を維持
//1.5.8 で範囲を state->size-1 に動的化 (※ Game2 専用呼出で size=12 のため挙動不変)
void rbRemovePieces(ReversiBoard* state, int count) {
	for (int j = 0; j < count; j++) {
		int rmx = GetRand(state->size - 1);
		int rmy = GetRand(state->size - 1);
		state->board[rmy][rmx] = 0;
	}
}

//=========================================================================
//盤面描画ヘルパ (Game1Scene/Game2Scene 共有、δ-4 で renderXxxScene のクローン解消)
//=========================================================================

//盤面背景 + 四辺枠線を描画 (色は引数差し替え、Game1=暗緑/Game2=明緑)
//1.5.8 で state->originX/Y + size*cellPx に動的化
void rbDrawBoard(ReversiBoard* state, int boardBgColor) {
	int boardEndX = state->originX + state->size * state->cellPx;
	int boardEndY = state->originY + state->size * state->cellPx;
	DrawBox(state->originX, state->originY, boardEndX, boardEndY, boardBgColor, TRUE);
	//四辺の枠線 (3 重描画は元コード由来、線幅強調)
	for (int i = 0; i < 3; i++) {
		DrawLine(state->originX, state->originY, boardEndX,       state->originY, ColorWhite);
		DrawLine(state->originX, state->originY, state->originX,  boardEndY,      ColorWhite);
		DrawLine(boardEndX,      state->originY, boardEndX,       boardEndY,      ColorWhite);
		DrawLine(state->originX, boardEndY,      boardEndX,       boardEndY,      ColorWhite);
	}
}

//縦横の格子線を描画 (盤面 size×size の内側仕切り、size-1 本ずつ)
//1.5.8 で state->cellPx/originX/originY/size に動的化
void rbDrawGrid(ReversiBoard* state) {
	int boardEndX = state->originX + state->size * state->cellPx;
	int boardEndY = state->originY + state->size * state->cellPx;
	int DrawX = state->originX, DrawY = state->originY;
	for (int i = 0; i < state->size - 1; i++) {
		DrawX += state->cellPx;
		DrawLine(DrawX, state->originY, DrawX, boardEndY, ColorWhite);
	}
	for (int j = 0; j < state->size - 1; j++) {
		DrawY += state->cellPx;
		DrawLine(state->originX, DrawY, boardEndX, DrawY, ColorWhite);
	}
}

//コマを描画 (pieces[0]=黒画像ハンドル、pieces[1]=白画像ハンドル)
//1.5.8 で DrawExtendGraph に変更、state->cellPx に応じてコマ画像 (元 47px) を拡大
void rbDrawPieces(ReversiBoard* state, int pieces[2]) {
	for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
		if (state->board[y][x]) {
			int left = x * state->cellPx + state->originX;
			int top  = y * state->cellPx + state->originY;
			//right/bottom は閉区間で 1px 引いてグリッド線跨ぎを避ける
			DrawExtendGraph(left, top, left + state->cellPx - 1, top + state->cellPx - 1,
			                pieces[state->board[y][x] - 1], TRUE);
		}
	}
}

//ターン/パス/勝者メッセージ箱を盤面下に描画 (PLAYING 中=1 は非表示)
//status > 1 (TURN_MSG/PASS_MSG/FINISHED) の時に灰色箱の中に state->msg を表示
//1.5.9 768px 化で MSG_FONT_SIZE=28 (FONT_SIZE_DEFAULT 40 より小さい) を専用で使用
//描画前後でフォントサイズを退避/復元、後続描画への副作用を遮断
void rbDrawMsg(ReversiBoard* state, int status) {
	if (status <= GAME_STATUS_PLAYING) return;
	int oldFontSize = GetFontSize();
	SetFontSize(MSG_FONT_SIZE);
	//δ-3: x64 ビルド時の C4267 (size_t→int 暗黙縮小) を回避。メッセージは数文字なので int で十分
	int mw = GetDrawStringWidth(state->msg.c_str(), (int)state->msg.size());
	DrawBox(MSG_BOX_CENTER_X - mw / 2 - MSG_BOX_PADDING_X, MSG_BOX_Y_TOP,
	        MSG_BOX_CENTER_X + mw / 2 + MSG_BOX_PADDING_X, MSG_BOX_Y_BOTTOM, GetColor(150, 150, 150), TRUE);
	DrawString(MSG_BOX_CENTER_X - mw / 2, MSG_TEXT_Y, state->msg.c_str(), ColorWhite);
	SetFontSize(oldFontSize);
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
//1.5.8 で state->cellPx に動的追従 (丸半径 + フォントサイズ)
void rbDrawHints(ReversiBoard* state, int turn, bool showGain) {
	//ヒント色 (関数冒頭で 1 度だけ計算、最大 144 マス走査での GetColor 呼び出しを回避)
	int hintColor = GetColor(255, 165, 0);
	//呼び出し元のフォントサイズを退避 (パス 2 で小さくするため、関数末尾で復元)
	int oldFontSize = GetFontSize();
	//1.5.8 で動的フォントサイズ (12=48→20、10=57→23、8=72→30、6=96→40)
	int hintFontSize = state->cellPx * 5 / 12;
	int hintRadius = state->cellPx / 3;

	//パス 1: 半透明オレンジ丸を描画 (255 段階で 128 = 50% アルファ)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
		if (rbPutPiece(state, x, y, turn, false)) {
			//マス中央に塗り潰し丸を描画 (半径はマスの 1/3、視認性と被らなさのバランス)
			int cx = x * state->cellPx + state->originX + state->cellPx / 2;
			int cy = y * state->cellPx + state->originY + state->cellPx / 2;
			DrawCircle(cx, cy, hintRadius, hintColor, TRUE);
		}
	}
	//ブレンドモードを元に戻す (パス 2 のテキストは不透明、以降の描画に影響しないように必須)
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//パス 2: 取得コマ数を白で中央寄せ重ね描き (showGain=false ならスキップ、1.5.7 でトグル化)
	if (showGain)
	{
		SetFontSize(hintFontSize);
		for (int y = 0; y < state->size; y++) for (int x = 0; x < state->size; x++) {
			int gain = rbPutPiece(state, x, y, turn, false);
			if (gain) {
				int cx = x * state->cellPx + state->originX + state->cellPx / 2;
				int cy = y * state->cellPx + state->originY + state->cellPx / 2;
				//桁数によらず中央配置するため、実描画幅を取得して半分ずらす
				int textW = GetDrawFormatStringWidth("%d", gain);
				DrawFormatString(cx - textW / 2, cy - hintFontSize / 2, ColorWhite, "%d", gain);
			}
		}

		//フォントサイズを呼び出し前の値に戻す (後続の rbDrawMsg/CountPanel/TurnIndicator に影響させない)
		SetFontSize(oldFontSize);
	}
}

//=========================================================================
//B2 プレイヤーランクシステム関連関数 (1.6.0 で追加)
//=========================================================================

//ティア名取得 (範囲外チェック付き)
const char* getTierName(int tier)
{
	if (tier < 0 || tier >= TIER_COUNT) return "?";
	return TIER_NAMES[tier];
}

//ティア色取得 (範囲外チェック付き、不正値は ColorWhite フォールバック)
unsigned int getTierColor(int tier)
{
	if (tier < 0 || tier >= TIER_COUNT) return ColorWhite;
	return TIER_COLORS[tier];
}

//XP 計算式 (案 C エンゲージメントベース、Game3 オプションペナルティ付き)
//勝=30 + コマ差ボーナス上限+30 + 短手数+10 + 完封+15 / 引分=15 / 負=10
//モード倍率 Game1=×1.0 / Game2=×1.4 / Game3=×0.6
//Game3 のヒント/取得数/弱 CPU/待った の ON 数 × 0.1 を 1.0 から減算してさらに乗算
int calcXpGain(int mode, bool won, int pieceDiff, int moveCount, bool perfect,
	bool g3Hints, bool g3Gain, bool g3Weak, bool g3Undo)
{
	int baseXp = 0;
	if (won) {
		baseXp = 30;
		//コマ差ボーナス (上限 +30)
		int diffBonus = (pieceDiff > 30) ? 30 : pieceDiff;
		if (diffBonus > 0) baseXp += diffBonus;
		//短手数ボーナス (12×12 で全 140 マス埋まる、空 10 以上なら +10)
		int totalCells = BOARD_SIZE * BOARD_SIZE - 4;
		if ((totalCells - moveCount) >= 10) baseXp += 10;
		//完封ボーナス (相手 ≤5 コマ)
		if (perfect) baseXp += 15;
	}
	else if (pieceDiff == 0) {
		baseXp = 15;	//引分 (両者同コマ数)
	}
	else {
		baseXp = 10;	//敗北、最低保証
	}
	//モード倍率
	float mult = (mode == MODE_GAME1) ? 1.0f
		: (mode == MODE_GAME2) ? 1.4f
		: 0.6f;	//Game3
	//Game3 オプションペナルティ (ON 数 × 0.1 を 1.0 から減算してさらに乗算)
	if (mode == MODE_GAME3) {
		int onCount = (g3Hints ? 1 : 0) + (g3Gain ? 1 : 0) + (g3Weak ? 1 : 0) + (g3Undo ? 1 : 0);
		float penalty = 1.0f - onCount * 0.1f;
		if (penalty < 0.1f) penalty = 0.1f;	//下限ガード (理論上は 0.6 まで)
		mult *= penalty;
	}
	int result = (int)(baseXp * mult);
	if (result < 0) result = 0;	//念のため下限ガード
	return result;
}

//XP 加算 + ティア再判定 + 演出トリガ判定
void applyXpAndCheck(int xpGained, bool won, int* outOldTier, int* outNewTier)
{
	int oldTier = g_playerStats.currentTier;
	if (outOldTier) *outOldTier = oldTier;

	//XP 加算
	g_playerStats.totalXp += xpGained;
	if (g_playerStats.totalXp > MAX_XP) g_playerStats.totalXp = MAX_XP;

	//敗北時の降格圧力 (全モード、5 + tier*2 を減算)
	if (!won && oldTier > 0) {
		int pressure = 5 + oldTier * 2;
		g_playerStats.totalXp -= pressure;
		if (g_playerStats.totalXp < 0) g_playerStats.totalXp = 0;
	}

	//昇格判定 (現ティアの次の閾値を超えていれば +1、複数段飛ばし対応)
	int newTier = oldTier;
	while (newTier < TIER_COUNT - 1 && g_playerStats.totalXp >= TIER_THRESHOLDS[newTier + 1]) {
		newTier++;
	}
	//降格判定 (現ティア閾値 - DEMOTE_BUFFER_XP を下回ったら -1、複数段降格対応)
	while (newTier > 0 && g_playerStats.totalXp < TIER_THRESHOLDS[newTier] - DEMOTE_BUFFER_XP) {
		newTier--;
	}

	g_playerStats.currentTier = newTier;
	if (outNewTier) *outNewTier = newTier;
}

//ランク章描画 (小型、対局画面の右パネル下部用)
//(x, y) はランク章円の中心ではなく左上座標として扱い、半径 24 の円 + 右にティア名フォント 22
void rbDrawRankBadgeSmall(int x, int y)
{
	int oldFontSize = GetFontSize();
	int cx = x + 24;	//円中心 X
	int cy = y + 24;	//円中心 Y
	unsigned int tierColor = getTierColor(g_playerStats.currentTier);
	DrawCircle(cx, cy, 24, tierColor, TRUE);
	DrawCircle(cx, cy, 24, ColorWhite, FALSE);
	SetFontSize(22);
	DrawString(x + 56, y + 12, getTierName(g_playerStats.currentTier), tierColor);
	SetFontSize(oldFontSize);
}

//終局時のリザルトオーバーレイ (FINISHED 状態で常時表示、半透明背景 + XP 表示)
//xpGained=今回獲得 XP、oldTier=変動前ティア、newTier=変動後ティア (rankup/demote 判定済み)
//1.6.0 polish: 盤面エリア (X=5..725) 内に半透明黒背景の枠を描いて、その中に XP テキストを中央配置。
//右パネル (PANEL_X=745+) と完全に分離、盤面コマとの被りを背景フェードで軽減
void rbDrawResultOverlay(int xpGained, int oldTier, int newTier)
{
	int oldFontSize = GetFontSize();
	unsigned int tierColor = getTierColor(newTier);

	//半透明黒背景の枠 (盤面エリア内、コマと XP テキストの間に視認性確保のクッション)
	//盤面中央 X = BOARD_ORIGIN_X + BOARD_TARGET_PX/2 = 365、Y は中央寄り上半分 (BLACK WINS! メッセージ箱 730+ と分離)
	const int boxX1 = 100, boxY1 = 310, boxX2 = 630, boxY2 = 460;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(boxX1, boxY1, boxX2, boxY2, GetColor(20, 20, 20), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(boxX1, boxY1, boxX2, boxY2, ColorWhite, FALSE);

	const int centerX = (boxX1 + boxX2) / 2;	//= 365、盤面中心と一致

	//「+47 XP」をティア色フォント 48 で枠内中央
	SetFontSize(48);
	int xpW = GetDrawFormatStringWidth("+%d XP", xpGained);
	DrawFormatString(centerX - xpW / 2, boxY1 + 15, tierColor, "+%d XP", xpGained);

	//「TOTAL: 423 / NEXT: 277 XP」をフォント 28 で枠内下段中央
	SetFontSize(28);
	int totalXp = g_playerStats.totalXp;
	int curTier = g_playerStats.currentTier;
	if (curTier >= TIER_COUNT - 1) {
		//ETERNAL 到達済
		int totalW = GetDrawFormatStringWidth("TOTAL: %d / MAX TIER", totalXp);
		DrawFormatString(centerX - totalW / 2, boxY1 + 95, ColorGold, "TOTAL: %d / MAX TIER", totalXp);
	}
	else {
		int nextNeeded = TIER_THRESHOLDS[curTier + 1] - totalXp;
		if (nextNeeded < 0) nextNeeded = 0;
		int totalW = GetDrawFormatStringWidth("TOTAL: %d / NEXT: %d XP", totalXp, nextNeeded);
		DrawFormatString(centerX - totalW / 2, boxY1 + 95, ColorWhite, "TOTAL: %d / NEXT: %d XP", totalXp, nextNeeded);
	}

	SetFontSize(oldFontSize);
}

//ランクアップ演出 (240 フレーム、5 段階)
//frame=0..239 を渡す、フレーム外は呼び出さない想定
void rbDrawRankUpAnimation(int frame, int oldTier, int newTier)
{
	int oldFontSize = GetFontSize();
	unsigned int newColor = getTierColor(newTier);

	//段階 1: 0-30f 画面全体に半透明オーバーレイをフェードイン
	int overlayAlpha = 180;
	if (frame < 30) overlayAlpha = (frame * 180) / 30;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, overlayAlpha);
	DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorOverlay, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	//段階 2: 30-90f ランク章を半径 10→120 でスケールアップ (中心 640, 384 固定)
	//1.6.1 polish: 旧実装で sinf/cosf による半径 8px の小円軌道オフセットを加えていたが、
	//円は回転対称なので「中心が揺れ動く」失敗演出になっていた。中心固定でスケールのみの方がスケール感が際立つ
	if (frame >= 30) {
		int phase2Frame = frame - 30;
		int radius;
		if (phase2Frame < 60) {
			radius = 10 + (phase2Frame * 110) / 60;	//10 → 120
		}
		else {
			radius = 120;	//以降固定
		}
		DrawCircle(640, 384, radius, newColor, TRUE);
		DrawCircle(640, 384, radius, ColorWhite, FALSE);
	}

	//段階 3: 90-180f ティア名を 1 文字ずつフェードイン (フォント 80)
	if (frame >= 90) {
		const char* name = getTierName(newTier);
		int nameLen = (int)strlen(name);
		int phase3Frame = frame - 90;
		int charsToShow = phase3Frame / 12;	//1 文字 = 12f
		if (charsToShow > nameLen) charsToShow = nameLen;
		SetFontSize(80);
		int totalW = GetDrawStringWidth(name, nameLen);
		int x = 640 - totalW / 2;
		int y = 540;
		//先頭から charsToShow 文字までを描画 (TCHAR/MBCS 互換のため部分文字列を一時バッファに)
		char buf[64];
		int n = (charsToShow < 63) ? charsToShow : 63;
		for (int i = 0; i < n; i++) buf[i] = name[i];
		buf[n] = '\0';
		if (n > 0) {
			DrawString(x, y, buf, newColor);
		}
	}

	//段階 4: 180-240f 「NEW RANK!」を 10f 周期で点滅
	if (frame >= 180) {
		int phase4Frame = frame - 180;
		bool show = ((phase4Frame / 10) % 2) == 0;
		if (show) {
			SetFontSize(60);
			const char* msg = "NEW RANK!";
			int msgLen = (int)strlen(msg);
			int msgW = GetDrawStringWidth(msg, msgLen);
			DrawString(640 - msgW / 2, 200, msg, ColorGold);
		}
	}

	SetFontSize(oldFontSize);
}

//降格演出 (120 フレーム、3 段階)
void rbDrawDemoteAnimation(int frame, int oldTier, int newTier)
{
	int oldFontSize = GetFontSize();
	unsigned int newColor = getTierColor(newTier);

	//段階 1: 0-30f 画面全体に赤フラッシュ (alpha 100 でフェードイン → アウト)
	if (frame < 30) {
		int alpha = (frame < 15) ? (frame * 100 / 15) : ((30 - frame) * 100 / 15);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorError, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	//段階 2: 30-90f 「DEMOTED...」をフォント 60 ColorError で
	if (frame >= 30 && frame < 90) {
		SetFontSize(60);
		const char* msg = "DEMOTED...";
		int msgLen = (int)strlen(msg);
		int msgW = GetDrawStringWidth(msg, msgLen);
		DrawString(640 - msgW / 2, 300, msg, ColorError);
	}

	//段階 3: 90-120f 新ティア章を半径 60 で表示
	if (frame >= 90) {
		DrawCircle(640, 450, 60, newColor, TRUE);
		DrawCircle(640, 450, 60, ColorWhite, FALSE);
		SetFontSize(36);
		const char* name = getTierName(newTier);
		int nameLen = (int)strlen(name);
		int nameW = GetDrawStringWidth(name, nameLen);
		DrawString(640 - nameW / 2, 530, name, newColor);
	}

	SetFontSize(oldFontSize);
}

//=========================================================================
//X キーエッジ検出ヘルパ (1.6.1 polish で追加、シーン遷移時の押しっぱなし連鎖を防止)
//=========================================================================

//X キーが今フレームで新しく押されたかどうかを返す (前フレーム非押下 → 今フレーム押下)
//関数内 static で前フレーム状態を保持、シーン間を跨いでも正しく動作する:
//シーン遷移はフレーム末尾なので、新シーンの最初の move/render で curX=1 / prevX=1 → edge=false
//これにより BoardSize→OPTIONS→MENU 連鎖と RANK_UP/DEMOTED スキップ→FINISHED→MENU 連鎖の両方を解消
bool isXKeyJustPressed(void)
{
	static int prevX = 0;
	int curX = CheckHitKey(KEY_INPUT_X);
	bool edge = (curX == 1 && prevX == 0);
	prevX = curX;
	return edge;
}

//=========================================================================
//対局中断機能 (1.6.1 で追加、項目 4: Q キー 2 段階確認、XP 計上なし)
//=========================================================================

//Q キーのエッジ検出 + 2 段階確認タイマー管理 (3 シーン共通)
//active=true ならミッドゲーム中、false なら強制 0 クリア (FINISHED/RANK_UP 中などに残らない)
//1 回目 Q → タイマー 180f (3 秒) セット、2 回目 Q → 戻り値 true (中断確定)
bool tryAbortMidGame(int* abortConfirmTimer, bool active)
{
	//Q キーエッジ検出: 関数内 static で前フレーム状態を保持
	//同フレームに複数 Game シーンが active になることはない (シーンディスパッチは 1 系統)
	static int prevQ = 0;
	int curQ = CheckHitKey(KEY_INPUT_Q);
	bool qEdge = (curQ == 1 && prevQ == 0);
	prevQ = curQ;

	if (!active)
	{
		//非アクティブ時はタイマー強制リセット (誤って残らないように)
		*abortConfirmTimer = 0;
		return false;
	}

	//タイマー減衰 (3 秒 = 180f カウントダウン)
	if (*abortConfirmTimer > 0) (*abortConfirmTimer)--;

	if (qEdge)
	{
		if (*abortConfirmTimer > 0)
		{
			//2 回目 Q (180f 以内): 中断確定
			*abortConfirmTimer = 0;
			return true;
		}
		else
		{
			//1 回目 Q: 確認モード入り (180f 制限時間)
			*abortConfirmTimer = 180;
		}
	}
	return false;
}

//中断ガイドと確認オーバーレイの描画 (active=true 時のみ「Q: 中断」常時 + タイマー>0 で中央オーバーレイ)
//guideY は Game2 ラウンド 1 FINISHED 時の finish msg (Y=410 から 3 行) との被り回避のため呼出側で 350 に上書き可能
void rbDrawAbortGuide(int abortConfirmTimer, bool active, int guideY)
{
	if (!active) return;	//非アクティブ時は何も描画しない

	int oldFontSize = GetFontSize();

	//常時ガイド「Q: 中断」(右パネル下部、フォント 28 で控えめに)
	//デフォルト Y=420 は Game1/2/3 共通の空き領域 (Game3 R:待った Y=350+40=390 から 30px 下、ランク章 Y=480 から 32px 上)
	//Game2 ラウンド 1 FINISHED 時のみ Y=350 に切替 (Round 表示 Y=240+40=280 と finish msg Y=410 の間に収める)
	SetFontSize(28);
	DrawString(PANEL_X, guideY, "Q: 中断", ColorSky);

	//確認モード中の中央オーバーレイ (1 回目 Q 押下後の 3 秒間)
	if (abortConfirmTimer > 0)
	{
		//半透明黒背景の枠 (rbDrawResultOverlay と同方式、盤面エリア内 X=100..630, Y=310..460)
		const int boxX1 = 100, boxY1 = 310, boxX2 = 630, boxY2 = 460;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
		DrawBox(boxX1, boxY1, boxX2, boxY2, GetColor(20, 20, 20), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawBox(boxX1, boxY1, boxX2, boxY2, ColorWarn, FALSE);

		const int centerX = (boxX1 + boxX2) / 2;	//= 365、盤面中心と一致

		//「中断しますか?」フォント 48 ColorWarn (中央寄せ)
		SetFontSize(48);
		const char* q = "中断しますか?";
		int qW = GetDrawStringWidth(q, (int)strlen(q));
		DrawString(centerX - qW / 2, boxY1 + 15, q, ColorWarn);

		//「もう一度 Q で確定」フォント 28 (中央寄せ、白)
		SetFontSize(28);
		const char* g1 = "もう一度 Q で確定";
		int g1W = GetDrawStringWidth(g1, (int)strlen(g1));
		DrawString(centerX - g1W / 2, boxY1 + 80, g1, ColorWhite);

		//「他のキーで自動キャンセル」フォント 28 (中央寄せ、白)
		const char* g2 = "(他のキーで自動キャンセル)";
		int g2W = GetDrawStringWidth(g2, (int)strlen(g2));
		DrawString(centerX - g2W / 2, boxY1 + 110, g2, ColorWhite);
	}

	SetFontSize(oldFontSize);
}
