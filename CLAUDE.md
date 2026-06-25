# CLAUDE.md — ReversedReversi プロジェクト情報

Visual Studio (MSVC) + DxLib による C++ リバーシゲーム。本体は [Project2.sln](Project2.sln) / [Project2/](Project2/) 配下。タイトルバー表記は「Reverse Reversi 1.1」、メニュー描画は「まきもどリバーシ Ver 1.1」([Project2/MenuScene.cpp:68](Project2/MenuScene.cpp#L68))。日本語 Windows 環境 (コードページ 932) でビルドする前提。

姉妹プロジェクトに [TwistTimeStopper](d:\Repositories\TwistTimeStopper) があり、シーン管理の雛形を共有している (元は同じ「Scene管理付き空プロジェクトRev2」テンプレート)。TwistTimeStopper は既に α/β/γ/δ のリファクタを完走しており、共通基盤化のリファレンスとして本ファイル中で頻繁に参照する。

---

## 1. プロジェクト概要

**まきもどリバーシ** は 12×12 盤面のリバーシ亜種。メニューから 3 モードを選択する構成だが、現状は **2 モード完成 + 1 モード未完成**:

| メニュー位置 | 表示名 | SCENE_NO | ファイル | 状態 |
|---|---|---|---|---|
| 0 | `ふつうの次元：頭をほどよく使う` | `SCENE_GAME1` | [Game1Scene.cpp](Project2/Game1Scene.cpp) | **完成** |
| 1 | `あまちゃん次元：頭をあまり使わない` | `SCENE_GAME3` | [Game3Scene.cpp](Project2/Game3Scene.cpp) | **未完成** (名前入力のみ→ Game2 へ丸投げ) |
| 2 | `まきもどり次元：頭をかなり使う` | `SCENE_GAME2` | [Game2Scene.cpp](Project2/Game2Scene.cpp) | **完成** |

メニュー配列実体: [MenuScene.cpp:7](Project2/MenuScene.cpp#L7) `SCENE_NO menu[3] = { SCENE_GAME1, SCENE_GAME3, SCENE_GAME2 };`

メニュー非掲載の `SCENE_GAME4` は **空シーン雛形** (新シーン追加時のコピー元、α-7 で確立)。enum 値とディスパッチには残るが menu[] からは到達不能。

### 2 ラウンド制 (まきもどりモード)

プロジェクト名「まきもど」の所以は **`SCENE_GAME2` (まきもどり次元)** の 2 ラウンド構造:

- ラウンド 1: 通常のリバーシを進行
- ラウンド間: `removePiece()` で 96 マスを削除して盤面を巻き戻し風にリセット (`CurrentRound` が 1→2 へ)
- ラウンド 2: 2 局目を開始、終局でゲーム終了処理

状態管理は [Game2Scene.cpp:14-15](Project2/Game2Scene.cpp#L14) のファイルスコープ `CurrentRound` / `excuted` グローバル。BGM もラウンドで切替 (`loop_68.wav` / `loop_95.wav`、`changeBGM` 関数)。**シーン再入場時のリセット処理が `initGame2Scene` に無い** ため、メニューに戻って再エントリすると `CurrentRound=2` のまま即終了表示になる既知の不具合あり (詳細は [10. 未完成機能の方針](#10-未完成機能の方針) を参照)。

### 空シーン雛形 (`SCENE_GAME4`)

α-7 (2026-06-25) で旧 `SCENE_GAME2`/`SCENE_GAME3` の死蔵スタブを整理: 旧 `SCENE_GAME3` (誤コピペ持ち) は削除、旧 `SCENE_GAME2` を新 `SCENE_GAME4` に転用して **TwistTimeStopper の Game4Scene 方式に倣った「空シーン雛形」** に位置付け。`menu[]` には載せず、新シーン追加時のコピー元として保持する。

---

## 2. ファイルエンコーディング方針: UTF-8 BOM 統一

**2026-06-25 に α-1 で全 17 ファイルを UTF-8 BOM (`EF BB BF`) + CRLF に統一済み。新規ファイルも UTF-8 BOM で作成すること。** [.editorconfig](.editorconfig) で `charset = utf-8-bom` を強制している。

### 現状

| 種別 | ファイル数 |
|---|---|
| UTF-8 BOM + CRLF | 17 (全数) |
| Shift-JIS (CP932) | 0 |
| UTF-8 BOM なし | 0 |

`.editorconfig` は [.editorconfig](.editorconfig) に新設済み (TwistTimeStopper と完全同一)。

### 移行手順 (α-1 で実施済、参考保存)

1. プロジェクトルートに `.editorconfig` を新設し `charset = utf-8-bom` / `end_of_line = crlf` / `indent_style = tab` を強制
2. Project2/ 配下の `.cpp` / `.h` を PowerShell で一括 CP932 → UTF-8 BOM 変換 (下記スニペット)
3. 変換後に Debug ビルドで再確認 (ダメ文字 `\` 混入による C2059/C2143 が出ないか)

```powershell
$path = 'd:\Repositories\ReversedReversi\Project2\<file>'
$bytes = [IO.File]::ReadAllBytes($path)
$utf8Strict = [Text.UTF8Encoding]::new($false, $true)
try { $text = $utf8Strict.GetString($bytes) }
catch { $text = [Text.Encoding]::GetEncoding(932).GetString($bytes) }
if ($text.Contains([char]0xFFFD)) { throw 'decode failed' }
$text = $text -replace "`r?`n", "`r`n"  # CRLF 統一
[IO.File]::WriteAllText($path, $text, [Text.UTF8Encoding]::new($true))
```

### Claude Code Write ツール使用時の注意

**Claude Code の Write ツールは UTF-8 BOM を自動付与しない** (改行も LF になる) ため、新規ファイル作成や既存ファイルの全体書き換えに使うと、Visual Studio が CP932 として誤解釈し、コメント内のダメ文字 (`ソ/表/能/予` など 2 バイト目が `0x5C` の文字) の `\` 混入で構文崩壊が発生する (TwistTimeStopper β-D-1 で実際に C2059/C2143 が 30+ 件連鎖)。

**対策**:
- 既存ファイルの一部修正は **Edit ツール優先** (BOM 保持される)
- Write ツールで全体書き換えした後は **必ず PowerShell で BOM + CRLF に再保存**
- 確認: Git Bash で `file <path>` を実行し、`UTF-8 (with BOM) text, with CRLF line terminators` と表示されることを確認

UTF-8 BOM 統一後 (α-1 完了済) は、Write ツール経由で新規作成・全体書き換えしたファイルは BOM が落ちて CP932 として誤解釈される (Visual Studio 上で文字化け、ビルドエラー連鎖) ので **Edit ツールを優先 / Write 使用後は PowerShell で BOM + CRLF 再保存** を徹底すること。

---

## 3. ビルド環境

- **ソリューション**: [Project2.sln](Project2.sln) (構成は **Debug|Win32 / Release|Win32 のみ**、x64 構成なし)
- **プロジェクト**: [Project2/Project2.vcxproj](Project2/Project2.vcxproj)
- **PlatformToolset**: `v145` (TwistTimeStopper も同じ。一般的な VS2022 標準 `v143` ではない点に注意。VS 18 Insiders 系で動作確認)
- **CharacterSet**: `MultiByte` (MBCS)
- **LanguageStandard**: 未指定 (ツールセット既定、C++14 相当)
- **WindowsTargetPlatformVersion**: `10.0`
- **RuntimeLibrary**: Debug = `/MTd` (MultiThreadedDebug)、Release = `/MT` (MultiThreaded) — 静的 CRT
- **SubSystem**: 明示なし (ConfigurationType=Application で既定 Windows)

### DxLib

**α-2 (2026-06-25) で Debug / Release 両構成とも `C:\DxLib` に統一済み** (旧 Release 構成は `C:\DxlibFile` 不在で C1083 ×8 だった)。`C:\DxLib` インストール前提でビルド可能。

中長期的な改善余地:
- `DXLIB_DIR` 環境変数や `.props` シートに切り出して移植性を確保 (フェーズ δ-3 候補)

### ビルド結果 (2026-06-25、α-2 完了時点)

| 構成 | プラットフォーム | 結果 | 警告 | エラー |
|---|---|---|---|---|
| Debug | Win32 | 成功 | 0 | 0 |
| Release | Win32 | 成功 | 0 | 0 |
| (どちらも) | x64 | 未試行 (vcxproj に x64 構成なし) | - | - |

### 画面・フレーム

- **画面**: 800×700 / **16bit カラー** ([GameMain.cpp:41](Project2/GameMain.cpp#L41) `SetGraphMode(800, 700, 16)`)。TwistTimeStopper は 24bit、こちらは 16bit と差異あり。32bit が現代標準
- **ウィンドウ**: `ChangeWindowMode(true)` でウィンドウ起動 ([GameMain.cpp:25](Project2/GameMain.cpp#L25))。**`SetWindowSizeChangeEnableFlag(true)`** ([GameMain.cpp:38](Project2/GameMain.cpp#L38)) で実行中リサイズ可 — TwistTimeStopper にない設定。動作未確認のコメント (`//Check:実行中に画面の大きさが変更可能か`) あり
- **タイトル**: `SetMainWindowText("Reverse Reversi 1.1")` ([GameMain.cpp:44](Project2/GameMain.cpp#L44))
- **背景色**: `SetBackgroundColor(0, 0, 0)` 黒 ([GameMain.cpp:47](Project2/GameMain.cpp#L47))
- **フレームレート**: 60 FPS ビジーウェイト ([GameMain.cpp:69](Project2/GameMain.cpp#L69) `while (GetNowCount() - FrameStartTime < 1000 / 60) {}`)
- **エントリーポイント**: `WinMain` ([GameMain.cpp:19](Project2/GameMain.cpp#L19))

### 毎フレームの定型フロー

```cpp
ClearDrawScreen();       // 1. バックバッファクリア
// 60FPS 待機 (ループ)
FrameStartTime = GetNowCount();
i = GetJoypadInputState(DX_INPUT_KEY_PAD1);
EdgeInput = i & ~Input;  // エッジ算出
Input = i;
FrameMove();             // 2. シーン更新
RenderScene();           // 3. シーン描画
ScreenFlip();            // 4. 表示反映
if (ProcessMessage() == -1) break;
if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break;
```

ループ脱出後は `DxLib_End()` → `GameRelease()` の順。**`DxLib_End` が `GameRelease` より先**である点は要観察 (シーン後処理が DxLib 終了後に走るため `DeleteGraph` 等の安全性疑義あり、TwistTimeStopper も同順)。

### 既知の絶対パス残骸

[Project2/Debug/Project2.vcxproj.FileListAbsolute.txt](Project2/Debug/Project2.vcxproj.FileListAbsolute.txt) に `D:\Visual Studio Repository\Scene管理付き空プロジェクトRev2\...` の旧パスが残存。クリーンビルド時に削除推奨。

---

## 4. シーンアーキテクチャ

### enum とグローバル状態 (現状)

[Project2/GameSceneMain.h](Project2/GameSceneMain.h) で `SCENE_NO` enum を定義 (α-7 で SCENE_GAME5 削除、番号詰め済):

```c
typedef enum _SCENE_NO {
    SCENE_NONE = -1,    // 下限センチネル
    SCENE_MENU,         // メニュー
    SCENE_GAME1,        // ふつうの次元 (完成)
    SCENE_GAME2,        // まきもどり次元 (完成、2 ラウンド制)
    SCENE_GAME3,        // あまちゃん次元 (未完成、名前入力のみ)
    SCENE_GAME4,        // 空シーン雛形 (menu[] 非掲載、新シーン追加時のコピー元)
    SCENE_MAX           // 上限センチネル
} SCENE_NO;
```

[Project2/GameSceneMain.cpp](Project2/GameSceneMain.cpp) で `static SCENE_NO sceneNo, prevScene, nextScene` を管理。

### ディスパッチはテーブル駆動 (β-D-1 完了、2026-06-25)

[Project2/GameSceneMain.cpp](Project2/GameSceneMain.cpp) で `SCENE_HANDLERS sceneTable[SCENE_MAX]` 関数ポインタ束を定義し、1 行ディスパッチ (`sceneTable[sceneNo].xxx()`) を実現済。新規シーン追加は **(1) SCENE_NO に 1 値追加、(2) sceneTable に 1 行追加** の 2 箇所のみ。

β-D-1 で同時に達成した改善:
- `prevScene` 削除 (changeScene 失敗時の戻り先が不要に、assert で検知)
- `MessageBox` 5 箇所撤去 → `MyOutputDebugString` + `assert` (Debug 停止 / Release 無視継続)
- `initCurrentScene` の `BOOL` 戻り値を活用、`FrameMove` で SCENE_MENU フォールバック (それも失敗なら SCENE_NONE で諦め、無限ループ防止)

### 4 関数命名規約 (シーンごと)

各シーン名 `XXXScene` (Menu / Game1 / Game2 / Game3 / Game4) に対し:

| 関数 | 戻り値 | 役割 |
|---|---|---|
| `initXXXScene()` | `BOOL` | リソース確保 / 初期化。シーン入場時 1 回呼ばれる。 |
| `moveXXXScene()` | `void` | 毎フレームの状態更新。 |
| `renderXXXScene()` | `void` | 毎フレームの描画。 |
| `releaseXXXScene()` | `void` | リソース解放。シーン退場時 1 回呼ばれる。 |
| `XXXSceneCollideCallback(nSrc, nTarget, nCollideID)` | `void` | DxLib 衝突判定コールバック。 |

ReversedReversi では **`init` の戻り値 `BOOL` を捨てている**点に注意 (TwistTimeStopper β-D-3 では失敗時に SCENE_MENU フォールバック → SCENE_NONE 諦めという布石が入っているが、本プロジェクトは未対応)。

### 遷移

`changeScene(SCENE_NO no)` で `nextScene` を設定。次フレームに `sceneNo != nextScene` を検出した時点で `releaseXXX → sceneNo = nextScene → initYYY` の順で実行される。

### ⚠️ Game1Scene / Game2Scene の `while (!ProcessMessage())` 独自ループ

[Game1Scene.cpp](Project2/Game1Scene.cpp) と [Game2Scene.cpp](Project2/Game2Scene.cpp) (旧 Game4Scene、α-7 で改名) の `move*` 関数内に **独自の `while (!ProcessMessage())` 無限ループ**が組まれており、メインループの 1 フレーム 1 ティック設計を完全に無視している。一度入ると ESC か終了条件まで戻ってこないため、`FrameMove` / `RenderScene` のフレーム駆動モデルが事実上崩壊している。

**本プロジェクト最大の構造的不具合**。リファクタの優先課題で、状態 (`status`, `turn`, `CurrentRound`, `msg`, `msg_wait`, `SqBoard` 等) を `static` 化してフレーム駆動に書き換えるのが望ましい。

### ⚠️ CollideCallback の制約

`MenuSceneCollideCallback` 上に「**ここでは要素を削除しないこと！！**」コメントあり。DxLib の衝突判定列挙の最中に呼ばれるため、コールバック内で対象オブジェクトを `delete` するとイテレータ破壊で UB になる。削除はフラグを立てて `moveXXXScene` 内で実施するパターン (TwistTimeStopper 規約と同じ)。

### TwistTimeStopper と共通化済 / 共通化候補

- ✅ **シーンディスパッチ表化** (β-D-1 完了): switch 5 箇所 → 関数ポインタテーブル 1 つに圧縮
- ✅ **`init` 戻り値の活用と SCENE_MENU フォールバック** (β-D-1 完了)
- ✅ **`MessageBox` → `MyOutputDebugString + assert`** (β-D-1 完了)
- ✅ **インクルードガード `__XXX_H_` → `XXX_H_` 統一** (α-6 完了)
- ✅ **共通 enum 化** (β-C 完了): GAME_STATUS / GAME_TURN / GAME_ROUND を GameSceneMain.h に集約
- ✅ **色グローバル集約** (β-D-2 完了): `ColorWhite/Red/Sky` を GameMain.cpp/.h に一本化、`ColorXxx2` 接尾辞撤廃
- ✅ **マジックナンバー定数化** (β-D-4 完了 + β-D-5 でグリッド描画座標も含めて完了): 画面・FPS・盤面サイズ・主要レイアウト座標を GameMain.h に集約。グリッド描画 (`5/580/48/11`) も β-D-5 で `BOARD_ORIGIN_X / BOARD_END_PX / CELL_PX / BOARD_SIZE_MAX` に置換
- ✅ **盤面ロジックの単一化** (β-D-5 完了): Game1Scene/Game2Scene のクローンコード (`putPiece`/`putPiece2`、`isPass`/`isPass2`、`think1/2`/`think01/02`、`setMsg`/`setMsg2`、`checkResult`/`checkResult2`、`SqBoard`/`SqBoardA`、`removePiece`) を `ReversiBoard` 構造体 (`board[BOARD_SIZE][BOARD_SIZE]` + `std::string msg` + `int msg_wait`) + `rb*` 関数群 (`rbInit`/`rbPutPiece`/`rbIsPass`/`rbThinkPlayer`/`rbThinkCpu`/`rbSetMsg`/`rbCheckResult`/`rbCountPieces`/`rbRemovePieces`) に統合 (TwistTimeStopper `TIMER_STATE` 同方式)。Game1Scene 371→203 行、Game2Scene 452→273 行で合計 ‑157 行削減

---

## 5. 命名規約

### インクルードガード (α-6 で正規化済、2026-06-25)

α-6 で TwistTimeStopper と同じ `<NAME>_H_` 形式に統一済。**実態調査で判明**: 元々ガードが存在したのは `GameSceneMain.h` 1 ファイルのみ (`__GAMESCENEMAIN_H_` 予約識別子)、他 8 ファイルはガード自体が無く多重 include 時に再宣言で済んでいた (プロトタイプ宣言のみのヘッダ構成だったため)。α-6 で `GameSceneMain.h` を改名 + 他 8 ファイルに新規ガード追加。

| ヘッダ | ガード形式 |
|---|---|
| GameMain.h | `#pragma once` (例外、TwistTimeStopper 流) |
| MenuScene.h | `MENUSCENE_H_` |
| Game1Scene.h | `GAMESCENE1_H_` |
| Game2Scene.h | `GAMESCENE2_H_` |
| Game3Scene.h | `GAMESCENE3_H_` |
| Game4Scene.h | `GAMESCENE4_H_` |
| GameStatus.h | `GAMESTATUS_H_` |
| GameSceneMain.h | `GAMESCENEMAIN_H_` |

Game1〜4Scene は **`GAMESCENE<N>_H_`** (数字は GAMESCENE の後)。`GAME<N>SCENE_H_` ではない点に注意 (TwistTimeStopper 流)。

### 関数命名

- シーン関数: `initXXXScene` / `moveXXXScene` / `renderXXXScene` / `releaseXXXScene` / `XXXSceneCollideCallback` ([4. シーンアーキテクチャ](#4-シーンアーキテクチャ) 参照)
- 共通ロジック関数 (盤面ステート操作、β-D-5 で導入): `rb*` プレフィックス (`rbInit`, `rbPutPiece`, `rbIsPass`, `rbThinkPlayer`, `rbThinkCpu`, `rbSetMsg`, `rbCheckResult`, `rbCountPieces`, `rbRemovePieces`)。第 1 引数は `ReversiBoard*` ([GameSceneMain.h](Project2/GameSceneMain.h) 定義)
- シーン専用補助関数: ローワーキャメル (例: Game2 `changeBGM`)
- β-D-5 で `*2` / `*02` サフィックスは全廃 (旧 `putPiece2`/`think01`/`think02`/`setMsg2`/`checkResult2`/`removePiece` は `rb*` に統合済)

### enum 命名

β-C 完了済 ([GameSceneMain.h](Project2/GameSceneMain.h)):

- `SCENE_NO`: `SCENE_NONE` (-1) / `SCENE_MENU` / `SCENE_GAME1〜4` / `SCENE_MAX` (両端センチネル + メンバー、α-7 で 5 値に縮小)
- `GAME_STATUS`: `GAME_STATUS_PLAYING` (=1) / `GAME_STATUS_TURN_MSG` / `GAME_STATUS_PASS_MSG` / `GAME_STATUS_FINISHED`
- `GAME_TURN`: `GAME_TURN_BLACK` (=1) / `GAME_TURN_WHITE` (=2)
- `GAME_ROUND`: `GAME_ROUND_FIRST` (=1) / `GAME_ROUND_SECOND` (=2)

規約: `<ENUM_NAME>_<MEMBER>` の `<ENUM_NAME>` は SCREAMING_SNAKE_CASE、enum タグは `_<ENUM_NAME>` 前置 (`typedef enum _GAME_STATUS { ... } GAME_STATUS;`)。SCENE_NO 流。

### 変数命名

- グローバル: 名詞 (`Input`, `EdgeInput`, `nameTmp`, `CurrentRound`, `excuted`, `FrameStartTime`)
  - β-D-5 で旧 `SqBoard` (Game1) / `SqBoardA` (Game2) は `static ReversiBoard state` (各シーンファイルローカル) に統合済
  - α-4 で `int startfont;` (MenuScene)、`int game_status = GAMETITLE;` (GameMain) を削除済
- ローカル: ローワーキャメル / 短縮 (`wx`, `wy`, `wn`, `kx`, `ky`, `turn`)
- 定数: マクロ大文字 (`MENU_MAX`, `PAD_INPUT_*`, `KEY_INPUT_*`)

### マジックナンバー集中地帯 (改善余地)

`Game1Scene.cpp` / `Game2Scene.cpp` (=まきもどり、旧 Game4) 全域に `12` (盤面サイズ), `48` (セルサイズ), `5`, `580`, `47`, `192`, `630`, `655`, `590`, `40/125`, `680`, `100/700/800` 等が散在。命名定数は `#define MENU_MAX 3` のみ。TwistTimeStopper β-D-4 と同じ手順で `GameMain.h` への定数集約が必要。

### インデント

タブ。`.editorconfig` がないため Visual Studio 既定に依存。`.editorconfig` 新設時は `indent_style = tab` を強制する方針。

---

## 6. コメント方針

**コードコメントは必須**。各箇所が「何のために存在し、何を果たすか」の役割メモを残す方針。学習由来の番号付き授業コメント (`//７(1) ①…`)、自明な訳コメント、教科書的説明も削減対象としない。

- **保持対象**: 関数/変数/分岐の役割説明、定数の意味付け、状態遷移の意図、DxLib 慣用の前提、入力キーの用途など — たとえ自明に見えても残す
- **削除対象 (dead code 系のみ)**:
  - コメントアウトされた旧コード (`// char xxx;` 形式や `/* … */` の旧実装ブロック)
  - ファイル名が古いままの残骸 (例: `Game3Scene.cpp:27` 「ゲーム画面１です」誤コピペ)
  - 明らかな誤情報
- **新規コード**: Claude/手動を問わず、新たに書く関数や複雑な分岐には必ず役割コメントを付ける

### dead code コメント整理 (α-8 で実施済、2026-06-25)

- ✅ [GameMain.cpp:27-34](Project2/GameMain.cpp) — `/* … */` 旧 MessageBox ウィンドウモード確認ブロック (削除済)
- ✅ Game1Scene.cpp — `//int back;`, `//back = LoadGraph(...)`, `//SetDrawBlendMode`, `//DrawGraph(100, ...)`, `/*テスト用：マウス座標取得 ... //テスト用ここまで*/` ブロック (削除済)
- ✅ Game2Scene.cpp (旧 Game4Scene) — 同種の残骸 (削除済)

### 保持判断したコメント

- [Game3Scene.cpp](Project2/Game3Scene.cpp) (旧 Game5Scene、あまちゃん) の `//ワープさせたい。例えば：1=通常モード　4=まきもどりモード` / `//問題はここよここ` (Game5 → Game3 改名後も保持、γ-3 で再検討予定の TODO 注記)
- [MenuScene.cpp](Project2/MenuScene.cpp) の `//無論ここにもBGMを。` (将来用メモ)
- 各 cpp の `//プログラムで格子を描く（上ではうまく行かない）` — 実装履歴の役割メモ
- `//Check:`, `//check:` 接頭辞のコメント — 確認待ち TODO 注記

リテラル `TODO/FIXME/HACK/XXX` は本プロジェクトでは 0 件 (代わりに日本語コメントで「ここ」「問題」「Check:」「無論」を使う慣習)。

---

## 7. 入力処理

### 仕組み

[Project2/GameMain.cpp:73-81](Project2/GameMain.cpp#L73) の毎フレーム冒頭:
```cpp
int i = GetJoypadInputState(DX_INPUT_KEY_PAD1);  // 現フレームのビットマスク
EdgeInput = i & ~Input;                           // 立ち上がりエッジ (押された瞬間)
Input = i;                                        // 現状保持
```

- **`Input`** (グローバル `int`): 押されている間ずっと真
- **`EdgeInput`** (グローバル `int`): 押された瞬間の 1 フレームだけ真

### 使い分け

| 用途 | 推奨 |
|---|---|
| メニュー移動 / 確定 (連打防止したい) | `EdgeInput & PAD_INPUT_UP` 等 |
| 移動 / 押しっぱなしで効くアクション | `Input & PAD_INPUT_*` |
| キーボード直接検査 (パッドにマップされない G/S/X 等) | `CheckHitKey(KEY_INPUT_G) == 1` |
| シーン終了の ESC | `CheckHitKey(KEY_INPUT_ESCAPE) == 1` (メインループ + 各シーン release) |

代表例: [MenuScene.cpp:36](Project2/MenuScene.cpp#L36) で `EdgeInput & PAD_INPUT_UP`、[MenuScene.cpp:58](Project2/MenuScene.cpp#L58) で `EdgeInput & PAD_INPUT_1` (決定)。

### 入力定義

[入力Inputについて.txt](入力Inputについて.txt) に `PAD_INPUT_UP/DOWN/LEFT/RIGHT/1〜4` の一覧 (現時点 1 プレイヤーのみ)。

### マウス入力

Game1Scene / Game2Scene (まきもどり、旧 Game4) では `GetMousePoint` 系を使って盤面クリック判定。`x / 48` で盤面インデックス化するため、**範囲外クリック (x>576) で `SqBoard[15][...]` のような領域外アクセスが起こる** UB あり (リファクタ候補)。

### Game3 (あまちゃん、旧 Game5) のキーボード文字入力

[Game3Scene.cpp](Project2/Game3Scene.cpp) で `KeyInputSingleCharString` を使って `nameTmp[12]` に 1 文字ずつ蓄積。`nameTmp[0] != NULL` 検知で `SCENE_GAME2` (まきもどり、旧 SCENE_GAME4) へ遷移する仮実装。`'\0'` または `0` と比較すべきで、`NULL` 比較は `char` と `void*` の暗黙変換警告対象 ([10. 未完成機能の方針](#10-未完成機能の方針) 参照)。

---

## 8. DxLib のお作法 (細かい点)

- **フォント**: `SetFontSize(N)` / `ChangeFontType(DX_FONTTYPE_*)` / `ChangeFont("ＭＳ 明朝")` はグローバル状態なので、各シーンの init で必ず設定し直す ([MenuScene.cpp:20-21](Project2/MenuScene.cpp#L20))。**ReversedReversi では `ChangeFont("ＭＳ 明朝")` が毎フレーム呼ばれているシーンがある** (Game1/Game2=まきもどり の render 内) — 本来 init 時 1 回でよく、TwistTimeStopper では init で完結している
- **色**: `GetColor(R, G, B)` の戻り値 `unsigned int` はキャッシュ。β-D-2 で [GameMain.cpp:20-22](Project2/GameMain.cpp#L20) に `ColorWhite/Red/Sky` を集約定義 + [GameMain.h](Project2/GameMain.h) で `extern` 公開 (旧 `ColorXxx2` 接尾辞は撤廃済)。新規色を追加する際は同じ extern + 定義の構造に合わせる
- **座標系**: 原点 (0, 0) は **画面左上**。`DrawString(x, y, str, color)` で直接指定
- **乱数**: `GetRand(n)` で 0〜n-1。シードは起動時に `GetNowCount()` で `SRand` 初期化する慣習 (TwistTimeStopper では `WinMain` 冒頭)
- **画像**: `LoadGraph` / `LoadDivGraph` の戻り値ハンドル `int` を保持し `DeleteGraph` で解放。本プロジェクトでは Game1/Game2 (=まきもどり) で `LoadDivGraph` が `moveXXXScene` 内で呼ばれていて毎フレーム再ロードする恐れ + `releaseXXXScene` で `DeleteGraph` していない → **リソースリーク**
- **音**: `PlaySoundFile(path, DX_PLAYTYPE_LOOP / BACK / NORMAL)`。`StopSoundFile()` でループ停止。**MenuScene と Game2 (まきもどり) は BGM 切替を行うが、Game1 にも `StopSoundFile` を入れて遷移時の BGM クリーンアップを徹底する必要あり**
- **デバッグ出力**: `MyOutputDebugString(_T("..."), 引数)` マクロを [GameMain.h](Project2/GameMain.h) に定義済 (β-D-3 完了)。Debug ビルドでのみ `OutputDebugString` を発火、Release は空展開。`<stdio.h>`/`<Windows.h>`/`<tchar.h>` も同ヘッダに集約 (`_stprintf_s` で 4996 セキュア化)
- **DxLib 終了順**: `DxLib_End()` → `GameRelease()` の順 ([GameMain.cpp:99-102](Project2/GameMain.cpp#L99)) — シーン後処理が DxLib 終了後に走る潜在的問題。逆順にする選択肢あり (TwistTimeStopper と同方針で要検討)

---

## 9. 警告対処パターン

現状 Debug|Win32 ビルドは警告ゼロだが、UTF-8 BOM 化や `<iostream>` 除去で連鎖的に発生しうる典型パターン:

### C4244 (numeric narrowing, `int`/`double` → `float`)

明示キャストまたは `*f` サフィックスで抑制。動作は変えない。

```cpp
// 整数 → float
RandomTgt = (float)(GetRand(19) + 1);

// double → float (リテラル側)
Score = CalFrame * 1.25f;

// double → float (関数戻り値) ★ floor → floorf が定石
scJudge = floorf(scJudge);
```

**注意**: `<iostream>` を取り除くと、それが連鎖的に読み込んでいた `<cmath>` の `float floor(float)` オーバーロードが消えて `floor(float)` が `double` を返すようになり C4244 が新規発生する。`floorf` 等 C99 の `*f` 系関数 (`<math.h>` 提供) に置き換えるのが最も安全 (TwistTimeStopper β-A での実例)。

### C4129 (unrecognized character escape)

原因: Shift-JIS 文字列リテラル中の **ダメ文字** (2 バイト目が `0x5C = '\\'` になる「ソ」「表」「能」「予」など)。**本プロジェクトは現状 CP932 のため発生候補多数**。UTF-8 BOM 化後は理論上発生しないが、Claude Code Write ツール経由で BOM が落ちると即座に再発する ([2. ファイルエンコーディング方針](#2-ファイルエンコーディング方針-utf-8-bom-統一を目標) 参照)。

### C4060 (空 switch)

```cpp
switch (x) {
default: break;  // ← 追加
}
```

### C4819 (CP932 で表示できない文字)

エンコーディング統一後は基本的に発生しないはず。出たら無 BOM のファイルが混入した可能性が高いので `.editorconfig` の効きを確認 + [2. ファイルエンコーディング方針](#2-ファイルエンコーディング方針-utf-8-bom-統一を目標) の復旧手順を実施。

### C4996 (非推奨 API)

β-A (2026-06-25) で `#pragma warning(disable : 4996)` を撤去済 + `itoa` → `_itoa_s` 置換済 (Game1Scene.cpp / Game2Scene.cpp 各 2 箇所、合計 4 箇所)。`_stprintf` も `_stprintf_s` に変更済 (MyOutputDebugString マクロ内、β-D-3)。今後の新規 C4996 発生時は同様にセキュア API に置換する方針。

---

## 10. 未完成機能の方針

完成させるかどうかをカテゴリ別に明示 (α-7 でシーン番号詰め後の表記):

### 完成済み

- **`SCENE_GAME1` (ふつうの次元)** — 12×12 リバーシ本体、`think1`(人) vs `think2`(CPU)、勝敗判定まで実装。ただし `while (!ProcessMessage())` 独自ループによるフレーム駆動破壊と、シーン再入場時の `SqBoard` リセット欠落あり ([4. シーンアーキテクチャ](#4-シーンアーキテクチャ) 参照)
- **`SCENE_GAME2` (まきもどり次元、旧 SCENE_GAME4)** — Game1 の拡張版、2 ラウンド制 (`CurrentRound`)、ラウンド間で `removePiece()` で 96 マス削除、BGM 切替 (`changeBGM`)。Game1 の問題点に加えて、ラウンド 2 終了時に `releaseGame2Scene()` を直接呼んで `DxLib_End()` を叩くシーン管理違反あり ([Game2Scene.cpp](Project2/Game2Scene.cpp))

### 未完成 (要対応)

- **`SCENE_GAME3` (あまちゃん次元、旧 SCENE_GAME5)** — 名前入力 (`KeyInputSingleCharString` で `nameTmp`) だけ実装。コメントに「ワープさせたい。例えば：1=通常モード　4=まきもどりモード」とあるが、現実装は `nameTmp[0] != NULL` 検知で無条件 `SCENE_GAME2` (まきもどり) 遷移。
  - 設計意図 (推測): 名前入力後に独自の「あまちゃん」モードでプレイさせたかった
  - 現実装: Game2 (まきもどり) に丸投げで Game3 独自ゲームロジックなし
  - **方針候補**:
    1. **(現状維持)** Game2 への橋渡し専用と割り切り、名前入力 UI を完成させて `nameTmp` を Game2 で表示反映する小修正に留める
    2. **(独自モード化)** Game1/Game2 を共通化した後、`SCENE_GAME3` 専用の簡易ルール (例: AI の手数制限、盤面サイズ違い) を追加する
    3. **(削除統合)** `SCENE_GAME3` を削除してメニュー 2 項目化、`nameTmp` をメニュー側に移動

### 空シーン雛形

- **`SCENE_GAME4`** — α-7 で旧 SCENE_GAME2 (死蔵スタブ) を転用した **空シーン雛形** ([Game4Scene.cpp](Project2/Game4Scene.cpp))。`menu[]` 非掲載で到達不能だが、TwistTimeStopper の Game4Scene 流に「新シーン追加時のコピー元」として保持。「ゲーム画面４です」の描画と「ボタン１でタイトルに戻る」遷移のみ実装、新シーン作成時はこれをコピーして識別子置換 + enum/case 追加で起動する。

---

## 11. Git 慣習

- **コミットメッセージ**: 日本語。Claude Code 経由の作業には先頭に **`CC：`** プレフィックス (全角コロン) を付ける。例: `CC：エンコーディング統一、ルール追加、.md ファイル追加`。複数作業を 1 コミットにまとめる場合はコンマ区切り
- **ブランチ**: 作業ブランチは `refact-<YYMMDD>-<purpose>` 形式 (例: `refact-260625-base`)。`main` が安定ブランチ
- **コミットは原則ユーザーが手動で行う**。Claude は working tree に変更を残すまでで、`git commit` は実行しない (ユーザーから明示の指示があるまで)

### 直近の git 履歴 (2026-06-25 時点、新→古)

```
1784d86 構築済みファイルをコミット。CC前最終コミット
03e45af CodeMaidによるクリーンアップ
843465b WinMain注釈の修正・ローカル変数の初期化処理追加
c9e05f8 something refined
da8a600 軽微な修正
6470f87 {}の改行を統一
ffd4e52 クラスダイアグラムの追加
e380ea6 gitignore追加後の再コミット
b3bccff CodeMaidによる文体整理
a1e0559 Add files via upload
```

`1784d86` の「CC 前最終コミット」がまさに Claude Code に渡す直前のスナップショット。`843465b` の「ローカル変数の初期化処理追加」が Game4 の `wx[12]={0}` 等に対応していると推定されるが、Game1 側は未対応のまま。

---

## 12. リファクタリング戦略 (初版、これから策定)

TwistTimeStopper が α/β/γ/δ のフェーズ駆動で完走しているので、本プロジェクトも同じ段階分けを基本骨格とする。**各項目は プランモードで対象提示 → ユーザー承認 → Edit → grep 検証 のミニサイクルで実施**。コミットはユーザーが手動。

### フェーズ α: 低リスク整理 (予定)

参照が 0 件・到達不能・誤情報が確定しているコードのみを対象に削除/移行。

候補 (全項目 2026-06-25 に完了):
1. ✅ **エンコーディング統一**: 17 ファイル CP932 → UTF-8 BOM 化、`.editorconfig` 新設
2. ✅ **Release 構成のビルド復活**: DxLib パスを `C:\DxlibFile` → `C:\DxLib` に統一
3. ✅ **`GameStatus.h` のビットフラグ値修正**: `0x00000016` (=22) / `0x00000032` (=50) を `0x10` / `0x20` に修正
4. ✅ **`Project2/Debug/Project2.vcxproj.FileListAbsolute.txt` の旧パス残骸クリーンアップ**
5. ✅ **未使用グローバル削除**: `int startfont;` (MenuScene)、`int game_status = GAMETITLE;` (GameMain)
6. ✅ **シーン番号詰め + 雛形保存** (旧候補「死蔵スタブ削除」を発展): 旧 SCENE_GAME4/5 → 新 SCENE_GAME2/3、旧 SCENE_GAME2 → 新 SCENE_GAME4 (雛形)、旧 SCENE_GAME3 削除
7. ✅ **インクルードガード正規化**: GameSceneMain.h を `__GAMESCENEMAIN_H_` → `GAMESCENEMAIN_H_`、他 8 ファイル (ガード無しだった) に新規追加。GameMain.h は `#pragma once`
8. ✅ **dead code コメント整理**: GameMain.cpp / Game1Scene.cpp / Game2Scene.cpp の `//int back`, `//SetDrawBlendMode`, `//DrawGraph(100, ...)`, `/*テスト用：マウス座標取得 ... //テスト用ここまで*/` ブロック、旧 MessageBox ブロックを削除

### フェーズ β: 中リスク整理 (β-D-5 を除き 2026-06-25 完了)

dead code 一掃 / マジックナンバー定数化 / シーン構造の整理。

候補:
1. ✅ **β-A: 残った dead code 一掃** (完了) — `<math.h>` 削除 (Game1/Game2)、`itoa` → `_itoa_s` (4 箇所)、`#pragma warning(disable: 4996)` 撤去
2. **β-B: 過剰コメント整理 — 保留** ([6. コメント方針](#6-コメント方針) により、役割メモは保護対象)
3. ✅ **β-C: 状態値の enum 化** (完了) — `GAME_STATUS` (PLAYING/TURN_MSG/PASS_MSG/FINISHED)、`GAME_TURN` (BLACK/WHITE)、`GAME_ROUND` (FIRST/SECOND) を [GameSceneMain.h](Project2/GameSceneMain.h) に集約。Game1Scene/Game2Scene の数字リテラルを全置換
4. **β-D: 構造系**
   - ✅ **β-D-1: シーンディスパッチャ共通化** (完了) — `SCENE_HANDLERS sceneTable[SCENE_MAX]` + 1 行ディスパッチ、`prevScene` 削除、`MessageBox` 撤去 (5 箇所)、`init` 戻り値活用 + SCENE_MENU フォールバック
   - ✅ **β-D-2: 色グローバル集約** (完了) — `ColorWhite/Red/Sky` を [GameMain.cpp](Project2/GameMain.cpp) 定義 + [GameMain.h](Project2/GameMain.h) `extern`、`ColorXxx2` 接尾辞撤廃
   - ✅ **β-D-3: エラーハンドリング強化** (完了) — `MyOutputDebugString` マクロを [GameMain.h](Project2/GameMain.h) に昇格 (`_stprintf_s` セキュア版)、`<stdio.h>`/`<Windows.h>`/`<tchar.h>` 同梱
   - ✅ **β-D-4: マジックナンバー定数化** (完了) — 画面 (`SCREEN_WIDTH/HEIGHT/BPP=16`)、FPS、盤面 (`BOARD_SIZE 12`, `BOARD_SIZE_MAX 11`, `CELL_PX 48`, `PIECE_SIZE_PX 47`, `BOARD_CENTER_LOW/HIGH 5/6`)、`MSG_WAIT_FRAMES 60`、メッセージ/パネルレイアウト座標を [GameMain.h](Project2/GameMain.h) に集約。Game1Scene/Game2Scene 全域で参照置換。グリッド描画 (`5/580/48/11`) も β-D-5 で同定数に置換完了
   - ✅ **β-D-5: 盤面ロジック単一化** (完了) — `ReversiBoard` 構造体 + 9 関数 (`rbInit`/`rbPutPiece`/`rbIsPass`/`rbThinkPlayer`/`rbThinkCpu`/`rbSetMsg`/`rbCheckResult`/`rbCountPieces`/`rbRemovePieces`) を [GameSceneMain.h](Project2/GameSceneMain.h) / [GameSceneMain.cpp](Project2/GameSceneMain.cpp) に集約。Game1Scene と Game2Scene のクローン削除、`static ReversiBoard state = {};` で個別保持。Game1Scene 371→203 行 / Game2Scene 452→273 行 (合計 ‑157 行)。動作不変、Debug/Release 両ビルド警告 0 / エラー 0

### フェーズ γ: 機能完成

候補:
1. **γ-1: Game1/Game4 のフレーム駆動化** — `while (!ProcessMessage())` 独自ループ撤去、状態を `static` 化してフレーム毎に進める形に書き換え。シーン再入場時の状態リセット (`SqBoard`, `CurrentRound`, `excuted`) も同時実装
2. **γ-2: Game4 ラウンド 2 終了処理の正規化** — `releaseGame4Scene` 直呼び + `DxLib_End` 違反を解消し、`changeScene(SCENE_MENU)` で正常復帰させる
3. **γ-3: Game5 (あまちゃん次元) の方針確定と実装** — [10. 未完成機能の方針](#10-未完成機能の方針) の方針候補 1〜3 から選択

### フェーズ δ: 仕上げ

候補:
1. **δ-1: ResourceLeak の解消** — `LoadDivGraph` を `init`、`DeleteGraph` を `release` に移し直す
2. **δ-2: 入力安全化** — マウス座標の盤面外アクセス防止 (`x>576`, `y>576` ガード)
3. **δ-3: x64 構成追加とプラットフォーム移植性向上** — DxLib x64 版でビルド、`DXLIB_DIR` 環境変数 / `.props` 化検討

### 共通方針

- **動作不変リファクタを優先** (機能変更は γ 以降)
- **Workflow (ultracode) を活用**: 複数案の並列探索 → ユーザー判断 → 採用案実装
- **Debug|Win32 ビルドで毎回検証** (警告ゼロを維持)
- **TwistTimeStopper の方針を流用しつつ、リバーシ固有の差異 (盤面ロジック、2 ラウンド制) はプロジェクト固有設計として独自に進める**

---

## 13. 過去の整理作業履歴

完了済みの方針判断 (再議論不要)。今後、フェーズ完了ごとに追記する。

### フェーズ α-1: エンコーディング統一 (完了, 2026-06-25)

- プロジェクトルートに [.editorconfig](.editorconfig) を新設 (TwistTimeStopper と完全同一: `charset = utf-8-bom` / `end_of_line = crlf` / `indent_style = tab` / `insert_final_newline = true` / `trim_trailing_whitespace = true`)
- Project2/ 配下 17 ファイル (.cpp 8 + .h 9) を CP932 → UTF-8 BOM + CRLF に変換 (16 ファイル変換 + 1 ファイル先行変換済)。ダメ文字 `[char]0xFFFD` 混入なし、ソース内容変更なし (BOM 付与 + マルチバイト膨張のみ、+4136 bytes)
- 変換ログ: `scratchpad\alpha1_encoding_log.txt`
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0 維持

### フェーズ α-2: Release 構成のビルド復活 (完了, 2026-06-25)

- [Project2/Project2.vcxproj](Project2/Project2.vcxproj) の Release 構成 DxLib パスを `C:\DxlibFile` → `C:\DxLib` に統一 (`AdditionalIncludeDirectories` 行 66、`AdditionalLibraryDirectories` 行 72)
- Debug 構成 (`C:\DxLib`、行 50/55) と同一インストールを参照する形で揃え、設定ミスを解消
- 検証: Release|Win32 リビルド成功 (C1083 8 件解消、`Release\Project2.exe` 6,050,816 bytes 生成)、Debug|Win32 リビルドもリグレッションなし (警告 0 / エラー 0、`Debug\Project2.exe` 11,734,528 bytes)

### フェーズ α-3: GameStatus.h ビットフラグ値修正 (完了, 2026-06-25)

- [Project2/GameStatus.h](Project2/GameStatus.h) の `GAMECLEAR 0x00000016` (=22) → `0x00000010`、`GAMEEND 0x00000032` (=50) → `0x00000020` に修正
- 既存マクロ `GAMEINIT=0x01` / `GAMETITLE=0x02` / `GAMEMAIN=0x04` / `GAMEOVER=0x08` の倍々ビット進行と整合
- 安全性: `game_status` は当時 GameMain.cpp:6 で書き込み専用 + 参照ゼロのため値変更は無害 (α-4 で宣言削除)
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0

### フェーズ α-5: vcxproj.FileListAbsolute.txt 旧パス残骸クリーンアップ (完了, 2026-06-25)

- `Project2/Debug/Project2.vcxproj.FileListAbsolute.txt` を削除。旧 `D:\Visual Studio Repository\Scene管理付き空プロジェクトRev2\...` パスが記録されていた
- MSBuild が次回ビルドで新パス基準に自動再生成。クリーンビルドで 0 bytes の placeholder が生成され、その後の通常ビルドで現行パスが記録される
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0

### フェーズ α-4: 未使用グローバル削除 (完了, 2026-06-25)

- [Project2/MenuScene.cpp:12](Project2/MenuScene.cpp) の `int startfont;` 削除 (参照ゼロ、grep 確認済)
- [Project2/GameMain.cpp:6](Project2/GameMain.cpp) の `int game_status = GAMETITLE;` 削除 (書き込み専用、参照ゼロ)。TwistTimeStopper β-A-4 と同方針: 宣言は削除、`GameStatus.h` 本体マクロ群は将来用 placeholder として保持
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0

### フェーズ α-7: シーン番号詰め + 雛形保存 (完了, 2026-06-25)

ユーザー判断「Game2 を空シーン雛形として保存、シーン番号は詰める」採用。

- **ファイル削除**: 旧 `Game3Scene.cpp/.h` (誤コピペ "ゲーム画面１です" 持ち死蔵スタブ)、旧 `Game5Scene.cpp/.h` (内容は新 Game3Scene へ移動)
- **ファイル循環シフト** (PowerShell Move-Item、TEMP 経由で衝突回避):
  - 旧 `Game4Scene.*` (まきもどり、488行) → 新 `Game2Scene.*`
  - 旧 `Game5Scene.*` (あまちゃん、66行) → 新 `Game3Scene.*`
  - 旧 `Game2Scene.*` (死蔵スタブ、36行) → 新 `Game4Scene.*` (空シーン雛形)
- **内部の関数名置換** (PowerShell 一括置換、BOM 保持):
  - 新 Game2Scene: `Game4Scene` → `Game2Scene` 7 + 5 箇所 (cpp/h)
  - 新 Game3Scene: `Game5Scene` → `Game3Scene` 6 + 5 箇所、`SCENE_GAME4` → `SCENE_GAME2` 1 箇所 (あまちゃん→まきもどり遷移)
  - 新 Game4Scene: `Game2Scene` → `Game4Scene` (Edit ツールで 6+5 箇所)、描画文字列 `ゲーム画面２です` → `ゲーム画面４です`
- **GameSceneMain.cpp**: `#include "Game5Scene.h"` 削除、5 つの switch から `case SCENE_GAME5:` 削除
- **GameSceneMain.h**: enum から `SCENE_GAME5` 削除、各メンバーのコメントを役割 (ふつう/まきもどり/あまちゃん/雛形) で書き直し
- **MenuScene.cpp:7**: `menu[] = { SCENE_GAME1, SCENE_GAME5, SCENE_GAME4 }` → `{ SCENE_GAME1, SCENE_GAME3, SCENE_GAME2 }` (表示順はふつう/あまちゃん/まきもどり不変)
- **Project2.vcxproj** + **Project2.vcxproj.filters**: `Game5Scene.cpp/.h` エントリ削除
- 検証: Project2/Debug, Project2/Release, ルート Debug/Release を一旦削除 (PDB ステール対策) してから Debug|Win32 リビルド成功、警告 0 / エラー 0。`Grep "Game5Scene|SCENE_GAME5"` でソース 0 件

### フェーズ α-6: インクルードガード正規化 (完了, 2026-06-25)

実態調査で判明: 元々ガードが存在したのは `GameSceneMain.h` 1 ファイルのみ (`__GAMESCENEMAIN_H_`)、他 8 ファイルはガード自体が無かった (プロトタイプ宣言のみのヘッダだったため再宣言で済んでいた)。

- `GameSceneMain.h`: `__GAMESCENEMAIN_H_` → `GAMESCENEMAIN_H_` (予約識別子 `__` 前置 + `_` 後置の除去)
- `GameMain.h`: `#pragma once` 追加 (TwistTimeStopper の例外規約)
- Game1〜4Scene.h: `GAMESCENE<N>_H_` 新規追加 (4 ファイル)
- `MenuScene.h`: `MENUSCENE_H_` 新規追加
- `GameStatus.h`: `GAMESTATUS_H_` 新規追加
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0

### フェーズ α-8: dead code コメント整理 (完了, 2026-06-25)

- [Project2/GameMain.cpp:27-34](Project2/GameMain.cpp) — `/* … */` 旧 MessageBox ウィンドウモード確認ブロック削除
- Project2/Game1Scene.cpp, Project2/Game2Scene.cpp (=まきもどり、α-7 後) — `//int back;`, `//back = LoadGraph("res/board1212.png");`, `//SetDrawBlendMode(...)`, `//DrawGraph(100, 0, back, FALSE);`, `/*テスト用：マウス座標取得 ... //テスト用ここまで*/` ブロック削除 (両ファイル同種パターン)
- 保持: `//プログラムで格子を描く（上ではうまく行かない）`、`//Check:` / `//check:` 接頭辞のコメント、Game3Scene の `//ワープさせたい...` / `//問題はここよここ` TODO 注記、MenuScene の `//無論ここにもBGMを。`
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0。grep で削除対象パターン 0 件確認

### フェーズ β-A: dead include 一掃 + `itoa` セキュア化 + #pragma 4996 撤去 (完了, 2026-06-25)

- Game1Scene.cpp / Game2Scene.cpp の `#include <math.h>` / `#include <stdio.h>` を削除し `#include <stdlib.h>` を追加 (`_itoa_s` 用)
- `itoa(pcnum[0], blackC, 10);` × 4 箇所 → `_itoa_s(pcnum[0], blackC, sizeof(blackC), 10);` に置換 (Game1Scene 2 箇所、Game2Scene 2 箇所)
- [GameMain.h](Project2/GameMain.h) の `#pragma warning(disable : 4996)` 撤去 (セキュア化により不要)
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0

### フェーズ β-D-3: MyOutputDebugString マクロ昇格 (完了, 2026-06-25)

- [GameMain.h](Project2/GameMain.h) に TwistTimeStopper 流の `MyOutputDebugString` マクロを追加。`<stdio.h>`/`<Windows.h>`/`<tchar.h>` も同梱
- `#ifdef _DEBUG` で実体化 / Release は空展開、内部は `_stprintf_s` セキュア版 (β-A の #pragma 撤去と整合)
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0。β-D-1 のフォールバックロジックで使用開始

### フェーズ β-C: 状態値 enum 化 (完了, 2026-06-25)

- [GameSceneMain.h](Project2/GameSceneMain.h) に 3 つの enum を追加:
  - `GAME_STATUS` (PLAYING=1 / TURN_MSG / PASS_MSG / FINISHED) — Game1Scene/Game2Scene 共通
  - `GAME_TURN` (BLACK=1 / WHITE=2) — `turn = 3 - turn` の bit 反転算術と整合
  - `GAME_ROUND` (FIRST=1 / SECOND=2) — Game2Scene 専用 (まきもどり 2 ラウンド制)
- Game1Scene.cpp / Game2Scene.cpp の `status` / `turn` / `CurrentRound` の数字リテラル比較・代入を全置換
- `turn = (GAME_TURN)(3 - (int)turn);` のキャストで bit 反転算術を維持 (関数化は β-D-5 まで保留)
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0。enum 値が元の整数値と一致するため動作不変

### フェーズ β-D-1: シーンディスパッチャ共通化 (完了, 2026-06-25)

- [GameSceneMain.cpp](Project2/GameSceneMain.cpp) 全面書き換え (TwistTimeStopper β-D-1 を移植)
- `SCENE_HANDLERS` 構造体 (5 関数ポインタ) + `static const SCENE_HANDLERS sceneTable[SCENE_MAX]` 位置初期化 (C++14 互換)
- 5 つの `switch (sceneNo)` (CollideCallback / initCurrentScene / move / render / release) を `if (isValidSceneIndex(sceneNo)) sceneTable[sceneNo].xxx();` の 1 行に圧縮
- `prevScene` 削除、`MessageBox` 5 箇所撤去 → `MyOutputDebugString` + `assert(0 && "...")` に置換
- `initCurrentScene` を `BOOL` 戻り値化、`FrameMove` で失敗時 SCENE_MENU フォールバック → それも失敗なら SCENE_NONE 諦め (無限ループ防止: nextScene = sceneNo)
- `<assert.h>` include 追加
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0。grep で `MessageBox` / `prevScene` / `switch (sceneNo)` すべて 0 件

### フェーズ β-D-4: マジックナンバー定数化 (完了, 2026-06-25)

- [GameMain.h](Project2/GameMain.h) に定数群を集約:
  - 画面: `SCREEN_WIDTH=800` / `SCREEN_HEIGHT=700` / `SCREEN_BPP=16` (TwistTimeStopper は 24)
  - FPS: `FPS=60` / `MS_PER_SEC=1000`
  - 盤面: `BOARD_SIZE=12` / `BOARD_SIZE_MAX=11` / `CELL_PX=48` / `BOARD_ORIGIN_X/Y=5` / `BOARD_END_PX=580` / `PIECE_SIZE_PX=47` / `BOARD_CENTER_LOW=5` / `BOARD_CENTER_HIGH=6`
  - パネル/メッセージレイアウト座標多数 (`PANEL_X 590`, `MSG_BOX_CENTER_X 192` 等)
  - フォント: `FONT_SIZE_DEFAULT=32`、メッセージ表示時間 `MSG_WAIT_FRAMES=60`
- GameMain.cpp の `SetGraphMode(800, 700, 16)` / `1000 / 60`、MenuScene.cpp の `SetFontSize(32)` 置換
- Game1Scene.cpp / Game2Scene.cpp で `12` (for ループ)、`11` (境界)、`48`/`47` (CELL/PIECE)、`5/6` (初期駒位置)、`60` (msg_wait)、`mx / 48` (マウス座標)、`GetRand(11)` を順次置換
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0。グリッド描画の細部レイアウト座標 (`5, 580, 48`) は β-D-5 で盤面ロジック統一と同時に対応予定

### フェーズ β-D-2: 色グローバル集約 (完了, 2026-06-25)

- [GameMain.cpp](Project2/GameMain.cpp) に `ColorWhite/Red/Sky` を集約定義 (`GetColor(255,255,255)`, `(255,0,0)`, `(40,235,255)`)
- [GameMain.h](Project2/GameMain.h) に `extern unsigned int ColorWhite, ColorRed, ColorSky;` 公開
- Game1Scene.cpp の `unsigned int ColorWhite/Red/Sky = ...` 3 行削除 (extern 経由で参照)
- Game2Scene.cpp の `ColorWhite2`/`ColorRed2`/`ColorSky2` 全参照 (合計 22 箇所) を接尾辞なしに置換 + 定義 3 行削除
- 検証: Debug|Win32 リビルド成功、警告 0 / エラー 0。grep で `ColorXxx2` 全消滅、定義は GameMain.cpp のみ確認

### フェーズ β-D-5: 盤面ロジック単一化 (完了, 2026-06-25)

Game1Scene / Game2Scene のクローンコード約 80% を `ReversiBoard` 構造体 + `rb*` 関数群に統合。フェーズ β を完走。

- **構造体定義** ([GameSceneMain.h](Project2/GameSceneMain.h)): `_ReversiBoard { int board[BOARD_SIZE][BOARD_SIZE]; std::string msg; int msg_wait; }`。TwistTimeStopper `TIMER_STATE` と同方式 (シーンごとに `static ReversiBoard state = {};` で保持)
- **追加 include** ([GameSceneMain.h](Project2/GameSceneMain.h)): `<string>` を追加 (struct メンバ用)。Game1/Game2Scene.cpp の自前 `<string>` include は削除
- **9 関数を [GameSceneMain.cpp](Project2/GameSceneMain.cpp) に移植**: `rbInit` / `rbPutPiece` / `rbIsPass` / `rbThinkPlayer` / `rbThinkCpu` / `rbSetMsg` / `rbCheckResult` / `rbCountPieces` / `rbRemovePieces`。Game1Scene 版ロジックをベースに `state->board` 経由の参照に書き換え。`mouse_flag` は `rbThinkPlayer` 関数内 static のまま (動作完全等価)
- **Game1Scene.cpp 移行**: グローバル `SqBoard`/`msg`/`msg_wait` 削除、`putPiece`/`isPass`/`think1`/`think2`/`setMsg`/`checkResult` 関数削除。`static ReversiBoard state = {};` 追加。思考テーブルを `{ rbThinkPlayer, rbThinkCpu }` に変更。グリッド描画マジックナンバー (`5/580/48/11`) を `BOARD_ORIGIN_X / BOARD_END_PX / CELL_PX / BOARD_SIZE_MAX` に置換、パネル/メッセージ座標も `PANEL_X` 等の定数に統一。371 → 203 行 (Δ ‑168)
- **Game2Scene.cpp 移行**: グローバル `SqBoardA`/`msg2`/`msg_wait2` 削除、`putPiece2`/`isPass2`/`think01`/`think02`/`setMsg2`/`checkResult2`/`removePiece` 関数削除。Game1 と同様の置換 + `removePiece()` → `rbRemovePieces(&state, 96)`。Game2 専用要素 (`CurrentRound`, `excuted`, `changeBGM`, ラウンド遷移ロジック, ランダム終了メッセージ) は保持。452 → 273 行 (Δ ‑179)
- **行数削減実績**: Game1Scene ‑168 / Game2Scene ‑179 / GameSceneMain.h +36 / GameSceneMain.cpp +154 = **合計 ‑157 行** (`git diff --stat` ベース: +307 / ‑460)
- **検証**:
  - 中間ビルド 3 回 (GameSceneMain 拡張後 / Game1Scene 移行後 / Game2Scene 移行後) + 最終 Debug + Release リビルド: いずれも警告 0 / エラー 0
  - grep: `SqBoard`/`SqBoardA` 全消滅、`putPiece`/`putPiece2`/`isPass`/`isPass2`/`think1`/`think2`/`think01`/`think02`/`setMsg`/`setMsg2`/`checkResult`/`checkResult2`/`removePiece` の実コード参照 0 件 (コメント言及のみ)。`std::string`/`int msg_wait` グローバル消滅
  - `rb*` 関数参照: Game1Scene 11 / Game2Scene 12 / GameSceneMain.cpp 16 / GameSceneMain.h 10 = 49 件
- **動作不変保証**: 8 方向反転ロジック / パス判定 / プレイヤー思考 / CPU 思考 (`GetRand(1)` 等値時入替) / メッセージ表示時間 / 初期駒配置 / 勝敗判定式 / removePieces ロジック はすべてコード移植のみで等価。盤面色 (Game1=暗緑/Game2=明緑) と終了表示位置 (Game1=Y150/Game2=Y330) の差分はシーン側に残置

### フェーズ β 完了サマリ (2026-06-25)

β-A / β-C / β-D-1 / β-D-2 / β-D-3 / β-D-4 / β-D-5 の **7 サブターゲット完了**。β-B はコメント方針 ([6. コメント方針](#6-コメント方針)) により撤回 (役割メモは保護対象)。次フェーズは γ (機能完成: フレーム駆動化 / Game2 ラウンド 2 終了正規化 / Game3 方針確定)。

---
