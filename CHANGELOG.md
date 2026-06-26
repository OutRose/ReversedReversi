# CHANGELOG

本ファイルは [まきもどリバーシ (Reverse Reversi)](Project2/) の変更履歴です。形式は [Keep a Changelog](https://keepachangelog.com/ja/1.1.0/) に準拠し、バージョンは [Semantic Versioning](https://semver.org/lang/ja/) に従います。

バージョン採番ルール (本プロジェクト固有):

- **MAJOR** — オリジナル (1.x) は当面据え置き
- **MINOR** — リファクタリングフェーズ転換ごとに +1 (α=1.2 / β=1.3 / γ=1.4 / δ=1.5 / 以降未定)
- **PATCH** — 各フェーズ内のサブターゲット完了ごとに +1

詳細な作業ログは [CLAUDE.md §13 過去の整理作業履歴](CLAUDE.md) を参照。

---

## [1.5.5] - 2026-06-26

### Added

- [CHANGELOG.md](CHANGELOG.md) を新規作成 (本ファイル)、過去 4 フェーズの変更履歴を遡って記録
- バージョン採番ルールを CHANGELOG 冒頭に明文化 (フェーズ別 MINOR + サブターゲット PATCH)

### Changed

- ウィンドウタイトル `Reverse Reversi 1.1` → `Reverse Reversi 1.5.5` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.1` → `Ver 1.5.5` ([MenuScene.cpp](Project2/MenuScene.cpp))
- メニュー DxLib 版数表示 `DX-Library 3.22a` → `DX-Library 3.24f` ([MenuScene.cpp](Project2/MenuScene.cpp))。実装インストール (`C:\DxLib`) の `DxLib.h` 内 `DXLIB_VERSION_STR_T` から取得

### Fixed

- **`lnt-uninitialized-local` 警告抑制** ([GameSceneMain.cpp](Project2/GameSceneMain.cpp)) — VS コード解析が指摘していた未初期化ローカル配列 4 件を zero-init `= {}` に変更
  - `rbPutPiece` 内 `int wx[BOARD_SIZE], wy[BOARD_SIZE];` (line 173)
  - `rbThinkRandom` 内 `int candX[BOARD_SIZE * BOARD_SIZE], candY[BOARD_SIZE * BOARD_SIZE];` (line 258, 259)
  - 既存ロジックは `wn`/`candCount` で書込済範囲を管理しており未初期化領域は読み出されないため動作不変、defensive coding の改善

---

## [1.5.4] - 2026-06-26

### Added

- **Game3 ヒントマスへの取得コマ数表示** — `rbDrawHints` を 2 パス構成に拡張。パス 1 = 半透明オレンジ丸、パス 2 = `rbPutPiece` 戻り値を `GetDrawFormatStringWidth` で中央寄せして白で重ね描画 ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- 新規定数 `HINT_GAIN_FONT_SIZE = 20` ([GameMain.h](Project2/GameMain.h))
- フォントサイズ退避/復元 (`GetFontSize` → `SetFontSize`) で後続描画への副作用を遮断

---

## [1.5.3] - 2026-06-26

### Added

- **Game3 (あまちゃん次元) ヒント表示** — プレイヤー手番中のみ、置けるマスを 50% 半透明オレンジ丸 (RGB 255, 165, 0) でハイライト
- 新規関数 `rbDrawHints(ReversiBoard*, int turn)` ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- Game3 `renderGame3Scene` PLAYING フェーズで条件付き呼び出し (`status == GAME_STATUS_PLAYING && turn == GAME_TURN_BLACK`)。CPU 手番中・メッセージ中は混乱を招くため非表示

---

## [1.5.2] - 2026-06-26

### Changed (δ-4)

- **描画 6 ブロックの共通関数化** — `renderGame1Scene` / `renderGame2Scene` のクローン描画コードを `rbDraw*` 関数群に集約
- 新規関数 `rbDrawBoard(boardBgColor)` / `rbDrawGrid()` / `rbDrawPieces(state, pieces)` / `rbDrawMsg(state, status)` / `rbDrawCountPanel(state)` / `rbDrawTurnIndicator(turn)` ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- Game1Scene::renderGame1Scene 100 → 17 行、Game2Scene::renderGame2Scene 130 → 39 行に圧縮
- 盤面色は引数化 (Game1=暗緑 / Game2=明緑)

### Removed

- `<stdlib.h>` include を Game1Scene/Game2Scene から削除 (`_itoa_s` が `rbDrawCountPanel` 経由になったため不要)

---

## [1.5.1] - 2026-06-26

### Added (δ-3)

- **x64 構成** — Debug|x64 / Release|x64 を [Project2.sln](Project2.sln) と [Project2.vcxproj](Project2/Project2.vcxproj) に追加 (PlatformToolset v145、Win32 と同設定)
- 新規ファイル [Project2/Common.props](Project2/Common.props) — `<DxLibDir>C:\DxLib</DxLibDir>` UserMacro で DxLib パスを 1 箇所に集約、4 構成全てから Import

### Fixed

- **C4267 警告解消** — x64 ビルドで `std::string::size()` (size_t 64-bit) を `GetDrawStringWidth` (int) に渡す箇所を `(int)state->msg.size()` で明示キャスト ([GameSceneMain.cpp `rbDrawMsg`](Project2/GameSceneMain.cpp))

### Removed

- [Project2.vcxproj](Project2/Project2.vcxproj) から `AdditionalIncludeDirectories` / `AdditionalLibraryDirectories` 直接指定を撤去 (Common.props に集約)
- 死蔵 PropertyGroup `<LibraryPath>$(LibraryPath)</LibraryPath>` を削除

---

## [1.5.0] - 2026-06-26

### Fixed (δ-2)

- **マウス入力安全化** — `rbThinkPlayer` ([GameSceneMain.cpp](Project2/GameSceneMain.cpp)) に範囲ガードを追加し盤面外クリック時の配列範囲外アクセス UB を解消
  - 負値ガード: `if (mx < 0 || my < 0) return false;` (整数除算のゼロ向き切り捨て対策)
  - 上限ガード: `if (bx >= BOARD_SIZE || by >= BOARD_SIZE) return false;`
- 右パネル領域や下部メッセージ領域へのクリックは安全に無視される

---

## [1.4.1] - 2026-06-26

### Added (γ-3)

- **Game3 (あまちゃん次元) 独自モード化完了** — 2 フェーズ構造 (`GAME3_PHASE_NAME_ENTRY` → `GAME3_PHASE_PLAYING`) に再構築 ([Game3Scene.cpp](Project2/Game3Scene.cpp))
- 新規関数 `rbThinkRandom(ReversiBoard*, int turn)` — 置ける場所からランダム選択する**弱い CPU** 思考、初心者向け ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- 対局画面の右パネルに `PLAYER: <nameTmp>` 表示 (`ColorSky`)
- メニュー復帰経路: 名前入力フェーズで X キー、対局終了で X キー (TwistTimeStopper 流)

### Fixed

- **1 文字入力で即遷移するバグ解消** — 元コード `if (nameTmp[0] != NULL) changeScene(SCENE_GAME2)` を、Enter キー検出 + `nameTmp[0] != '\0'` 判定に変更
- `nameTmp` バッファクリアを `initGame3Scene` で実施 (再入場時の前回値残留解消)

---

## [1.4.0] - 2026-06-25

### Changed (γ-1)

- **Game1Scene / Game2Scene フレーム駆動化** — 独自の `while (!ProcessMessage())` 無限ループを撤去、TwistTimeStopper 流のフレーム駆動モデルに書き直し (本プロジェクト最大の構造的不具合解消)
- 状態 (`status` / `turn` / `CurrentRound` / `excuted` / `roundTransitWait` / `finishedMsgRand` / `ReversiBoard state` / `pieces[]`) をファイルスコープ `static` 化
- `initGame1Scene` / `initGame2Scene` でシーン入場/再入場時に毎回リセット + リソース読込
- `renderGame1Scene` / `renderGame2Scene` を新設し描画コードを分離
- Game2 ラウンド遷移を `roundTransitWait` カウンタ (240 フレーム ≒ 4 秒) で実装 (旧 `WaitTimer(4000)` 相当)

### Fixed

- **シーン再入場リセット欠落** — メニューから再エントリ時に盤面・ターン・ラウンドが前回値のまま残る不具合を解消
- **Game1 終了時 `WaitTimer(2000)` フレームハング** 撤去
- **Game2 ラウンド 1 終了メッセージのフリッカー** — `finishedMsgRand` で 1 回固定抽選
- **`releaseGame2Scene` 内 `DxLib_End` 直呼び** 撤去 (γ-2 副次解消)
- **リソースリーク** — `LoadDivGraph` を init、`DeleteGraph` を release に移動 (`static int pieces[2] = { -1, -1 };` で再入場二重ロード防止) (δ-1 前倒し)
- **BGM 鳴り続け** — `releaseXxxScene` で `StopSoundFile` 追加

### Added

- 終了状態で X キー押下 → `changeScene(SCENE_MENU)` でメニュー復帰経路を追加
- 終了メッセージ文言を「Xキーでメニュー\nESCキーで終了」に更新

---

## [1.3.6] - 2026-06-25

### Changed (β-D-5)

- **盤面ロジックの単一化** — Game1Scene と Game2Scene のクローンコード約 80% を `ReversiBoard` 構造体 + `rb*` 関数群に統合
- 新規構造体 `_ReversiBoard { int board[BOARD_SIZE][BOARD_SIZE]; std::string msg; int msg_wait; }` ([GameSceneMain.h](Project2/GameSceneMain.h))
- 新規 9 関数 — `rbInit` / `rbPutPiece` / `rbIsPass` / `rbThinkPlayer` / `rbThinkCpu` / `rbSetMsg` / `rbCheckResult` / `rbCountPieces` / `rbRemovePieces` ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- Game1Scene 371 → 203 行、Game2Scene 452 → 273 行で合計 -157 行
- グリッド描画 (`5/580/48/11`) を `BOARD_ORIGIN_X / BOARD_END_PX / CELL_PX / BOARD_SIZE_MAX` に置換

### Removed

- 旧クローン関数群: `putPiece` / `putPiece2` / `isPass` / `isPass2` / `think1` / `think2` / `think01` / `think02` / `setMsg` / `setMsg2` / `checkResult` / `checkResult2` / `removePiece`
- 旧グローバル: `SqBoard` / `SqBoardA` / `msg` / `msg2` / `msg_wait` / `msg_wait2`

---

## [1.3.5] - 2026-06-25

### Changed (β-D-4)

- **マジックナンバー定数化** — 画面・FPS・盤面サイズ・主要レイアウト座標を [GameMain.h](Project2/GameMain.h) に集約
- 画面: `SCREEN_WIDTH=800` / `SCREEN_HEIGHT=700` / `SCREEN_BPP=16`
- FPS: `FPS=60` / `MS_PER_SEC=1000`
- 盤面: `BOARD_SIZE=12` / `BOARD_SIZE_MAX=11` / `CELL_PX=48` / `BOARD_ORIGIN_X/Y=5` / `BOARD_END_PX=580` / `PIECE_SIZE_PX=47` / `BOARD_CENTER_LOW=5` / `BOARD_CENTER_HIGH=6`
- パネル/メッセージレイアウト座標多数 (`PANEL_X 590`, `MSG_BOX_CENTER_X 192` 等)
- フォント: `FONT_SIZE_DEFAULT=32`、メッセージ表示時間 `MSG_WAIT_FRAMES=60`

---

## [1.3.4] - 2026-06-25

### Added (β-D-3)

- `MyOutputDebugString` マクロを [GameMain.h](Project2/GameMain.h) に追加 (TwistTimeStopper 流)
- Debug ビルドでのみ `OutputDebugString` を発火、Release は空展開、`_stprintf_s` セキュア版
- `<stdio.h>` / `<Windows.h>` / `<tchar.h>` を同ヘッダに集約

---

## [1.3.3] - 2026-06-25

### Changed (β-D-2)

- **色グローバル集約** — `ColorWhite` / `ColorRed` / `ColorSky` を [GameMain.cpp](Project2/GameMain.cpp) に定義 + [GameMain.h](Project2/GameMain.h) で `extern` 公開
- Game2Scene の `ColorXxx2` 接尾辞 (22 箇所) を撤廃、`ColorXxx` に統一

---

## [1.3.2] - 2026-06-25

### Changed (β-D-1)

- **シーンディスパッチャ共通化** — `SCENE_HANDLERS sceneTable[SCENE_MAX]` 関数ポインタ束 + 1 行ディスパッチ (`sceneTable[sceneNo].xxx()`) に圧縮
- 5 つの `switch (sceneNo)` を関数ポインタテーブル参照に置換
- `prevScene` 削除 (failure 検知は `assert` で代替)
- `initCurrentScene` の `BOOL` 戻り値を活用、`FrameMove` で SCENE_MENU フォールバック (それも失敗なら SCENE_NONE で諦め、無限ループ防止)

### Removed

- `MessageBox` 5 箇所撤去 → `MyOutputDebugString` + `assert` (Debug 停止 / Release 無視継続)

---

## [1.3.1] - 2026-06-25

### Added (β-C)

- **状態値の enum 化** — [GameSceneMain.h](Project2/GameSceneMain.h) に 3 つの enum 追加
  - `GAME_STATUS` (PLAYING=1 / TURN_MSG / PASS_MSG / FINISHED)
  - `GAME_TURN` (BLACK=1 / WHITE=2)
  - `GAME_ROUND` (FIRST=1 / SECOND=2)
- Game1Scene / Game2Scene の数字リテラル比較・代入を全置換 (動作不変)

---

## [1.3.0] - 2026-06-25

### Changed (β-A)

- **`itoa` セキュア化** — `itoa(...)` × 4 箇所を `_itoa_s(..., sizeof(buf), 10)` に置換 (Game1Scene 2 + Game2Scene 2)

### Removed

- `#include <math.h>` / `#include <stdio.h>` を Game1Scene/Game2Scene から削除、代わりに `#include <stdlib.h>` (`_itoa_s` 用)
- `#pragma warning(disable : 4996)` を [GameMain.h](Project2/GameMain.h) から撤去 (セキュア化により不要)

---

## [1.2.7] - 2026-06-25

### Removed (α-8)

- **dead code コメント整理**
- [GameMain.cpp](Project2/GameMain.cpp) — `/* ... */` 旧 MessageBox ウィンドウモード確認ブロック削除
- Game1Scene.cpp / Game2Scene.cpp — `//int back;` / `//back = LoadGraph(...)` / `//SetDrawBlendMode` / `//DrawGraph(100, ...)` / `/*テスト用：マウス座標取得 ... //テスト用ここまで*/` ブロック削除

---

## [1.2.6] - 2026-06-25

### Changed (α-7)

- **シーン番号詰め + 雛形保存** — 旧 SCENE_GAME4 (まきもどり) → 新 SCENE_GAME2、旧 SCENE_GAME5 (あまちゃん) → 新 SCENE_GAME3、旧 SCENE_GAME2 (死蔵スタブ) → 新 SCENE_GAME4 (空シーン雛形)
- `menu[]` 配列を `{ SCENE_GAME1, SCENE_GAME3, SCENE_GAME2 }` に更新 (表示順はふつう/あまちゃん/まきもどり不変)

### Removed

- 旧 Game3Scene.cpp/.h (誤コピペ「ゲーム画面１です」持ち死蔵スタブ) を削除
- enum `SCENE_GAME5` を削除、各 switch から `case SCENE_GAME5:` を削除

---

## [1.2.5] - 2026-06-25

### Changed (α-6)

- **インクルードガード正規化** — `GameSceneMain.h` を `__GAMESCENEMAIN_H_` (予約識別子 `__` 前置 + `_` 後置) → `GAMESCENEMAIN_H_` に改名
- 他 8 ファイル (ガード自体が無かった) に新規ガード追加 (`MENUSCENE_H_`, `GAMESCENE<N>_H_`, `GAMESTATUS_H_` 等)
- `GameMain.h` は `#pragma once` (TwistTimeStopper の例外規約)

---

## [1.2.4] - 2026-06-25

### Removed (α-5)

- `Project2/Debug/Project2.vcxproj.FileListAbsolute.txt` を削除 (旧 `D:\Visual Studio Repository\Scene管理付き空プロジェクトRev2\...` パスが残存していた)
- MSBuild が次回ビルドで新パス基準に自動再生成

---

## [1.2.3] - 2026-06-25

### Removed (α-4)

- 未使用グローバル削除
- [MenuScene.cpp](Project2/MenuScene.cpp) の `int startfont;` (参照ゼロ)
- [GameMain.cpp](Project2/GameMain.cpp) の `int game_status = GAMETITLE;` (書き込み専用、参照ゼロ)
- `GameStatus.h` 本体のマクロ群は将来用 placeholder として保持

---

## [1.2.2] - 2026-06-25

### Fixed (α-3)

- **`GameStatus.h` ビットフラグ値修正**
- `GAMECLEAR 0x00000016` (=22) → `0x00000010`
- `GAMEEND 0x00000032` (=50) → `0x00000020`
- 既存マクロ `GAMEINIT=0x01` / `GAMETITLE=0x02` / `GAMEMAIN=0x04` / `GAMEOVER=0x08` の倍々ビット進行と整合

---

## [1.2.1] - 2026-06-25

### Fixed (α-2)

- **Release 構成のビルド復活** — [Project2.vcxproj](Project2/Project2.vcxproj) の Release 構成 DxLib パスを `C:\DxlibFile` (不在) → `C:\DxLib` に統一
- `AdditionalIncludeDirectories` と `AdditionalLibraryDirectories` の 2 行を修正、Debug 構成と同一インストール参照に整合
- C1083 ×8 件解消、`Release\Project2.exe` 6,050,816 bytes 生成可能に

---

## [1.2.0] - 2026-06-25

### Changed (α-1)

- **エンコーディング統一 (UTF-8 BOM + CRLF)** — Project2/ 配下 17 ファイル (.cpp 8 + .h 9) を CP932 → UTF-8 BOM + CRLF に変換
- ダメ文字 (`ソ/表/能/予` など 2 バイト目が `0x5C` の文字) による C4129/C2059/C2143 連鎖の予防

### Added

- ルート直下に [.editorconfig](.editorconfig) を新設 (`charset = utf-8-bom` / `end_of_line = crlf` / `indent_style = tab` / `insert_final_newline = true` / `trim_trailing_whitespace = true`)
- TwistTimeStopper と完全同一設定

---

## [1.1.0] - 2021-11-16 〜 2022-11-15

### Initial release

オリジナルリリース。「Scene 管理付き空プロジェクト Rev2」テンプレートをベースに制作。

- 12×12 リバーシ盤面
- 3 モード: ふつうの次元 / あまちゃん次元 / まきもどり次元
  - ふつう: プレイヤー vs CPU 貪欲思考、完成
  - あまちゃん: 名前入力 → まきもどりに遷移する仮実装 (未完成)
  - まきもどり: 2 ラウンド制、ラウンド間で 96 マスをランダム削除、BGM 切替、完成
- DxLib 3.22a、PlatformToolset v145、MultiByte、`/MT`、Win32 のみ、800x700 / 16bit
- 主要コミット: `a1e0559` (Add files via upload), `b3bccff` (CodeMaid 文体整理), `ffd4e52` (クラスダイアグラム), `843465b` (WinMain 注釈修正), `03e45af` (CodeMaid クリーンアップ)

---

[1.5.5]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.5
[1.5.4]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.4
[1.5.3]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.3
[1.5.2]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.2
[1.5.1]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.1
[1.5.0]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.0
[1.4.1]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.4.1
[1.4.0]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.4.0
[1.3.6]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.3.6
[1.3.5]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.3.5
[1.3.4]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.3.4
[1.3.3]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.3.3
[1.3.2]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.3.2
[1.3.1]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.3.1
[1.3.0]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.3.0
[1.2.7]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.7
[1.2.6]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.6
[1.2.5]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.5
[1.2.4]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.4
[1.2.3]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.3
[1.2.2]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.2
[1.2.1]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.1
[1.2.0]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.2.0
[1.1.0]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.1.0
