# CLAUDE.md — ReversedReversi プロジェクト情報

Visual Studio (MSVC) + DxLib による C++ リバーシゲーム。本体は [Project2.sln](Project2.sln) / [Project2/](Project2/) 配下。タイトルバー表記は「Reverse Reversi 1.1」、メニュー描画は「まきもどリバーシ Ver 1.1」([Project2/MenuScene.cpp:68](Project2/MenuScene.cpp#L68))。日本語 Windows 環境 (コードページ 932) でビルドする前提。

姉妹プロジェクトに [TwistTimeStopper](d:\Repositories\TwistTimeStopper) があり、シーン管理の雛形を共有している (元は同じ「Scene管理付き空プロジェクトRev2」テンプレート)。TwistTimeStopper は既に α/β/γ/δ のリファクタを完走しており、共通基盤化のリファレンスとして本ファイル中で頻繁に参照する。

---

## 1. プロジェクト概要

**まきもどリバーシ** は 12×12 盤面のリバーシ亜種。メニューから 3 モードを選択する構成だが、現状は **2 モード完成 + 1 モード未完成**:

| メニュー位置 | 表示名 | SCENE_NO | ファイル | 状態 |
|---|---|---|---|---|
| 0 | `ふつうの次元：頭をほどよく使う` | `SCENE_GAME1` | [Game1Scene.cpp](Project2/Game1Scene.cpp) (405 行) | **完成** |
| 1 | `あまちゃん次元：頭をあまり使わない` | `SCENE_GAME5` | [Game5Scene.cpp](Project2/Game5Scene.cpp) (66 行) | **未完成** (名前入力のみ→ Game4 へ丸投げ) |
| 2 | `まきもどり次元：頭をかなり使う` | `SCENE_GAME4` | [Game4Scene.cpp](Project2/Game4Scene.cpp) (488 行) | **完成** |

メニュー配列実体: [MenuScene.cpp:7](Project2/MenuScene.cpp#L7) `SCENE_NO menu[3] = { SCENE_GAME1, SCENE_GAME5, SCENE_GAME4 };`

### 2 ラウンド制 (まきもどりモード)

プロジェクト名「まきもど」の所以は **`SCENE_GAME4` (まきもどり次元)** の 2 ラウンド構造:

- ラウンド 1: 通常のリバーシを進行
- ラウンド間: `removePiece()` で 96 マスを削除して盤面を巻き戻し風にリセット (`CurrentRound` が 1→2 へ)
- ラウンド 2: 2 局目を開始、終局でゲーム終了処理

状態管理は [Game4Scene.cpp:14-15](Project2/Game4Scene.cpp#L14) のファイルスコープ `CurrentRound` / `excuted` グローバル。BGM もラウンドで切替 (`loop_68.wav` / `loop_95.wav`、`changeBGM` 関数)。**シーン再入場時のリセット処理が `initGame4Scene` に無い** ため、メニューに戻って再エントリすると `CurrentRound=2` のまま即終了表示になる既知の不具合あり (詳細は [10. 未完成機能の方針](#10-未完成機能の方針) を参照)。

### 死蔵スタブシーン

`SCENE_GAME2` / `SCENE_GAME3` は `menu[]` に登録されておらず、メニューから到達不能。中身は「ゲーム画面２です / ボタン１でタイトルに戻る」だけの完全スタブ ([Game2Scene.cpp](Project2/Game2Scene.cpp) 36 行 / [Game3Scene.cpp](Project2/Game3Scene.cpp) 38 行)。`Game3Scene` の描画文字列は「ゲーム画面１です」と誤コピペされたまま ([Game3Scene.cpp:27](Project2/Game3Scene.cpp#L27))。`SCENE_NO` enum と `switch` の case 列には残っており、リファクタ初期に整理対象となる。

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

[Project2/GameSceneMain.h:15-24](Project2/GameSceneMain.h#L15) で `SCENE_NO` enum を定義:

```c
typedef enum _SCENE_NO {
    SCENE_NONE = -1,    // 下限センチネル
    SCENE_MENU,         // メニュー
    SCENE_GAME1,        // ふつうの次元 (完成)
    SCENE_GAME2,        // 死蔵スタブ (menu[] 未登録)
    SCENE_GAME3,        // 死蔵スタブ (menu[] 未登録)
    SCENE_GAME4,        // まきもどり次元 (完成、2 ラウンド制)
    SCENE_GAME5,        // あまちゃん次元 (未完成、名前入力のみ)
    SCENE_MAX           // 上限センチネル
} SCENE_NO;
```

[Project2/GameSceneMain.cpp](Project2/GameSceneMain.cpp) で `static SCENE_NO sceneNo, prevScene, nextScene` を管理。

### ⚠️ ディスパッチは switch のコピペ 5 箇所 (TwistTimeStopper 未踏)

[Project2/GameSceneMain.cpp](Project2/GameSceneMain.cpp) では `switch (sceneNo)` を **`CollideCallback` / `initCurrentScene` / `moveCurrentScene` / `renderCurrentScene` / `releaseCurrentScene` の 5 関数で同じ case 列をコピペ**して 5 箇所のディスパッチを実現している (それぞれが MENU/GAME1〜GAME5 の 7 ケース)。シーン追加コストは「enum 1 行 + switch 5 箇所 + 関数プロトタイプ追加」。

**TwistTimeStopper では β-D-1 で `SCENE_HANDLERS sceneTable[SCENE_MAX]` の関数ポインタテーブルへ移行済み**。1 行ディスパッチ化することでシーン追加コストが 5 箇所 → 2 箇所になり、`prevScene` の削除と `MessageBox` フォールバックの撤去も同時に達成された ([d:/Repositories/TwistTimeStopper/Project2/GameSceneMain.cpp:46-61](d:/Repositories/TwistTimeStopper/Project2/GameSceneMain.cpp#L46-L61) 参照)。本プロジェクトの **β-D-1 相当の作業候補**。

### 4 関数命名規約 (シーンごと)

各シーン名 `XXXScene` (Menu / Game1 / Game2 / Game3 / Game4 / Game5) に対し:

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

### ⚠️ MessageBox によるブロッキング (5 箇所)

不正な `sceneNo` や `default` ケースで [GameSceneMain.cpp:76,105,121,137,153](Project2/GameSceneMain.cpp) に `MessageBox(NULL, "まだそのシーンはない", "作成途中", 0);` が並ぶ。フレーム処理中にモーダル表示するためフリーズ感を生む。TwistTimeStopper β-D-1 では `MyOutputDebugString + assert` に置換済み — 本プロジェクトでも置換候補。

### ⚠️ Game1Scene / Game4Scene の `while (!ProcessMessage())` 独自ループ

[Game1Scene.cpp:208](Project2/Game1Scene.cpp#L208) と [Game4Scene.cpp:231](Project2/Game4Scene.cpp#L231) の `move*` 関数内に **独自の `while (!ProcessMessage())` 無限ループ**が組まれており、メインループの 1 フレーム 1 ティック設計を完全に無視している。一度入ると ESC か終了条件まで戻ってこないため、`FrameMove` / `RenderScene` のフレーム駆動モデルが事実上崩壊している。

**本プロジェクト最大の構造的不具合**。リファクタの優先課題で、状態 (`status`, `turn`, `CurrentRound`, `msg`, `msg_wait`, `SqBoard` 等) を `static` 化してフレーム駆動に書き換えるのが望ましい。

### ⚠️ CollideCallback の制約

`MenuSceneCollideCallback` 上に「**ここでは要素を削除しないこと！！**」コメントあり。DxLib の衝突判定列挙の最中に呼ばれるため、コールバック内で対象オブジェクトを `delete` するとイテレータ破壊で UB になる。削除はフラグを立てて `moveXXXScene` 内で実施するパターン (TwistTimeStopper 規約と同じ)。

### TwistTimeStopper と共通化可能な箇所

- **シーンディスパッチ表化**: switch 5 箇所 → 関数ポインタテーブル 1 つに圧縮 (β-D-1 相当)
- **`init` 戻り値の活用と SCENE_MENU フォールバック**: β-D-3 相当
- **`MessageBox` → `MyOutputDebugString + assert`**: プラットフォーム依存解消 (β-D-1 相当)
- **インクルードガードを `__XXX_H_` → `XXX_H_` 統一**: 予約識別子の回避 ([5. 命名規約](#5-命名規約) 参照)
- **盤面ロジックの単一化**: Game1Scene と Game4Scene の 80% 以上が `2`/`A` 接尾辞違いのコードクローン。`ReversiBoard` 構造体 (`int board[12][12]`, `std::string msg`, `int msg_wait`) と `putPiece(state, x, y, ...)` のような state 引数渡しに統合できる (488 + 405 行から数百行削減可能)
- **シーン間共有構造体**: TwistTimeStopper の `TIMER_STATE` に相当する `ReversiSession` (`SqBoard`, `nameTmp`, `CurrentRound` 等) を `GameSceneMain.h` で定義してシーン間で共有

---

## 5. 命名規約

### インクルードガード (現状)

ReversedReversi は **`__GAMESCENEMAIN_H_` 形式** ([GameSceneMain.h:1](Project2/GameSceneMain.h#L1)) で **予約識別子** (`__` 始まり + 末尾 `_` 後置) を使用。標準上 UB 扱い。

**改善方針**: TwistTimeStopper と同じ `<NAME>_H_` 形式に統一する (β 候補):

| ヘッダ | 現状 → 改善後 |
|---|---|
| GameMain.h | (#pragma once か `GAMEMAIN_H_`) |
| MenuScene.h | `__MENUSCENE_H_` → `MENUSCENE_H_` |
| Game1Scene.h | `__GAMESCENE1_H_` → `GAMESCENE1_H_` |
| Game2Scene.h | `__GAMESCENE2_H_` → `GAMESCENE2_H_` |
| Game3Scene.h | `__GAMESCENE3_H_` → `GAMESCENE3_H_` |
| Game4Scene.h | `__GAMESCENE4_H_` → `GAMESCENE4_H_` |
| Game5Scene.h | `__GAMESCENE5_H_` → `GAMESCENE5_H_` |
| GameStatus.h | `__GAMESTATUS_H_` → `GAMESTATUS_H_` |
| GameSceneMain.h | `__GAMESCENEMAIN_H_` → `GAMESCENEMAIN_H_` |

Game1〜5Scene は **`GAMESCENE<N>_H_`** (数字は GAMESCENE の後)。`GAME<N>SCENE_H_` ではない点に注意 (TwistTimeStopper 流)。

### 関数命名

- シーン関数: `initXXXScene` / `moveXXXScene` / `renderXXXScene` / `releaseXXXScene` / `XXXSceneCollideCallback` ([4. シーンアーキテクチャ](#4-シーンアーキテクチャ) 参照)
- 補助関数: ローワーキャメル (`putPiece`, `isPass`, `checkResult`, `setMsg`, `think1`, `think2`)
- Game4 の補助関数群は `*2` / `*02` サフィックスで Game1 と分離 (`putPiece2`, `think01`, `think02`, `setMsg2`, `checkResult2`) — **完全コードクローンの臭み**。リファクタ対象

### 変数命名

- グローバル: 名詞 (`SqBoard`, `SqBoardA`, `Input`, `EdgeInput`, `nameTmp`, `CurrentRound`, `excuted`, `game_status`, `FrameStartTime`)
  - `SqBoard` (Game1) と `SqBoardA` (Game4) も同様の `2`/`A` 接尾辞重複
- ローカル: ローワーキャメル / 短縮 (`wx`, `wy`, `wn`, `kx`, `ky`, `turn`)
- 定数: マクロ大文字 (`MENU_MAX`, `PAD_INPUT_*`, `KEY_INPUT_*`)

### マジックナンバー集中地帯 (改善余地)

`Game1Scene.cpp` / `Game4Scene.cpp` 全域に `12` (盤面サイズ), `48` (セルサイズ), `5`, `580`, `47`, `192`, `630`, `655`, `590`, `40/125`, `680`, `100/700/800` 等が散在。命名定数は `#define MENU_MAX 3` のみ。TwistTimeStopper β-D-4 と同じ手順で `GameMain.h` への定数集約が必要。

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

### 現状の dead code コメント残骸 (フェーズ α 候補)

- [GameMain.cpp:27-34](Project2/GameMain.cpp#L27) — `/* … */` で囲まれた旧 MessageBox ウィンドウモード確認ブロック
- [Game1Scene.cpp:187,196,198,263,265,292-317](Project2/Game1Scene.cpp) — `back = LoadGraph(...)`, `SetDrawBlendMode`, テスト用マウスデバッグの大量残骸
- [Game4Scene.cpp:210,219,221,298,329-353](Project2/Game4Scene.cpp) — Game1 と同種の残骸
- [Game5Scene.cpp:40,46](Project2/Game5Scene.cpp) — `//ワープさせたい。例えば：1=通常モード　4=まきもどりモード` / `//問題はここよここ` (TODO 注記)
- [MenuScene.cpp:25](Project2/MenuScene.cpp#L25) — `//無論ここにもBGMを。` (将来用メモ、保留判定)

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

Game1Scene / Game4Scene では `GetMousePoint` 系を使って盤面クリック判定 ([Game1Scene.cpp:113](Project2/Game1Scene.cpp#L113), [Game4Scene.cpp:115](Project2/Game4Scene.cpp#L115))。`x / 48` で盤面インデックス化するため、**範囲外クリック (x>576) で `SqBoard[15][...]` のような領域外アクセスが起こる** UB あり (リファクタ候補)。

### Game5 のキーボード文字入力

[Game5Scene.cpp](Project2/Game5Scene.cpp) で `KeyInputSingleCharString` を使って `nameTmp[12]` に 1 文字ずつ蓄積。`nameTmp[0] != NULL` 検知で `SCENE_GAME4` へ遷移する仮実装。`'\0'` または `0` と比較すべきで、`NULL` 比較は `char` と `void*` の暗黙変換警告対象 ([10. 未完成機能の方針](#10-未完成機能の方針) 参照)。

---

## 8. DxLib のお作法 (細かい点)

- **フォント**: `SetFontSize(N)` / `ChangeFontType(DX_FONTTYPE_*)` / `ChangeFont("ＭＳ 明朝")` はグローバル状態なので、各シーンの init で必ず設定し直す ([MenuScene.cpp:20-21](Project2/MenuScene.cpp#L20))。**ReversedReversi では `ChangeFont("ＭＳ 明朝")` が毎フレーム呼ばれているシーンがある** (Game1/Game4 の render 内) — 本来 init 時 1 回でよく、TwistTimeStopper では init で完結している
- **色**: `GetColor(R, G, B)` の戻り値 `unsigned int` は本来キャッシュすべき。本プロジェクトでは `ColorWhite`, `ColorRed`, `ColorSky` 系のグローバルが Game1/Game4 で別々に定義されている (`ColorWhite2` 等) — TwistTimeStopper β-D-2a 相当の集約候補
- **座標系**: 原点 (0, 0) は **画面左上**。`DrawString(x, y, str, color)` で直接指定
- **乱数**: `GetRand(n)` で 0〜n-1。シードは起動時に `GetNowCount()` で `SRand` 初期化する慣習 (TwistTimeStopper では `WinMain` 冒頭)
- **画像**: `LoadGraph` / `LoadDivGraph` の戻り値ハンドル `int` を保持し `DeleteGraph` で解放。本プロジェクトでは Game1/Game4 で `LoadDivGraph` が `moveXXXScene` 内で呼ばれていて毎フレーム再ロードする恐れ + `releaseXXXScene` で `DeleteGraph` していない → **リソースリーク**
- **音**: `PlaySoundFile(path, DX_PLAYTYPE_LOOP / BACK / NORMAL)`。`StopSoundFile()` でループ停止。**MenuScene と Game4 は BGM 切替を行うが、Game1 にも `StopSoundFile` を入れて遷移時の BGM クリーンアップを徹底する必要あり**
- **デバッグ出力**: TwistTimeStopper 流の `MyOutputDebugString(...)` マクロは未導入。`<stdio.h>`/`<Windows.h>`/`<tchar.h>` を `GameMain.h` に集約して移植するのが β-D-3 相当
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

[GameMain.h:5](Project2/GameMain.h#L5) に `#pragma warning(disable : 4996)` で全黙殺中。`itoa`/`lstrcpy`/`strcpy` 等が対象と推測。本来は安全な代替 (`strncpy_s`, `snprintf` 等) に書き換えるべきで、`#pragma` 撤去を β 候補に追加。

---

## 10. 未完成機能の方針

完成させるかどうかをカテゴリ別に明示:

### 完成済み

- **`SCENE_GAME1` (ふつうの次元)** — 12×12 リバーシ本体、`think1`(人) vs `think2`(CPU)、勝敗判定まで実装。ただし `while (!ProcessMessage())` 独自ループによるフレーム駆動破壊と、シーン再入場時の `SqBoard` リセット欠落あり ([4. シーンアーキテクチャ](#4-シーンアーキテクチャ) 参照)
- **`SCENE_GAME4` (まきもどり次元)** — Game1 の拡張版、2 ラウンド制 (`CurrentRound`)、ラウンド間で `removePiece()` で 96 マス削除、BGM 切替 (`changeBGM`)。Game1 の問題点に加えて、ラウンド 2 終了時に `releaseGame4Scene()` を直接呼んで `DxLib_End()` を叩くシーン管理違反あり ([Game4Scene.cpp:286-291](Project2/Game4Scene.cpp#L286), [Game4Scene.cpp:480-484](Project2/Game4Scene.cpp#L480))

### 未完成 (要対応)

- **`SCENE_GAME5` (あまちゃん次元)** — 名前入力 (`KeyInputSingleCharString` で `nameTmp`) だけ実装。コメントに「ワープさせたい。例えば：1=通常モード　4=まきもどりモード」とあるが、現実装は `nameTmp[0] != NULL` 検知で無条件 `SCENE_GAME4` 遷移。
  - 設計意図 (推測): 名前入力後に独自の「あまちゃん」モードでプレイさせたかった
  - 現実装: Game4 に丸投げで Game5 独自ゲームロジックなし
  - **方針候補**:
    1. **(現状維持)** Game4 への橋渡し専用と割り切り、名前入力 UI を完成させて `nameTmp` を Game4 で表示反映する小修正に留める
    2. **(独自モード化)** Game1/Game4 を共通化した後、`SCENE_GAME5` 専用の簡易ルール (例: AI の手数制限、盤面サイズ違い) を追加する
    3. **(削除統合)** `SCENE_GAME5` を削除してメニュー 2 項目化、`nameTmp` をメニュー側に移動

### 死蔵スタブ (整理対象)

- **`SCENE_GAME2` / `SCENE_GAME3`** — `menu[]` に登録されておらず到達不能。中身は `MessageBox` 風の「ゲーム画面２です」描画のみ ([Game2Scene.cpp](Project2/Game2Scene.cpp), [Game3Scene.cpp](Project2/Game3Scene.cpp))。
  - **方針候補**:
    1. **(削除)** `SCENE_NO` enum と `switch` 5 箇所、`Project2.vcxproj` の ClCompile/ClInclude、`.h`/`.cpp` を全削除 (α 候補、推奨)
    2. **(雛形化)** TwistTimeStopper の `Game4Scene` 流に「空シーン雛形」として残し、新シーン追加時のコピー元として活用

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

候補:
1. ✅ **エンコーディング統一** (完了, 2026-06-25): 17 ファイル CP932 → UTF-8 BOM 化、`.editorconfig` 新設 ([2. ファイルエンコーディング方針](#2-ファイルエンコーディング方針-utf-8-bom-統一))
2. ✅ **Release 構成のビルド復活** (完了, 2026-06-25): DxLib パスを `C:\DxlibFile` → `C:\DxLib` に統一 ([3. ビルド環境](#3-ビルド環境))
3. **インクルードガード正規化**: `__XXX_H_` → `XXX_H_` (8 ファイル)
4. **死蔵スタブ削除**: `SCENE_GAME2` / `SCENE_GAME3` 一式 (enum 値、case、`.h`/`.cpp`、vcxproj 登録) — または「空シーン雛形」として残す方針判断要
5. **dead code コメント整理**: [6. コメント方針](#6-コメント方針) で列挙した残骸の削除 ([Game1Scene.cpp:187-317](Project2/Game1Scene.cpp), [Game4Scene.cpp:210-353](Project2/Game4Scene.cpp), [GameMain.cpp:27-34](Project2/GameMain.cpp))
6. **`Project2/Debug/Project2.vcxproj.FileListAbsolute.txt` の旧パス残骸クリーンアップ**
7. **`GameStatus.h` のビットフラグ値修正**: `0x00000016` (=22) / `0x00000032` (=50) を `0x10` / `0x20` に修正 (現状 `game_status` は書き込み専用で参照ゼロのため安全)
8. **未使用グローバル削除**: `int startfont;` ([MenuScene.cpp:12](Project2/MenuScene.cpp#L12))、その他参照ゼロのもの

### フェーズ β: 中リスク整理 (予定)

dead code 一掃 / マジックナンバー定数化 / シーン構造の整理。

候補:
1. **β-A: 残った dead code 一掃** — 未使用 include の整理、`#pragma warning(disable: 4996)` 撤去の可否検討
2. **β-B: 過剰コメント整理 — 保留** (TwistTimeStopper と同じく [6. コメント方針](#6-コメント方針) により、役割メモは保護対象)
3. **β-C: 状態値の enum 化** — `status` / `turn` / `CurrentRound` 等のマジックナンバー (0/1/2/3) を enum 化
4. **β-D: 構造系**
   - **β-D-1: シーンディスパッチャ共通化** — `switch` 5 箇所 → `SCENE_HANDLERS` 関数ポインタテーブル + `sceneTable` 配列 + 1 行ディスパッチ (TwistTimeStopper の β-D-1 を移植)
   - **β-D-2: 色 / フォント / 共通リソース集約** — `ColorWhite` 系を `GameMain.cpp` に集約 + `extern` 公開、`ChangeFont` の init 集約
   - **β-D-3: エラーハンドリング強化** — `MyOutputDebugString` マクロ昇格、`init` 戻り値の活用、`MessageBox` 撤去
   - **β-D-4: マジックナンバー定数化** — 画面 (`SCREEN_WIDTH/HEIGHT/BPP`)、FPS、盤面 (`BOARD_SIZE 12`, `CELL_PX 48`)、レイアウト座標を `GameMain.h` に集約
   - **β-D-5: 盤面ロジック単一化** — `Game1Scene` と `Game4Scene` の `putPiece` / `putPiece2` / `think*` / `setMsg*` を `ReversiBoard` 構造体経由の関数群に統合 (約 800 行から 200〜300 行削減見込)

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

---
