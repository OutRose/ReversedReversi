# CHANGELOG

本ファイルは [まきもどリバーシ (Reverse Reversi)](Project2/) の変更履歴です。形式は [Keep a Changelog](https://keepachangelog.com/ja/1.1.0/) に準拠し、バージョンは [Semantic Versioning](https://semver.org/lang/ja/) に従います。

バージョン採番ルール (本プロジェクト固有):

- **MAJOR** — オリジナル (1.x) は当面据え置き
- **MINOR** — リファクタリングフェーズ転換ごとに +1 (α=1.2 / β=1.3 / γ=1.4 / δ=1.5 / 以降未定)
- **PATCH** — 各フェーズ内のサブターゲット完了ごとに +1 (挙動変化を伴う場合)
- **SUBPATCH (4 セグメント目)** — コードのみの変更で挙動が不変な場合、PATCH を上げず SUBPATCH を +1 する (例: 新規 extern 定数を追加するが本リリースでは未使用、内部リファクタで挙動同一)。CHANGELOG エントリは作成するが視認できる挙動変化はないことを明記。形式は `MAJOR.MINOR.PATCH.SUBPATCH` (例: 1.5.6.1)
- **据え置き** — ドキュメント / メタファイル (CHANGELOG / README / LICENSE / CLAUDE.md 等) のみの変更ではバージョンを上げない。Keep a Changelog 流の `[Unreleased]` セクションに記録し、次の本体変更リリースに巻き込まれて公開される

詳細な作業ログは [CLAUDE.md §13 過去の整理作業履歴](CLAUDE.md) を参照。

---

## [1.5.9.2] - 2026-06-27

### Fixed

- **MenuScene の Credits 表示見切れ修正** ([MenuScene.cpp:88](Project2/MenuScene.cpp)): 1.5.9 で 1280×768 化したメニューの Credits (画面右下、開始 x=820) が 1.5.9.1 のプレイテストで URL 末尾 (".com/") が画面右端で切れる事象を確認。フォント 28 では 1 文字あたり実描画幅 ~16px となり、URL 31 文字 (約 496px) が右側余白 460px (= 1280 - 820) に収まらなかった
- **対策**: `SetFontSize(28)` → `SetFontSize(22)` に下げて 1 文字あたり ~12.6px に縮小、URL 31 文字 ≈ 391px で 460px 余白に 69px マージン込みで収まる。5 行 × 22 = 110px 縦使用 (旧 140px から 30px 圧縮)、y=580 開始 → 690 終了で 768 内に余裕

### Changed

- ウィンドウタイトル `Reverse Reversi 1.5.9.1` → `Reverse Reversi 1.5.9.2` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.5.9.1` → `Ver 1.5.9.2` ([MenuScene.cpp](Project2/MenuScene.cpp))

### Notes

- **挙動変化は Credits フォントサイズ縮小のみ** — タイトル / メニュー項目 / 各シーンの描画・ロジックはすべて 1.5.9.1 と完全同一
- 1.5.9.1 で追加したカラーパレット 10 色は引き続き全コードから未参照の伏線として保持
- 視覚的な fit 修正 (機能追加/削除なし) のため SUBPATCH バンプ、1.5.9.1 → 1.5.9.2

---

## [1.5.9.1] - 2026-06-27

### Added

- **カラーパレット拡張 10 色** を [GameMain.cpp](Project2/GameMain.cpp) 定義 + [GameMain.h](Project2/GameMain.h) extern 宣言として追加 (本リリースでは未参照の伏線、SUBPATCH バンプ対象)
  - **ランク章 +6 色** (B2 プレイヤーランクシステム 10 ティア対応の余裕枠): `ColorIron` (110,120,140、Bronze 下位エントリー) / `ColorDiamond` (130,240,255、Platinum 上位の宝石系トップ) / `ColorEmerald` (50,200,130、ミントグリーン寄り) / `ColorRuby` (224,50,95、深紅ピンク寄り、ColorRed=純赤=「選択中」用と区別) / `ColorSapphire` (40,90,200、深サファイア青) / `ColorAmethyst` (170,100,200、アメジスト紫、既存色に紫系なし唯一性高)
  - **汎用 UI +4 色** (テーマ化伏線、状態通知の用途分離): `ColorSuccess` (40,180,60、純緑寄り、Emerald ミントと用途分離) / `ColorError` (255,80,80、警報赤、ColorRed と用途分離) / `ColorInfo` (120,180,240、明るい青、Sapphire 深青/Hover 淡水色と段階分離) / `ColorAccent` (255,130,200、マゼンタ寄り、既存にピンク系なし)
- 既存 `ColorBronze` / `Silver` / `Gold` / `Platinum` (4 色、1.5.6.1) と組み合わせて **計 10 ランク色** (LoL 9 ティア / Overwatch 7 ティア / 将棋系 9 段位どれにも対応可)。CLAUDE.md §10 のラフ案「NOVICE → APPRENTICE → ADEPT → EXPERT → MASTER → GRAND MASTER」(6 段) なら 4 色余る計算

### Changed

- ウィンドウタイトル `Reverse Reversi 1.5.9` → `Reverse Reversi 1.5.9.1` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.5.9` → `Ver 1.5.9.1` ([MenuScene.cpp](Project2/MenuScene.cpp))

### Notes

- **挙動完全不変** — 新規 extern 10 色はいずれの描画コードからも参照されない伏線。全シーンの描画 / メニュー操作 / 対局進行 / 「待った」 / OPTIONS / settings.ini 永続化はすべて 1.5.9 と完全同一
- ファイルスコープ動的初期化: 新規 `GetColor()` 10 件は既存 10 件と同様に `SetGraphMode` 前で走るが、RGB 値が 16/24/32 bit いずれでも一意の結果を返す範囲なので動作不変 ([GameMain.cpp:17-19](Project2/GameMain.cpp#L17-L19) のコメント参照)
- **`ColorXxx` extern 総数 10 → 20** (ランク章 4 → 10 / 汎用 UI 3 → 7 / 基本 3 不変)
- 既存インライン `GetColor()` 呼び出し (盤面色 `(0,100,20)`/`(0,140,20)`、メッセージ箱グレー `(150,150,150)`、ヒントオレンジ `(255,165,0)`、OPTIONS プレビュー駒、Menu/Template の白赤リテラル) の extern 化は CLAUDE.md §10 未対応事項のまま継続
- CLAUDE.md §10 項目 2「カラーパレット増設 (継続的)」の継続的タスク方針に沿った第 2 弾。今後も色追加が必要になり次第、SUBPATCH または機能側 PATCH に巻き込んで継続

---

## [1.5.9] - 2026-06-27

### Added

- **画面サイズ拡張 800×700 → 1280×768** + **全シーンリレイアウト** — 1.5.6 (待った) → 1.5.7 (OPTIONS) → 1.5.8 (盤面サイズ縮小) と Game3 拡張機能を蓄積するうちに 800×700 が窮屈になり、OPTIONS シーンでプレビュー領域とトグル列が完全に被って読めなくなっていた問題を抜本対策
  - 画面解像度: SCREEN_WIDTH 800 → **1280** (1.6× 拡張)、SCREEN_HEIGHT 700 → **768** (1.097× 拡張、Windows DPI 125% の 1080p ディスプレイでも見切れない安全値) ([GameMain.h](Project2/GameMain.h))
  - 盤面描画ターゲット領域: 576×576 → **720×720** (1.25× 拡張) ([GameMain.h](Project2/GameMain.h) `BOARD_TARGET_PX` 新規定数追加)
  - 6×6 セル 96px → **120px** (グリッド線間隔広がり、視覚的「粗さ」解消)
  - 8×8 セル 72 → **90px**、10×10 セル 57 → **72px**、12×12 セル 48 → **60px**
  - フォントサイズデフォルト 32 → **40** (1.25× 拡張)
  - メッセージ用専用フォント `MSG_FONT_SIZE = 28` を新設 ([GameMain.h](Project2/GameMain.h))、`rbDrawMsg` 内で局所的に SetFontSize 退避/復元、768px 高に msg を収める
- OPTIONS プレビュー領域は **320×320 で維持** (1.5.8 と同サイズ、位置を (450,100) → (810,200) に移動、トグル列との水平距離 240px 確保で被り解消) ([OptionsScene.cpp](Project2/OptionsScene.cpp))

### Changed

- 全主要レイアウト定数を 1280×768 用に更新 ([GameMain.h](Project2/GameMain.h)):
  - `BOARD_ORIGIN_Y` 5 → 5 (1.5.9 当初 10 で計画、最終 768px 化で 5 に戻し) / `BOARD_END_PX` 580 → 725 / `PIECE_SIZE_PX` 不変 (47、ソース画像サイズ)
  - パネル: `PANEL_X` 590 → 745 / `PANEL_TURN_X` 680 → 900 / `PANEL_ROUND_LABEL_Y` 220 → 240 (font 40 で WHITE_VALUE 175..215 との重なり解消) / `PANEL_END_MSG_Y` 330 → 410 (Game2/Game3 終了メッセージ、Game3 R:待った 350..390 と独立) / `PANEL_END_MSG_GAME1_Y` 150 → 240 (Game1 終局時 WHITE_VALUE 175..215 と 25px gap 確保)
  - メッセージ箱: `MSG_BOX_CENTER_X` 192 → 365 (盤面中心追従) / `MSG_BOX_Y_TOP` 630 → 730 / `MSG_BOX_Y_BOTTOM` 655 → 762 / `MSG_TEXT_Y` 620 → 728 (盤面下端 725 直下にタイトに収める)
  - フォント: `FONT_SIZE_DEFAULT` 32 → 40 / 新規 `MSG_FONT_SIZE` 28 (rbDrawMsg 専用) / `HINT_GAIN_FONT_SIZE` 20 → 24 (上限固定値、動的算出 `cellPx * 5 / 12` は維持)
- `rbInit` の cellPx 計算を `(size == 10) ? 57 : (576 / size)` → `BOARD_TARGET_PX / size` (720/size) に簡略化、10×10 特殊ケース撤廃 ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- `rbInit` の `originX/originY` を中央寄せ補正計算 → `BOARD_ORIGIN_X / BOARD_ORIGIN_Y` 固定に簡略化 (全サイズが綺麗に割り切れるため、補正不要) ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- `rbDrawMsg` 内に `SetFontSize(MSG_FONT_SIZE)` ↔ `SetFontSize(oldFontSize)` の退避/復元処理を追加、msg を 28px フォントで描画して 768px 高に収める ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- MenuScene のタイトル位置 (120,50) → (280,60)、メニュー項目開始 (130,140,gapY=80) → (260,170,gapY=90)、Credits (160,480) → (820,580) に再配置 + Credits は `SetFontSize(28)` で 5 行 140px 縦使用に圧縮 ([MenuScene.cpp](Project2/MenuScene.cpp))
- Game3Scene NAME_ENTRY フェーズの座標一式を 1280×768 用に再配置 (タイトル 280→480 / 指示 160,100→280,180 / 入力欄 270,280→490,410 / 下線 265,330,515→485,470,795 / 完了案内 50,370→120,540 / X ガイド 50,600→120,700) + フォントサイズも 45→55 / 30→36 / 40→50 に拡張 ([Game3Scene.cpp](Project2/Game3Scene.cpp))
- Game3Scene PLAYING フェーズの右パネル line spacing 修正 ([Game3Scene.cpp](Project2/Game3Scene.cpp)): PLAYER 名 nameTmp は `PANEL_ROUND_LABEL_Y + 35` → `+ 50` (font 40 の高さに追従、PLAYER:y=240..280 と nameTmp y=290..330 の 5px 重なり解消)、「R: 待った」ガイドは `+ 70` → `+ 110` (nameTmp との 20px gap 確保 + `PANEL_END_MSG_Y=410` とも独立)
- OptionsScene のタイトルフォント 45→50、位置 (120,50)→(180,40)、項目列 (130,180,gapY=60)→(130,140,gapY=70)、操作ガイドを y=570 + SetFontSize(28) で 4 行 112px 縦使用に再配置 ([OptionsScene.cpp](Project2/OptionsScene.cpp))
- OptionsScene `renderBoardPreview` のプレビューサイズ 320 維持、位置 (450,100)→(810,200)、ラベルフォント 20→22 ([OptionsScene.cpp](Project2/OptionsScene.cpp))
- GameSceneTemplate の Template Scene 描画位置を (30,50)/(30,100) → (60,100)/(60,200) に調整 ([GameSceneTemplate.cpp](Project2/GameSceneTemplate.cpp))
- ウィンドウタイトル `Reverse Reversi 1.5.8` → `Reverse Reversi 1.5.9` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.5.8` → `Ver 1.5.9` ([MenuScene.cpp](Project2/MenuScene.cpp))

### Fixed (1280×768 移行で発見された既存バグも同時解消)

- **Game1 終局時の右パネル要素重なり** ([Game1Scene.cpp](Project2/Game1Scene.cpp) ※ファイル変更なし、定数 `PANEL_END_MSG_GAME1_Y` の値変更で解消): 1.5.8 以前から WHITE カウント数値 (PANEL_WHITE_VALUE_Y=125, font 32 → y=125..157) と Game1 終了メッセージ (PANEL_END_MSG_GAME1_Y=150, font 32 → y=150..246) が y=150..157 の 7px overlap。1.5.9 で font 40 化により 15px overlap (175..215 vs 200..240) に悪化。`PANEL_END_MSG_GAME1_Y` を 200 → 240 に下げて 25px gap 確保 (font 40 line 高に対し十分な余裕)
- **Game3 PLAYER 名と「R: 待った」ガイドの重なり**: 1.5.7 で導入された PLAYER 名表示は `PANEL_ROUND_LABEL_Y + 35` の line spacing で、font 32 当時は 3px gap だったが 1.5.9 の font 40 化で 5px overlap に悪化。+50 に修正
- 多並列 adversarial 検証 (Workflow) で 1280×768 化後の全シーンを 3 エージェント並列でレビューし発見した critical/minor 重なりを修正

### Notes

- **ゲームロジック (rbPutPiece / rbIsPass / rbThinkCpu / rbThinkRandom / rbCheckResult / rbCountPieces / rbRemovePieces) は完全に挙動不変** — `state->size` / `cellPx` / `originX/Y` の値が変わるだけ、関数本体のコードは変更なし
- 思考テーブル / Game1/2/3 の状態遷移 / 「待った」機能 / OPTIONS の 4 トグル仕様 / settings.ini フォーマットも完全不変
- 1.5.8 形式の `settings.ini` (showHints/showGain/weakCpu/allowUndo/boardSize) はそのまま読込可能、互換性維持
- piece.png (47×47 ソース) は `DrawExtendGraph` で表示サイズ (60/72/90/120px) に拡大表示。約 2.5× 拡大で多少ジャギーが出る可能性あり (将来高解像度差し替え余地、本リリースでは画像維持)
- `SetWindowSizeChangeEnableFlag(true)` で実行中リサイズも引き続き機能
- **768px 高採用の理由**: Windows DPI 125% で 1080p ディスプレイの場合 900px は物理 1125px となりタイトルバー + タスクバー込みで見切れる事例あり。768 で安全マージン確保 (DPI 125% で物理 960px)

---

## [1.5.8] - 2026-06-27

### Added

- **盤面サイズ縮小モード (Game3 専用)** — Game3 (あまちゃん) で 6×6 / 8×8 / 10×10 / 12×12 の 4 段階から盤面サイズを選択できる
  - 新規オプション `boardSize` を `ReversiOptions` に追加 (デフォルト 12、settings.ini で永続化) ([GameSceneMain.h](Project2/GameSceneMain.h) / [GameMain.cpp](Project2/GameMain.cpp))
  - 新規定数 `BOARD_SIZE_CHOICES[] = { 6, 8, 10, 12 }` ([GameSceneMain.h](Project2/GameSceneMain.h) extern + [GameSceneMain.cpp](Project2/GameSceneMain.cpp) 定義)
  - `ReversiBoard` 構造体に `size` / `cellPx` / `originX` / `originY` の 4 フィールドを追加、有効領域は左上 size×size のみ使用 (ストレージは 12×12 固定維持) ([GameSceneMain.h](Project2/GameSceneMain.h))
  - サイズ別メトリクス: 6×6 (cellPx=96) / 8×8 (72) / 10×10 (57、中央寄せ補正で originX/Y=8) / 12×12 (48、現状デフォルト)
  - 初期 4 駒位置を `size/2 - 1` / `size/2` で動的計算 (6→2,3 / 8→3,4 / 10→4,5 / 12→5,6)
- OPTIONS シーン拡張 — 5 行目「盤面サイズ」+ 右側 320×320 盤面プレビュー領域を追加 ([OptionsScene.cpp](Project2/OptionsScene.cpp))
  - **Left/Right 矢印キー** で前/次のサイズへ循環 (5 行目選択時のみ有効)
  - Enter/Space は 1〜4 行目で ON/OFF トグル、5 行目では次サイズへ循環
  - プレビュー領域に現在選択中サイズの盤面 (暗緑 + グリッド + 初期 4 駒) を即時描画
  - ガイド文言を 3 行 → 4 行に拡張 ("↑↓: 選択 / Enter/Space: ON/OFF・サイズ循環 / ←→: サイズ変更 (5 行目) / X: 戻る")
- `settings.ini` に `boardSize=12` キーを追加、`loadOptions` で有効値検証 (6/8/10/12 のみ受理、不正値はデフォルト 12 維持) ([GameMain.cpp](Project2/GameMain.cpp))
- **雛形シーン名固定化** — 旧 `Game4Scene.cpp/.h` を `GameSceneTemplate.cpp/.h` にリネーム、今後の新シーン追加時の永続コピー元として明示化 (ユーザー指示)
  - `SCENE_GAME4` → `SCENE_TEMPLATE` enum 変更 ([GameSceneMain.h](Project2/GameSceneMain.h))
  - 関数 5 つを `initGameSceneTemplate` / `moveGameSceneTemplate` / `renderGameSceneTemplate` / `releaseGameSceneTemplate` / `GameSceneTemplateCollideCallback` に改名
  - インクルードガード `GAMESCENE4_H_` → `GAMESCENETEMPLATE_H_`
  - 描画文字列 `"ゲーム画面４です"` → `"Template Scene"` (雛形であることを明示)
  - Project2.vcxproj / .filters の参照更新

### Changed

- `rbInit` シグネチャ `void rbInit(ReversiBoard*)` → `void rbInit(ReversiBoard*, int size)` — size に応じて cellPx/originX/originY と初期 4 駒位置を自動計算 ([GameSceneMain.h](Project2/GameSceneMain.h) / [GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- 全 `rb*` ロジック関数 (`rbPutPiece` / `rbIsPass` / `rbThinkPlayer` / `rbThinkCpu` / `rbThinkRandom` / `rbCheckResult` / `rbCountPieces` / `rbRemovePieces`) を動的サイズ対応に書き換え — 二重 for ループの境界・GetRand 範囲・配列添字を `state->size` / `state->size - 1` に統一 ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- `rbThinkPlayer` のマウス座標変換を `(mx - state->originX) / state->cellPx` に動的化 — 小盤面 (中央寄せ) でも正しく判定、盤面外クリックは UB なく無視
- `rbDrawBoard` / `rbDrawGrid` シグネチャに `ReversiBoard*` 引数を追加 ([GameSceneMain.h](Project2/GameSceneMain.h) / [GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- `rbDrawPieces` を `DrawGraph` → `DrawExtendGraph` に変更 — `state->cellPx` に応じてコマ画像 (元 47px) を 95〜47px に拡大描画 ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- `rbDrawHints` の丸半径 = `state->cellPx / 3`、取得コマ数フォントサイズ = `state->cellPx * 5 / 12` で動的追従 (12→20 / 10→23 / 8→30 / 6→40) ([GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- Game1Scene / Game2Scene / Game3Scene の `rbInit` 呼出をそれぞれ `rbInit(&state, 12)` / `rbInit(&state, 12)` / `rbInit(&state, g_game3Options.boardSize)` に変更 — Game1/Game2 は完全挙動不変、Game3 のみ動的サイズ
- Game1Scene / Game2Scene / Game3Scene の `rbDrawBoard` / `rbDrawGrid` 呼出を `&state` 引数追加に統一
- ウィンドウタイトル `Reverse Reversi 1.5.7` → `Reverse Reversi 1.5.8` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.5.7` → `Ver 1.5.8` ([MenuScene.cpp](Project2/MenuScene.cpp))

### Notes

- スコープは **Game3 (あまちゃん) 限定** — Game1=ふつう / Game2=まきもどり は `rbInit(&state, 12)` で完全挙動不変、難易度・盤面・思考すべて 1.5.7 と同等
- Game3 デフォルト動作 (boardSize=12) は 1.5.7 と完全一致 — settings.ini を作らず起動しても挙動同じ
- Game3 対局中に OPTIONS でサイズを変更しても、対局中の盤面サイズは init 時に確定したもので継続する。サイズ変更を反映するにはメニューに戻って Game3 を再エントリする必要あり (4 トグルは毎フレーム参照で即時反映だが、boardSize は盤面レイアウトに関わるため不可)
- ストレージは引き続き `board[12][12]` (576B 固定)、メモリ使用量に変化なし
- 雛形シーン名固定化により、今後の新シーン追加手順は `(1) GameSceneTemplate.cpp/.h をコピー → 識別子置換 / (2) SCENE_NO に 1 値追加 / (3) sceneTable[] に 1 行追加` の 3 ステップに統一

---

## [1.5.7] - 2026-06-26

### Added

- **B1: オプション設定のトグル化 (Game3 専用)** — メニューに「OPTIONS」項目を追加、`SCENE_OPTIONS` 新設で 4 つのトグルを ON/OFF 可能に
  - 新規シーン `SCENE_OPTIONS` ([Project2/GameSceneMain.h](Project2/GameSceneMain.h) enum 追加、[GameSceneMain.cpp](Project2/GameSceneMain.cpp) sceneTable[] 拡張)
  - 新規ファイル [OptionsScene.h](Project2/OptionsScene.h) / [OptionsScene.cpp](Project2/OptionsScene.cpp) — 5 関数 (init/move/render/release/CollideCallback)、カーソル移動 + トグル反転 + メニュー復帰
  - 4 トグル: **ヒント表示** (`showHints`) / **取得コマ数表示** (`showGain`) / **弱い CPU** (`weakCpu`、true=`rbThinkRandom` / false=`rbThinkCpu`) / **待った機能** (`allowUndo`)
  - 新規構造体 `ReversiOptions` ([GameSceneMain.h](Project2/GameSceneMain.h))、グローバル `g_game3Options` (デフォルト全 true で現状動作と一致)
- **`settings.ini` ファイル永続化** — 4 トグル状態をテキスト形式で保存、次回起動でも保持
  - 新規関数 `loadOptions()` / `saveOptions()` ([GameMain.cpp](Project2/GameMain.cpp))。`fopen_s` + `sscanf_s` でセキュア、ファイル無し/不正値時はデフォルト維持
  - `loadOptions` は `WinMain` で `InitGame()` 直後に 1 回呼出、`saveOptions` は OPTIONS シーンでトグル変更ごとに呼出
- メニュー項目: `MENU_MAX` 3 → 4、`menu[]` / `menuList[]` に「オプション設定」追加 ([MenuScene.cpp](Project2/MenuScene.cpp))
- Project2.vcxproj / .filters に `OptionsScene.cpp` / `OptionsScene.h` を追加

### Changed

- `rbDrawHints` シグネチャに `bool showGain` パラメータを追加 — false なら Pass 2 (取得コマ数の数字描画) をスキップ、フォントサイズ退避/復元も条件付き ([GameSceneMain.h](Project2/GameSceneMain.h) / [GameSceneMain.cpp](Project2/GameSceneMain.cpp))
- Game3Scene の 4 箇所に `g_game3Options` 参照を埋込 ([Game3Scene.cpp](Project2/Game3Scene.cpp)):
  - moveGame3Scene の R キー判定: `g_game3Options.allowUndo && ...`
  - moveGame3Scene の思考テーブル: `{ rbThinkPlayer, g_game3Options.weakCpu ? rbThinkRandom : rbThinkCpu }`
  - moveGame3Scene のスナップショット persist: `if (g_game3Options.allowUndo && isPlayerTurn) ...`
  - renderGame3Scene のヒント描画: `if (g_game3Options.showHints && ...) rbDrawHints(&state, turn, g_game3Options.showGain);`
  - renderGame3Scene の「R: 待った」ガイド: `g_game3Options.allowUndo && undoAvailable && ...`
- ウィンドウタイトル `Reverse Reversi 1.5.6.1` → `Reverse Reversi 1.5.7` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.5.6.1` → `Ver 1.5.7` ([MenuScene.cpp](Project2/MenuScene.cpp))

### Notes

- スコープは **Game3 (あまちゃん) 限定** — Game1=ふつう / Game2=まきもどり は据え置き、難易度維持と差別化を兼ねる
- デフォルト全 ON は **現状の Game3 動作と完全一致**、settings.ini を作らず起動しても挙動同じ
- `showGain=true` だが `showHints=false` の組合せは UI では許可 (独立トグル)、描画コードでは showHints が外側ガードなので showGain も実質無効

---

## [1.5.6.1] - 2026-06-26

**SUBPATCH リリース** — コードのみの変更で挙動は完全に不変 (新規 extern 色定数の追加のみ、本リリースでは未使用)。

### Added

- **B3: カラーパレット増設 (初期 7 色)** — [GameMain.cpp](Project2/GameMain.cpp) 定義 + [GameMain.h](Project2/GameMain.h) extern を既存パターンに沿って追加
  - **ランク章用 4 色** (B2 プレイヤーランクシステム伏線): `ColorBronze` (205,127,50) / `ColorSilver` (192,192,192) / `ColorGold` (255,215,0) / `ColorPlatinum` (229,228,226)
  - **汎用 UI 色 3 色** (B1 オプショントグル + テーマ化伏線): `ColorWarn` (255,235,0、警告系黄色) / `ColorOverlay` (128,128,128、半透明オーバーレイ用中間グレー) / `ColorHover` (180,220,255、ホバー強調)
  - 既存インライン `GetColor()` 呼び出し (盤面色 / メッセージ箱グレー / ヒントオレンジ / Menu/Game4 の白赤リテラル) の extern 化は本リリースのスコープ外
  - 新規 7 色は本リリースでは未参照 — B1 / B2 / テーマ化等の将来機能で使用される
- **4 セグメント版数ルール (SUBPATCH)** を CHANGELOG 冒頭の採番ルールに明文化 — コードのみ変更で挙動不変な場合の新慣例 (例: 本リリース)

### Changed

- ウィンドウタイトル `Reverse Reversi 1.5.6` → `Reverse Reversi 1.5.6.1` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.5.6` → `Ver 1.5.6.1` ([MenuScene.cpp](Project2/MenuScene.cpp))

### Notes

- カラーパレット増設は **継続的タスク** として CLAUDE.md §10 に残置。今後の機能拡張 (B1/B2/テーマ化/Easter Egg 等) で新色が必要になり次第、同じ `ColorXxx` 命名で追加し SUBPATCH バンプ (1.5.6.2、1.5.6.3、...) または機能側 PATCH バンプに巻き込む方針
- バイナリサイズ +28 byte 程度 (7 個の `unsigned int` グローバル、4 byte × 7)、パフォーマンス影響ゼロ

---

## [1.5.6] - 2026-06-26

### Added

- **Game3 (あまちゃん次元)「待った」機能** — R キー押下で「プレイヤーが直近に置いた手の直前」(CPU の応手も含めて) に盤面/ターン/ステータスを巻き戻し ([Game3Scene.cpp](Project2/Game3Scene.cpp))
  - 新規 file-scope static: `prevState` (ReversiBoard) / `prevStatus` (GAME_STATUS) / `prevTurn` (GAME_TURN) / `undoAvailable` (bool)
  - `initGame3Scene` で 4 つすべてリセット (再入場時の前回値残留防止、ゲーム開始直後は undo 不可)
  - `moveGame3Scene` PHASE_PLAYING 冒頭で R キー判定 + 一括復元 (`status != GAME_STATUS_FINISHED` ガード付き)
  - PLAYING の思考 (else) ブランチでプレイヤー手番のみ思考前にスナップショット → 思考確定時 (`think` true 返却) に persist
  - `renderGame3Scene` で「R: 待った」ガイド表示 (`undoAvailable && status != FINISHED` 時のみ、`PANEL_ROUND_LABEL_Y + 70` 位置、`ColorSky`)。テキストは 800x700 解像度で右端からはみ出さないよう短縮版を採用
  - **終局状態 (`GAME_STATUS_FINISHED`) では無効** — 勝敗を尊重してプレイを区切る方針、R 押下も UI ガイドも非反応
  - スコープは Game3 限定 (Game1=ふつう / Game2=まきもどり は難易度維持で据え置き)
- [LICENSE.md](LICENSE.md) を新規作成 — **MIT License** 本文、Copyright (c) 2026 OutRose。末尾に「第三者ライブラリのライセンス」セクションで [DXライブラリ](https://dxlib.xsrv.jp/) (作者: 山田 巧 氏) のライセンス遵守注記を併記
- [README.md](README.md) を新規作成 (GitHub 公開向け) — **日本語上 / 英語下** のバイリンガル構成。タイトル / 特徴 / 3 モード説明 / ビルド要件 / ビルド手順 / 操作方法 / ライセンス / ドキュメント / クレジットの 8 セクション
- README から [LICENSE.md](LICENSE.md) / [CHANGELOG.md](CHANGELOG.md) / [CLAUDE.md](CLAUDE.md) / [Project2/Common.props](Project2/Common.props) への相互リンク

### Changed

- ウィンドウタイトル `Reverse Reversi 1.5.5` → `Reverse Reversi 1.5.6` ([GameMain.cpp](Project2/GameMain.cpp))
- メニュー版数表示 `まきもどリバーシ Ver 1.5.5` → `Ver 1.5.6` ([MenuScene.cpp](Project2/MenuScene.cpp))

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

[1.5.9.2]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.9.2
[1.5.9.1]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.9.1
[1.5.9]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.9
[1.5.8]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.8
[1.5.7]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.7
[1.5.6.1]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.6.1
[1.5.6]: https://github.com/OutRose/ReversedReversi/releases/tag/v1.5.6
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
