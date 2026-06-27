# CLAUDE.md — ReversedReversi プロジェクト情報

Visual Studio (MSVC) + DxLib による C++ リバーシゲーム。本体は [Project2.sln](Project2.sln) / [Project2/](Project2/) 配下。タイトルバー表記は「Reverse Reversi 1.6.0」、メニュー描画は「まきもどリバーシ Ver 1.6.0」([Project2/MenuScene.cpp:67](Project2/MenuScene.cpp#L67))。日本語 Windows 環境 (コードページ 932) でビルドする前提。バージョン履歴は [CHANGELOG.md](CHANGELOG.md)、ライセンスは [LICENSE.md](LICENSE.md) (MIT)、公開向け案内は [README.md](README.md) を参照 (採番ルール: フェーズ MINOR + サブターゲット PATCH + **挙動不変なコード変更は SUBPATCH** (4 セグメント、1.5.6.1 で導入)、ドキュメント/メタファイルのみの変更は据え置き、**大型機能リリース (B2 ランクシステム等) はフェーズ転換相当として MINOR バンプ可 — 1.6.0 で B2 ランクシステム実装時に採用** — CHANGELOG 冒頭参照)。

姉妹プロジェクトに [TwistTimeStopper](d:\Repositories\TwistTimeStopper) があり、シーン管理の雛形を共有している (元は同じ「Scene管理付き空プロジェクトRev2」テンプレート)。TwistTimeStopper は既に α/β/γ/δ のリファクタを完走しており、共通基盤化のリファレンスとして本ファイル中で頻繁に参照する。

---

## 1. プロジェクト概要

**まきもどリバーシ** は 12×12 盤面のリバーシ亜種 (Game3 のみ 1.5.8 で 6/8/10/12 の 4 段階対応)。メニューから 3 モードを選択する構成で、γ-3 (2026-06-26) で **3 モード全て完成**:

| メニュー位置 | 表示名 | SCENE_NO | ファイル | 状態 |
|---|---|---|---|---|
| 0 | `ふつうの次元：頭をほどよく使う` | `SCENE_GAME1` | [Game1Scene.cpp](Project2/Game1Scene.cpp) | **完成** (プレイヤー vs `rbThinkCpu` 貪欲、12×12 固定) |
| 1 | `あまちゃん次元：頭をあまり使わない` | `SCENE_GAME3` | [Game3Scene.cpp](Project2/Game3Scene.cpp) | **完成** (名前入力 → プレイヤー vs `rbThinkRandom` ランダム CPU、1.5.8 で 6/8/10/12 サイズ可変) |
| 2 | `まきもどり次元：頭をかなり使う` | `SCENE_GAME2` | [Game2Scene.cpp](Project2/Game2Scene.cpp) | **完成** (2 ラウンド制、`rbRemovePieces` でラウンド間 96 マス削除、12×12 固定) |

メニュー配列実体: [MenuScene.cpp:7](Project2/MenuScene.cpp#L7) `SCENE_NO menu[4] = { SCENE_GAME1, SCENE_GAME3, SCENE_GAME2, SCENE_OPTIONS };` (1.5.7 で OPTIONS 追加)

メニュー非掲載の `SCENE_TEMPLATE` ([GameSceneTemplate.cpp/.h](Project2/GameSceneTemplate.cpp)) は **空シーン雛形** (新シーン追加時のコピー元、α-7 で確立、1.5.8 で `Game4Scene`→`GameSceneTemplate` リネームで固定名化)。enum 値とディスパッチには残るが menu[] からは到達不能。

### 2 ラウンド制 (まきもどりモード)

プロジェクト名「まきもど」の所以は **`SCENE_GAME2` (まきもどり次元)** の 2 ラウンド構造:

- ラウンド 1: 通常のリバーシを進行
- ラウンド間: `removePiece()` で 96 マスを削除して盤面を巻き戻し風にリセット (`CurrentRound` が 1→2 へ)
- ラウンド 2: 2 局目を開始、終局でゲーム終了処理

状態管理は [Game2Scene.cpp:14-15](Project2/Game2Scene.cpp#L14) のファイルスコープ `CurrentRound` / `excuted` グローバル。BGM もラウンドで切替 (`loop_68.wav` / `loop_95.wav`、`changeBGM` 関数)。**シーン再入場時のリセット処理が `initGame2Scene` に無い** ため、メニューに戻って再エントリすると `CurrentRound=2` のまま即終了表示になる既知の不具合あり (詳細は [10. 未完成機能の方針](#10-未完成機能の方針) を参照)。

### 空シーン雛形 (`SCENE_TEMPLATE`、1.5.8 で固定名化)

α-7 (2026-06-25) で旧 `SCENE_GAME2`/`SCENE_GAME3` の死蔵スタブを整理: 旧 `SCENE_GAME3` (誤コピペ持ち) は削除、旧 `SCENE_GAME2` を新 `SCENE_GAME4` に転用して **TwistTimeStopper の Game4Scene 方式に倣った「空シーン雛形」** に位置付け。1.5.8 (2026-06-27) でファイル名を `Game4Scene` → `GameSceneTemplate` に固定化 (ユーザー指示、シーン番号スライドで陳腐化する名前を避ける)、enum も `SCENE_GAME4` → `SCENE_TEMPLATE` に改名。`menu[]` には載せず、新シーン追加時の永続コピー元として保持する。

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

- **ソリューション**: [Project2.sln](Project2.sln) (構成は **Debug / Release × Win32 / x64 = 4 構成**、δ-3 で x64 追加)
- **プロジェクト**: [Project2/Project2.vcxproj](Project2/Project2.vcxproj)
- **PlatformToolset**: `v145` (TwistTimeStopper も同じ。一般的な VS2022 標準 `v143` ではない点に注意。VS 18 Insiders 系で動作確認)
- **CharacterSet**: `MultiByte` (MBCS)
- **LanguageStandard**: 未指定 (ツールセット既定、C++14 相当)
- **WindowsTargetPlatformVersion**: `10.0`
- **RuntimeLibrary**: Debug = `/MTd` (MultiThreadedDebug)、Release = `/MT` (MultiThreaded) — 静的 CRT
- **SubSystem**: 明示なし (ConfigurationType=Application で既定 Windows)

### DxLib

**α-2 (2026-06-25) で Debug / Release 両構成とも `C:\DxLib` に統一済み** (旧 Release 構成は `C:\DxlibFile` 不在で C1083 ×8 だった)。**δ-3 (2026-06-26) で [Common.props](Project2/Common.props) に DxLib パスを集約**: `<DxLibDir>C:\DxLib</DxLibDir>` UserMacro 1 箇所で全 4 構成 (Debug/Release × Win32/x64) のパス指定を一元化。ローカル環境で別の場所にインストールしている場合は Common.props の `DxLibDir` を 1 行書き換えるだけで済む。

DxLib ヘッダ ([DxDataTypeWin.h](file:///C:/DxLib/DxDataTypeWin.h)) が `_MSC_VER >= 1900` + `_WIN64` + `_MT`/`_MD` を自動判定して適切な lib を `#pragma comment(lib, ...)` でリンクする (本プロジェクトの v145 + `/MT` だと `DxLib_vs2015_x86_MT.lib` または `DxLib_vs2015_x64_MT.lib` 等が選択される)。`AdditionalLibraryDirectories = C:\DxLib` の指定だけで Win32/x64 両方が正しく解決される。

### ビルド結果 (2026-06-26、δ-3 完了時点)

| 構成 | プラットフォーム | 結果 | 警告 | エラー | 出力サイズ |
|---|---|---|---|---|---|
| Debug | Win32 | 成功 | 0 | 0 | `Debug\Project2.exe` 約 11.7 MB |
| Release | Win32 | 成功 | 0 | 0 | `Release\Project2.exe` 約 6.1 MB |
| Debug | x64 | 成功 | 0 | 0 | `x64\Debug\Project2.exe` 約 13.6 MB |
| Release | x64 | 成功 | 0 | 0 | `x64\Release\Project2.exe` 約 6.9 MB |

### 画面・フレーム

- **画面**: 1280×768 / **16bit カラー** ([GameMain.cpp:41](Project2/GameMain.cpp#L41) `SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP)`、定数は [GameMain.h](Project2/GameMain.h) で定義)。1.5.9 で 800×700 → 1280×768 に拡張 (1.6× / 1.097×、Y は Windows DPI 125% の 1080p ディスプレイでもタイトルバー + タスクバー込みで見切れない控えめ拡張)。TwistTimeStopper は 24bit、こちらは 16bit と差異あり。32bit が現代標準
- **盤面**: BOARD_TARGET_PX = 720px (1.5.9 で 576 → 720 に拡張)。サイズ別 CELL_PX は 6→120 / 8→90 / 10→72 / 12→60 で全て綺麗に割り切れる ([GameMain.h](Project2/GameMain.h))
- **ウィンドウ**: `ChangeWindowMode(true)` でウィンドウ起動 ([GameMain.cpp:25](Project2/GameMain.cpp#L25))。**`SetWindowSizeChangeEnableFlag(true)`** ([GameMain.cpp:38](Project2/GameMain.cpp#L38)) で実行中リサイズ可 — TwistTimeStopper にない設定。動作未確認のコメント (`//Check:実行中に画面の大きさが変更可能か`) あり
- **タイトル**: `SetMainWindowText("Reverse Reversi 1.5.9")` ([GameMain.cpp](Project2/GameMain.cpp))
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

[Project2/GameSceneMain.h](Project2/GameSceneMain.h) で `SCENE_NO` enum を定義 (α-7 で SCENE_GAME5 削除 + 番号詰め、1.5.7 で SCENE_OPTIONS 追加、1.5.8 で SCENE_GAME4 → SCENE_TEMPLATE リネーム):

```c
typedef enum _SCENE_NO {
    SCENE_NONE = -1,    // 下限センチネル
    SCENE_MENU,         // メニュー
    SCENE_GAME1,        // ふつうの次元 (完成、12×12 固定)
    SCENE_GAME2,        // まきもどり次元 (完成、12×12 固定、2 ラウンド制)
    SCENE_GAME3,        // あまちゃん次元 (完成、1.5.8 で 6/8/10/12 サイズ可変)
    SCENE_TEMPLATE,     // 空シーン雛形 (menu[] 非掲載、固定名コピー元、1.5.8 で旧 SCENE_GAME4 から改名)
    SCENE_OPTIONS,      // オプション設定画面 (1.5.7 で導入、Game3 のトグル + 1.5.8 で盤面サイズ)
    SCENE_MAX           // 上限センチネル
} SCENE_NO;
```

[Project2/GameSceneMain.cpp](Project2/GameSceneMain.cpp) で `static SCENE_NO sceneNo, prevScene, nextScene` を管理。

### ディスパッチはテーブル駆動 (β-D-1 完了、2026-06-25)

[Project2/GameSceneMain.cpp](Project2/GameSceneMain.cpp) で `SCENE_HANDLERS sceneTable[SCENE_MAX]` 関数ポインタ束を定義し、1 行ディスパッチ (`sceneTable[sceneNo].xxx()`) を実現済。新規シーン追加手順 (1.5.8 で雛形コピー慣習を明文化):

1. **[GameSceneTemplate.cpp/.h](Project2/GameSceneTemplate.cpp) を新ファイル名にコピー** (雛形ファイル自体は残置) — PowerShell `Copy-Item GameSceneTemplate.cpp <NewScene>.cpp` 等。BOM 保持される
2. コピーした新ファイル内の識別子を一括置換 (`GameSceneTemplate` → 新シーン名)
3. [GameSceneMain.h](Project2/GameSceneMain.h) の `SCENE_NO` enum に 1 値追加
4. [GameSceneMain.cpp](Project2/GameSceneMain.cpp) の `sceneTable[]` に 1 行追加 + `#include "<NewScene>.h"` 追記
5. [Project2.vcxproj](Project2/Project2.vcxproj) / [Project2.vcxproj.filters](Project2/Project2.vcxproj.filters) に ClCompile/ClInclude 追加

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

### ✅ Game1Scene / Game2Scene のフレーム駆動化 (γ-1 完了、2026-06-25)

旧仕様では `moveGame1Scene`/`moveGame2Scene` 内に独自の `while (!ProcessMessage())` 無限ループが組まれ、メインループの 1 フレーム 1 ティック設計を完全に無視していた (**本プロジェクト最大の構造的不具合**)。γ-1 で TwistTimeStopper 流のフレーム駆動モデルに書き直し済:

- 状態 (`status`, `turn`, `CurrentRound`, `excuted`, `roundTransitWait`, `finishedMsgRand`) と `ReversiBoard state` をファイルスコープ `static` 化
- `initGame1Scene` / `initGame2Scene` でシーン入場/再入場時に毎回リセット + リソース読込 (LoadDivGraph、フォント、BGM)
- `moveGame1Scene` / `moveGame2Scene` は `switch (status)` の 1 ティックのみ
- `renderGame1Scene` / `renderGame2Scene` に描画コードを分離 (描画 6 ブロックの共通関数化は δ-4 で完了)
- `releaseGame1Scene` / `releaseGame2Scene` で BGM 停止 + 画像ハンドル `DeleteGraph` 解放 (δ-1 前倒し、リソースリーク解消)
- メニュー復帰経路: 終了状態で `X` キー押下 → `changeScene(SCENE_MENU)` (TwistTimeStopper 流)
- Game2 ラウンド遷移は `roundTransitWait` カウンタで 240 フレーム (旧 `WaitTimer(4000)` 相当、フレームハング解消)
- Game2 ラウンド 1 終了メッセージ抽選は `finishedMsgRand` で 1 回固定 (旧フリッカー解消)
- `releaseGame2Scene` の `DxLib_End` 直呼び撤去 (γ-2 副次解消)

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
| GameSceneTemplate.h | `GAMESCENETEMPLATE_H_` (1.5.8 で旧 Game4Scene.h `GAMESCENE4_H_` から改名) |
| OptionsScene.h | `OPTIONSSCENE_H_` (1.5.7 で追加) |
| GameStatus.h | `GAMESTATUS_H_` |
| GameSceneMain.h | `GAMESCENEMAIN_H_` |

Game1〜3Scene は **`GAMESCENE<N>_H_`** (数字は GAMESCENE の後)。`GAME<N>SCENE_H_` ではない点に注意 (TwistTimeStopper 流)。雛形 `GameSceneTemplate.h` は固定名の `GAMESCENETEMPLATE_H_` で、新シーン追加時のコピー元として永続化。

### 関数命名

- シーン関数: `initXXXScene` / `moveXXXScene` / `renderXXXScene` / `releaseXXXScene` / `XXXSceneCollideCallback` ([4. シーンアーキテクチャ](#4-シーンアーキテクチャ) 参照)
- 共通ロジック関数 (盤面ステート操作、β-D-5 で導入): `rb*` プレフィックス (`rbInit`, `rbPutPiece`, `rbIsPass`, `rbThinkPlayer`, `rbThinkCpu`, `rbSetMsg`, `rbCheckResult`, `rbCountPieces`, `rbRemovePieces`)。第 1 引数は `ReversiBoard*` ([GameSceneMain.h](Project2/GameSceneMain.h) 定義)
- 共通描画関数 (δ-4 で導入): `rbDraw*` プレフィックス (`rbDrawBoard`, `rbDrawGrid`, `rbDrawPieces`, `rbDrawMsg`, `rbDrawCountPanel`, `rbDrawTurnIndicator`)。Game1Scene/Game2Scene の `renderXxxScene` から呼び出す共通描画ヘルパで、盤面色は引数化。Game3 あまちゃん専用ヒント表示 `rbDrawHints` も同グループ (置けるマスをオレンジ半透明丸でハイライト、後続セッションで追加)
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

Game1Scene / Game2Scene / Game3Scene では `rbThinkPlayer` ([GameSceneMain.cpp](Project2/GameSceneMain.cpp)) 内で `GetMousePoint` を使って盤面クリック判定。`x / CELL_PX` で盤面インデックス化する。δ-2 (2026-06-26) で **範囲外クリックでの UB を解消済**: `mx < 0 || my < 0` の負値ガード (C++ の整数除算はゼロ向き切り捨てで負値が `bx=0` にマップされるため必須) + `bx >= BOARD_SIZE || by >= BOARD_SIZE` の上限ガードで早期 return。右パネル領域や下部メッセージ領域へのクリックは安全に無視される。

### Game3 (あまちゃん、旧 Game5) のキーボード文字入力

[Game3Scene.cpp](Project2/Game3Scene.cpp) で `KeyInputSingleCharString` を使って `nameTmp[12]` に 1 文字ずつ蓄積。`nameTmp[0] != NULL` 検知で `SCENE_GAME2` (まきもどり、旧 SCENE_GAME4) へ遷移する仮実装。`'\0'` または `0` と比較すべきで、`NULL` 比較は `char` と `void*` の暗黙変換警告対象 ([10. 未完成機能の方針](#10-未完成機能の方針) 参照)。

---

## 8. DxLib のお作法 (細かい点)

- **フォント**: `SetFontSize(N)` / `ChangeFontType(DX_FONTTYPE_*)` / `ChangeFont("ＭＳ 明朝")` はグローバル状態なので、各シーンの init で必ず設定し直す ([MenuScene.cpp:20-21](Project2/MenuScene.cpp#L20))。**ReversedReversi では `ChangeFont("ＭＳ 明朝")` が毎フレーム呼ばれているシーンがある** (Game1/Game2=まきもどり の render 内) — 本来 init 時 1 回でよく、TwistTimeStopper では init で完結している
- **色**: `GetColor(R, G, B)` の戻り値 `unsigned int` はキャッシュ。β-D-2 で [GameMain.cpp:20-22](Project2/GameMain.cpp#L20) に `ColorWhite/Red/Sky` を集約定義 + [GameMain.h](Project2/GameMain.h) で `extern` 公開 (旧 `ColorXxx2` 接尾辞は撤廃済)。1.5.6.1 (B3) で **ランク章用 4 色** (`ColorBronze/Silver/Gold/Platinum`) + **汎用 UI 用 3 色** (`ColorWarn/Overlay/Hover`) を追加 (計 10 色)。1.5.9.1 (2026-06-27) でさらに **ランク章用 +6 色** (`ColorIron/Diamond/Emerald/Ruby/Sapphire/Amethyst`) + **汎用 UI 用 +4 色** (`ColorSuccess/Error/Info/Accent`) を追加して **計 20 色** (ランク章 10 / 汎用 UI 7 / 基本 3)。**1.6.0 (2026-06-27) で B2 ランクシステム実装時に伏線回収 — `TIER_COLORS[]` で 10 ランク色を順番に割当 + 進捗バーで ColorSuccess/Overlay + 演出で ColorError/Warn/Accent + ColorGold 強調文字** で実質的に多数が活用された (1.5.9.2 までは伏線として未参照)。新規色を追加する際は同じ `ColorXxx` 命名 + extern 宣言 (GameMain.h) + 定義 (GameMain.cpp) の構造に合わせる。**カラーパレットは継続的拡張対象** — 新色 1 つ追加ごとに SUBPATCH バンプ (1.5.6.1 → 1.5.6.2 → ...)、まとまった色バッチでも SUBPATCH 1 つに集約可 (1.5.6.1 で 7 色 / 1.5.9.1 で 10 色のバッチ追加実績)、機能と同時追加なら機能側 PATCH バンプに巻き込む方針 ([CHANGELOG.md](CHANGELOG.md) 採番ルール参照)
- **座標系**: 原点 (0, 0) は **画面左上**。`DrawString(x, y, str, color)` で直接指定
- **乱数**: `GetRand(n)` で 0〜n-1。シードは起動時に `GetNowCount()` で `SRand` 初期化する慣習 (TwistTimeStopper では `WinMain` 冒頭)
- **画像**: `LoadGraph` / `LoadDivGraph` の戻り値ハンドル `int` を保持し `DeleteGraph` で解放。γ-1 (2026-06-25) で Game1/Game2 を `LoadDivGraph` → init、`DeleteGraph` → release に整理済 (`static int pieces[2] = { -1, -1 };` でハンドル保持、再入場時の二重ロード防止)。リソースリーク解消
- **音**: `PlaySoundFile(path, DX_PLAYTYPE_LOOP / BACK / NORMAL)`。`StopSoundFile()` でループ停止。γ-1 で Game1/Game2 とも `releaseXxxScene` で `StopSoundFile` を呼ぶよう整理済 (メニュー復帰時の BGM クリーンアップ徹底)
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

- **`SCENE_GAME1` (ふつうの次元)** — 12×12 リバーシ本体、プレイヤー (黒、`rbThinkPlayer`) vs CPU (白、`rbThinkCpu`)、勝敗判定まで実装。γ-1 (2026-06-25) でフレーム駆動化 + シーン再入場リセット + X キーメニュー復帰実装済
- **`SCENE_GAME2` (まきもどり次元、旧 SCENE_GAME4)** — Game1 の拡張版、2 ラウンド制 (`CurrentRound`)、ラウンド間で `rbRemovePieces(&state, 96)` で 96 マス削除、BGM 切替 (`changeBGM`)。γ-1 でフレーム駆動化 + ラウンド遷移 240 フレームカウンタ化 + 終了メッセージフリッカー解消 + ラウンド 2 終了時の X キーメニュー復帰 + `releaseGame2Scene` の `DxLib_End` 直呼び撤去 (γ-2 副次解消) 完了
- **`SCENE_GAME3` (あまちゃん次元、旧 SCENE_GAME5)** — γ-3 (2026-06-26) で独自モード化完了。内部 2 フェーズ構造 (`GAME3_PHASE_NAME_ENTRY` → `GAME3_PHASE_PLAYING`)。名前入力は `KeyInputSingleCharString` + Enter キー検出で確定 (元の 1 文字入力即遷移バグ解消)。対局は Game1 と同じ盤面ロジック + 思考テーブル `{ rbThinkPlayer, rbThinkRandom }` で**弱い CPU** (置ける場所からランダム選択) を採用、初心者でも勝てる難易度。対局画面の右パネルに `PLAYER:` として `nameTmp` を表示。後続セッション (2026-06-26) で **ヒント表示** (`rbDrawHints`、置けるマスにオレンジ半透明丸、プレイヤー手番中のみ) + さらに後続 (2026-06-26) で **ヒントマスへの取得コマ数表示** (オレンジ円の中心に白で裏返り枚数を中央寄せ表示、`HINT_GAIN_FONT_SIZE=20`) を追加。1.5.6 (2026-06-26) で **「待った」機能** (R キー、`prevState`/`prevStatus`/`prevTurn`/`undoAvailable` static で前手スナップショット保持、CPU 応手も含めた巻き戻し、FINISHED 中は無効、`undoAvailable && status != FINISHED` 時のみ「R: 待った」ガイド表示) を追加。1.5.7 (2026-06-26) で **オプション設定のトグル化** (`SCENE_OPTIONS` 新設、ヒント / 取得コマ数 / 弱い CPU / 待った の 4 トグルを `g_game3Options` で管理、`settings.ini` 永続化) を追加。1.5.8 (2026-06-27) で **盤面サイズ縮小モード** (6×6 / 8×8 / 10×10 / 12×12 の 4 段階、`ReversiOptions.boardSize` + OPTIONS 5 行目で ←→ 選択 + 右側 320×320 プレビュー、`ReversiBoard.size/cellPx/originX/Y` で動的サイズ対応、Game1/Game2 は 12×12 固定維持) を追加。1.5.9 (2026-06-27) で **画面サイズ拡張** (800×700 → 1280×900、盤面ターゲット 576 → 720px、CELL_PX=60/72/90/120 で全サイズ綺麗に割り切れ、フォントデフォルト 32 → 40、OPTIONS プレビュー 320 → 500、全シーンリレイアウト)
- **B2 プレイヤーランクシステム** (1.6.0、2026-06-27) — **完成**。10 ティア (NOVICE → APPRENTICE → ADEPT → EXPERT → MASTER → GRANDMASTER → SAGE → LEGEND → MYTHIC → ETERNAL) × XP 計算式 (勝=30+コマ差+短手数+完封 / 引分=15 / 負=10、モード倍率 Game1=×1.0/Game2=×1.4/Game3=×0.6 + Game3 オプションペナルティ) × 全モード降格 (降格圧力 5+tier×2 XP、バッファ 50 XP) × 多段ランクアップ演出 240f (フェード→スケール+回転→文字フェード→点滅) + 降格演出 120f。永続化は `settings.ini` に 4 キー (totalXp/currentTier/totalGames/totalWins) 追加、1.5.x 形式と完全後方互換。UI は MenuScene 右上にランク章 + 進捗バー + 次閾値、各 Game シーン右パネル下部に小ランク章、終局時に XP オーバーレイ + ランクアップ/降格演出。OPTIONS 6 行目「ランクをリセット」(Enter 2 連打で確定)

### 未完成 (要対応)

(現状なし — 全 3 モード γ-3 で完成)

### 将来実装予定 (Game3 あまちゃん次元の拡張)

γ-3 + 後続 (2026-06-26) + 1.5.7 + 1.5.8 で「弱い CPU + ヒント表示 + 取得コマ数表示 + 待った機能 + オプション設定 (Game3 専用、`SCENE_OPTIONS` 経由で 4 トグル ON/OFF、`settings.ini` 永続化) + 盤面サイズ縮小モード (6/8/10/12 の 4 段階)」を実装済。

(現状なし — 1.5.8 で予定リスト消化済み)

### 将来予定 (プロジェクト全体の拡張)

Game3 専用ではなく、ふつう/まきもどり/あまちゃんを横断するシステム機能:

1. ~~**プレイヤーランクシステム**~~ — **1.6.0 (2026-06-27) で完了** ([§10「完成済み」](#10-未完成機能の方針) の B2 エントリ参照)。10 ティア + XP + 全モード降格 + 多段アニメ演出 + OPTIONS リセットで実装、設計検討事項 (モード倍率 / Game3 オプションペナルティ / リセット手段) もすべて確定
2. **カラーパレット増設 (継続的)** — 1.5.6.1 (2026-06-26) で初期 7 色 (`ColorBronze`/`Silver`/`Gold`/`Platinum`/`Warn`/`Overlay`/`Hover`) を `ColorXxx` 命名で extern 追加済 ([CHANGELOG.md](CHANGELOG.md) [1.5.6.1] 参照)。**1.5.9.1 (2026-06-27) でランク章 +6 色** (`ColorIron`/`Diamond`/`Emerald`/`Ruby`/`Sapphire`/`Amethyst`) **+ 汎用 UI +4 色** (`ColorSuccess`/`Error`/`Info`/`Accent`) **を追加して計 20 色** (CHANGELOG.md [1.5.9.1] 参照)。ランク章 10 色 (Bronze〜Platinum 4 + Iron/Diamond/Emerald/Ruby/Sapphire/Amethyst 6) で LoL 9 ティア / Overwatch 7 ティア / 将棋 9 段位どれにも余裕で対応可能。本リリースでは未参照、B1/B2/テーマ化等の将来機能で使用される伏線。**継続的タスク** として残置: 今後の機能拡張で新色が必要になり次第、同じ `ColorXxx` 命名で [GameMain.cpp](Project2/GameMain.cpp) 定義 + [GameMain.h](Project2/GameMain.h) extern 追加を継続。1 色追加ごとに SUBPATCH バンプ (1.5.6.1 → 1.5.6.2 → ...)、まとまった色バッチでも SUBPATCH 1 つに集約可 (1.5.6.1 で 7 色 / 1.5.9.1 で 10 色のバッチ追加実績)、機能と同時追加の場合は機能側 PATCH バンプに巻き込み。**未対応**: 既存インライン `GetColor()` 呼び出し (盤面色 `(0,100,20)` / `(0,140,20)`、メッセージ箱グレー `(150,150,150)`、ヒントオレンジ `(255,165,0)`、OPTIONS プレビューの黒/白駒、MenuScene/GameSceneTemplate の白赤リテラル) の extern 化 — 別タスクで一括対応する場合 `BoardColorDark/Light` `MsgBoxBg` `ColorHintOrange` 等の命名統一を要設計検討
3. **描画品質改善 (ガビガビ対策)** — 1.5.9 で盤面 720×720 / コマ 60〜120px に拡張後、ユーザー報告で **盤面グリッド線とコマ円輪郭のジャギー** が顕在化 (2026-06-27)。原因は 2 系統: **(a) `DrawLine` の AA 無し**: 1px 細線をビットマップ直書きしているため斜め成分のないグリッド線でも端部がジャギー、**(b) `DrawExtendGraph` のニアレストネイバー拡大**: piece.png 47×47 ソースを 60/72/90/120px に約 1.28〜2.55× 拡大、デフォルトは `DX_DRAWMODE_NEAREST` のため拡大率が大きいほどジャギー悪化。**対策候補** (どれか単独 or 複合): **(1) `SetDrawMode(DX_DRAWMODE_BILINEAR)` を `rbDrawPieces` 内で局所適用**: バイリニア補間で輪郭平滑化、関数末尾で `DX_DRAWMODE_NEAREST` 復元 (副作用最小、実装コスト最低)、**(2) piece.png 高解像度差し替え**: 120×120 ソースを用意し最大セル時に等倍、他サイズで縮小描画 (縮小はジャギー出にくい、画質改善効果最大、画像差し替えコスト + `DX_DRAWMODE_BILINEAR` 併用が定石)、**(3) グリッド線の太線化 or 盤面背景テクスチャ化**: `DrawLine` を 2px に重ね描き or 盤面 + グリッドを 1 枚のテクスチャに統合して `DrawGraph` で描画、テクスチャ系統は `DX_DRAWMODE_BILINEAR` で平滑化可能。**推奨は (1) 単独着手**: 即効性高くロールバック容易、効果不足なら (2) を追加検討。版数は SUBPATCH (1.5.9.1) または挙動変更含めて機能 PATCH バンプに巻き込み
4. **対局途中での中断機能 (give up / メニュー復帰)** — 1.6.0 リリース後のユーザー指摘で発覚 (2026-06-27): 現状は対局中 (GAME_STATUS_PLAYING/TURN_MSG/PASS_MSG) に X キーが無効、ESC はプロセス終了 (WinMain ループ脱出) のため、**一度対局を始めたら FINISHED まで戻れない**。Game3 の「待った」 (R キー) は 1 手戻し専用で、メニュー復帰には使えない。**仕様要件**: (1) 対局中に Esc 以外の専用キー (例: Q キー / Ctrl+X / Backspace) で「中断」操作を提供、(2) 誤操作防止のため確認プロンプト 2 段階 (1 回目 押下で「中断しますか? もう一度 Q で確定」を ColorWarn 表示、2 回目で `changeScene(SCENE_MENU)`、180f タイマー切れまたは別キーで解除)、(3) **中断時は XP 計上しない** (ユーザー指示)、totalGames/totalWins も加算しない (対局未完了として扱う)、(4) Game2 ラウンド 1 → ラウンド 2 遷移中の中断も同様に XP なし、(5) Game3 の「待った」スナップショット (prevState 等) と Game1/2 の盤面状態を中断時に破棄 (release で `state = {}` + フラグリセット)。**実装規模**: 小〜中、各 Game シーン (Game1/2/3) の `moveXxxScene` に確認タイマー (`abortConfirmTimer`) + Q キー判定 + 確認文言の renderXxxScene 描画追加で 30〜40 行 × 3 シーン = 90〜120 行。各シーン共通ヘルパ `bool tryAbortMidGame(int* abortConfirmTimer, GAME_STATUS status)` を [GameSceneMain.cpp](Project2/GameSceneMain.cpp) に切り出すと重複削減可。**UI 配置**: 中断確認文言は対局画面の上半分中央あたり (盤面と被るが半透明背景で視認性確保、`rbDrawResultOverlay` と同方式)、または右パネルの空き Y=420〜480 領域に小型表示で「Q: 中断」ガイドを常時 + 押下後に確認文言オーバーレイ。**版数**: 機能追加で挙動変化を伴うため PATCH バンプ 1.6.0 → 1.6.1 想定
5. **ランクアップ演出の粗修正** — 1.6.0 リリース後のユーザー指摘で発覚 (2026-06-27)、2 系統の問題: **(a) BGM の再開始**: [Game1Scene.cpp](Project2/Game1Scene.cpp) / [Game2Scene.cpp](Project2/Game2Scene.cpp) / [Game3Scene.cpp](Project2/Game3Scene.cpp) でランクアップ判定時に `PlaySoundFile("res/loop_68.wav", DX_PLAYTYPE_BACK)` を呼んでいるが、これによって既存の LOOP BGM (Game1/Game3=loop_95.wav / Game2=loop_68.wav) が中断され、ランクアップ演出 240f 終了後に **BGM が冒頭から再開** される現象。原因は DxLib `PlaySoundFile` が同一ファイル / 同一チャンネルに対する重複呼出時の挙動 (再生位置リセット + LOOP 状態破棄) と推測。**対策候補**: (1) `LoadSoundMem` + `PlaySoundMem` で SE 専用ハンドルを事前ロード (BGM とは別チャンネル想定)、(2) ランクアップ用に専用 wav 追加 (res/rankup.wav 等、既存 BGM と全く別ファイル) で重複回避、(3) BGM を一時停止 → SE 再生 → BGM 再開 (現在の再生位置を `GetSoundCurrentTime` で記録 → `SetSoundCurrentTime` で復帰)。**推奨は (1)**: リソース追加不要 + 副作用最小。**(b) ランク章の揺れ動き**: [GameSceneMain.cpp:rbDrawRankUpAnimation](Project2/GameSceneMain.cpp) で「回転感」演出のために `sinf(angle)*8 / cosf(angle)*8` のオフセットを円中心 (640, 384) に加算しているが、円は回転対称なので **見た目には中心が半径 8 の小円軌道を描く揺れ** に見える失敗演出。**対策**: (1) オフセットを完全撤去 (中心固定でスケールのみで十分、シンプル路線)、(2) 円の代わりに **星型 / 多角形 / リボン形** など回転対称でない形状に変更 (回転が視覚的に意味を持つ)、(3) 円の周囲に小さな星 / きらめきを多数配置して回転 (中心は動かさず周辺要素のみ回転)。**推奨は (1) 単独着手**: 即効性高くコード削減、効果不足なら (3) を追加検討。**実装規模**: (a) 約 +20 行 / (b) 1〜2 行削除 = 合計 約 +20 行。**版数**: 4 (中断機能) と同時実装で PATCH バンプ 1.6.0 → 1.6.1 に巻き込み推奨、または単独で SUBPATCH 1.6.0.1
6. **OPTIONS シーンのレイアウト改善** — 1.6.0 リリース後のユーザー指摘 (2026-06-27)、2 系統の問題: **(a) 6 行目「ランクをリセット [TIER N XP]」がプレビュー領域と被る**: ティア名 + XP 値の幅が動的に変化し、最悪ケース (例「GRANDMASTER 99999 XP」など) でプレビュー領域 (x=810 開始) に食い込む。スクリーンショットで「APPRENTICE 84 XP」(中程度) で既に右端 "XP]" がプレビューに被って判読困難。**根本対策 (ユーザー提案)**: **盤面サイズ + プレビューを別画面 (新規 `SCENE_BOARD_SIZE`) に分離**。OPTIONS は ON/OFF トグル系 (ヒント / 取得数 / 弱 CPU / 待った / ランクリセット) のみ、新シーンに盤面サイズ ←→ 選択 + 320×320 プレビュー描画を移動。これで OPTIONS の項目列が画面左半分 (x=130..600 程度) に集約され、プレビュー領域 (x=810+) との衝突が消滅。**実装手順**: [GameSceneTemplate.cpp](Project2/GameSceneTemplate.cpp) をコピー → `BoardSizeScene` (仮称) として識別子置換 → `renderBoardPreview` ([OptionsScene.cpp](Project2/OptionsScene.cpp) static 関数) を新シーンに移動 → SCENE_NO enum 拡張 + sceneTable[] 拡張 + vcxproj 登録 (CLAUDE.md §4 5 ステップ手順)。OPTIONS の 5 行目「盤面サイズ」は「盤面サイズ設定 →」のような遷移ボタンに変更、Enter で `changeScene(SCENE_BOARD_SIZE)`。新シーンから X キーで OPTIONS に戻る (要 prevScene 復帰経路、または直接 SCENE_OPTIONS にハードコード)。**実装規模**: 新シーン 約 +150 行 + OPTIONS 改修 約 +20 行 = 約 +170 行。**(b) ON/OFF 表示のインデント不揃い**: 現実装は `labels[]` に空白パディング (例 `"ヒント表示       "`) を埋め込んで `DrawFormatString("%s%s", label, "[ ON ]")` で連結描画しているが、DxLib + MS 明朝のフォントで日本語全角文字の幅と ASCII 半角空白の幅の比率が一定でないため、ラベル長が違う行で ON/OFF の左端 X 位置が揃わない。**対策候補**: (1) **ラベル描画と ON/OFF 描画を分離 + ON/OFF は固定 X で描画**: `DrawString(130, y, label, color)` で日本語部のみ描画 → `DrawString(500, y, "[ ON ]", color)` で全行同一 X に固定 (推奨、シンプル + 確実)、(2) ラベル描画後に `GetDrawStringWidth(label)` で実描画幅を測って ON/OFF を相対位置で描画 (動的だが計算コスト)、(3) 等幅フォントに切替 (他画面と整合性崩れる、不採用)。**推奨は (1)**: 約 +10 行で確実に整列。ユーザー言及の「オプション名と ON/OFF 表示は分けたほうがいい」と一致。**実装規模**: (b) 約 +10 行。**版数**: (a) (b) 同時実装で PATCH バンプ 1.6.0 → 1.6.1 に巻き込み、項目 4/5 と一緒のリリースが効率的

### 将来予定 (リポジトリ整備)

ゲーム機能とは別系統のリポジトリ/ドキュメント整備タスクは **全完了**:

- **CHANGELOG.md** 分割 → 1.5.5 で完了 ([CHANGELOG.md](CHANGELOG.md))
- **LICENSE.md** (MIT) → 1.5.6 で公開 ([LICENSE.md](LICENSE.md))、第三者ライブラリ (DxLib) ライセンス遵守注記を末尾セクションに併記 (作成自体は LICENSE/README コミット時、本体未変更だったため一時的に `[Unreleased]` に記録 → 1.5.6 で待った機能と同梱公開)
- **README.md** (GitHub 公開用、JA 上 / EN 下) → 1.5.6 で公開 ([README.md](README.md))、テキストのみ (スクリーンショットは将来追加余地、同上)

追加候補 (任意): 将来 README にスクリーンショットを追加する場合は `docs/screenshots/` ディレクトリを設置、メニュー画面 + Game3 ヒント表示画面の 2 枚程度が想定。

### 空シーン雛形

- **`SCENE_TEMPLATE`** ([GameSceneTemplate.cpp](Project2/GameSceneTemplate.cpp)、1.5.8 で旧 `SCENE_GAME4` / `Game4Scene.cpp` から固定名に改名) — α-7 で旧 SCENE_GAME2 (死蔵スタブ) を転用した **空シーン雛形**。`menu[]` 非掲載で到達不能だが、新シーン追加時の永続コピー元として保持。「Template Scene」の描画と「ボタン１でタイトルに戻る」遷移のみ実装、新シーン作成時は **GameSceneTemplate.cpp/.h をコピーして識別子置換 + enum/sceneTable に 1 行追加** で起動する (4. シーンアーキテクチャ参照)。雛形ファイル自体は残置することで、シーン番号スライドによる名前陳腐化を回避する設計 (ユーザー指示)。

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
1. ✅ **γ-1: Game1/Game2 のフレーム駆動化** (完了, 2026-06-25) — `while (!ProcessMessage())` 独自ループ撤去、状態 (`status`/`turn`/`CurrentRound`/`excuted`/`roundTransitWait`/`finishedMsgRand`/`ReversiBoard state`/`pieces[]`) をファイルスコープ `static` 化、`initGame1Scene`/`initGame2Scene` でシーン再入場時の毎回リセット + リソース読込、`renderGame1Scene`/`renderGame2Scene` に描画分離、終了状態で X キー → `changeScene(SCENE_MENU)`。Game2 ラウンド遷移は 240 フレームカウンタ、終了メッセージ抽選は 1 回固定 (フリッカー解消)
2. ✅ **γ-2: Game2 ラウンド 2 終了処理の正規化** (γ-1 副次解消, 2026-06-25) — `releaseGame2Scene` 内の `DxLib_End` 直呼び撤去、move 内の `releaseGame2Scene()` 直呼びも撤去、X キー → `changeScene(SCENE_MENU)` で正常復帰する形に統一
3. ✅ **γ-3: Game3 (あまちゃん次元) の方針確定と実装** (完了, 2026-06-26) — 方針 2 (独自モード化) 採用。Game3Scene を 2 フェーズ構造 (NAME_ENTRY → PLAYING) に再構築、`rbThinkRandom` (置ける場所からランダム選択) を新規追加して弱い CPU を実装。1 文字入力即遷移バグ解消 (Enter キー検出に変更)。`nameTmp` を対局画面の右パネルに `PLAYER:` 表示。ヒント表示・「待った」機能は将来追加候補 ([10. 未完成機能の方針](#10-未完成機能の方針) §将来実装予定)

### フェーズ δ: 仕上げ

候補:
1. ✅ **δ-1: ResourceLeak の解消** (γ-1 で前倒し完了, 2026-06-25) — `LoadDivGraph` を `initGame1/2Scene`、`DeleteGraph` を `releaseGame1/2Scene` に移動済 (`static int pieces[2] = { -1, -1 };` で再入場時の二重ロード防止、release で `StopSoundFile` も対)
2. ✅ **δ-2: 入力安全化** (完了, 2026-06-26) — `rbThinkPlayer` ([GameSceneMain.cpp](Project2/GameSceneMain.cpp)) に範囲ガード追加。`mx < 0 || my < 0` (負値ガード、ゼロ向き切り捨てで負値が `bx=0` にマップされる問題を捕捉) + `bx >= BOARD_SIZE || by >= BOARD_SIZE` (上限ガード) で盤面外クリックを早期 return。`state->board[by][bx]` の配列範囲外アクセス UB を解消
3. ✅ **δ-3: x64 構成追加とプラットフォーム移植性向上** (完了, 2026-06-26) — [Common.props](Project2/Common.props) を新設して DxLib パス (`<DxLibDir>C:\DxLib</DxLibDir>`) を 1 箇所に集約、4 構成 (Debug/Release × Win32/x64) 全てから Import。Project2.sln + Project2.vcxproj に Debug|x64 / Release|x64 構成追加。x64 ビルドで C4267 (`size_t→int` 暗黙縮小) が 1 件発生 (`rbDrawMsg` 内 `state->msg.size()`) → 明示キャスト `(int)state->msg.size()` で解消。全 4 構成警告 0 / エラー 0 で成功
4. ✅ **δ-4: 描画 6 ブロックの共通関数化** (完了, 2026-06-26) — `rbDrawBoard(boardBgColor)` / `rbDrawGrid()` / `rbDrawPieces(state, pieces)` / `rbDrawMsg(state, status)` / `rbDrawCountPanel(state)` / `rbDrawTurnIndicator(turn)` を [GameSceneMain.cpp](Project2/GameSceneMain.cpp) に追加。Game1Scene::renderGame1Scene 100→17 行、Game2Scene::renderGame2Scene 130→39 行に圧縮。盤面色 (Game1=暗緑/Game2=明緑) は引数化、動作不変

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

### フェーズ γ-1: Game1/Game2 フレーム駆動化 (完了, 2026-06-25)

本プロジェクト最大の構造的不具合だった `while (!ProcessMessage())` 独自ループを撤去し、TwistTimeStopper 流のフレーム駆動モデルに書き直し。γ-2 (Game2 終了処理正規化) + δ-1 (リソースリーク解消) も副次的に同時達成。

- **状態の `static` ファイルスコープ化**:
  - Game1Scene: `state` / `status` / `turn` / `pieces[2]`
  - Game2Scene: 上記 + `CurrentRound` / `excuted` / `roundTransitWait` / `finishedMsgRand`
- **`initGame1Scene` / `initGame2Scene` の責務拡張** (シーン入場/再入場時に毎回呼ばれる):
  - 状態リセット (再入場バグ解消)
  - `rbInit(&state)` で盤面ゼロクリア + 初期 4 駒配置
  - `ChangeFont` フォント設定 (move/render での冗長呼び出し撤去)
  - `LoadDivGraph` リソース読込 (`pieces[2]` ハンドル取得)
  - `PlaySoundFile` BGM 再生開始
- **`moveGame1Scene` / `moveGame2Scene` の縮小**: `switch (status)` の 1 ティック分のみ。SetDrawScreen / ClearDrawScreen / ScreenFlip / 内側 ESC 検出はメインループ側に移譲
- **`renderGame1Scene` / `renderGame2Scene` の新設**: 旧 move 内の描画 6 ブロック (盤面背景/格子/コマ/メッセージ/カウントパネル/ターンインジケータ/終了メッセージ) を丸ごと移動。共通関数化は次セッション候補
- **`releaseGame1Scene` / `releaseGame2Scene` の責務拡張**:
  - `StopSoundFile` で BGM 停止 (メニュー復帰時に BGM が鳴り続ける問題解消)
  - `DeleteGraph(pieces[0/1])` で画像ハンドル解放 (リーク解消、`pieces[0] != -1` ガード付きで再入場二重解放防止)
  - Game2 の `DxLib_End` 直呼び撤去 (γ-2 副次解消)
- **メニュー復帰経路の追加**: 終了状態 (`GAME_STATUS_FINISHED`) で X キー押下 → `changeScene(SCENE_MENU)`。終了メッセージ表示文言を「Xキーでメニュー\nESCキーで終了」に更新
- **Game1 終了時 `WaitTimer(2000)` 撤去**: フレームハングバグ解消
- **Game2 ラウンド遷移の 240 フレームカウンタ化**: `roundTransitWait` で 240 フレーム (4 秒) 経過後に `rbRemovePieces(&state, 96)` → `CurrentRound = SECOND` → `GAME_STATUS_TURN_MSG`。旧 `WaitTimer(4000)` 相当の挙動を維持しつつフレーム駆動化
- **Game2 ラウンド 1 終了メッセージのフリッカー解消**: 初回フレームで `GetRand(2)` を 1 回だけ抽選し `finishedMsgRand` に保存、render で参照
- **Game2 ラウンド 2 BGM 切替**: move 末尾で `CurrentRound == SECOND && excuted == 0` 時に `changeBGM()` を 1 回だけ呼ぶ
- **検証**:
  - 中間ビルド 2 回 (Game1Scene 書換後 / Game2Scene 書換後) + 最終 Debug + Release リビルド: いずれも警告 0 / エラー 0
  - grep: `while (!ProcessMessage())` / `WaitTimer` / `DxLib_End` が Game1Scene/Game2Scene から消滅、`DeleteGraph` が両ファイルにヒット (リーク解消確認)
- **動作差分**: ゲームロジック (盤面操作・思考・パス判定・勝敗判定) は不変。挙動変更はバグ修正 (再入場リセット / 終了メッセージフリッカー / `WaitTimer` ハング / BGM 鳴り続け / リソースリーク) と機能追加 (X キー復帰) のみ

### フェーズ δ-4: 描画 6 ブロックの共通関数化 (完了, 2026-06-26)

γ-1 で `renderGame1Scene` / `renderGame2Scene` に分離した描画ブロックが Game1/Game2 でほぼコードクローン (盤面色のみ差) になっていたのを `rbDraw*` 関数群に集約。

- **6 関数を [GameSceneMain.h](Project2/GameSceneMain.h) / [GameSceneMain.cpp](Project2/GameSceneMain.cpp) に追加**: `rbDrawBoard(int boardBgColor)` / `rbDrawGrid(void)` / `rbDrawPieces(ReversiBoard*, int pieces[2])` / `rbDrawMsg(ReversiBoard*, int status)` / `rbDrawCountPanel(ReversiBoard*)` / `rbDrawTurnIndicator(int turn)`
- **Game1Scene::renderGame1Scene 圧縮**: 100 行 → 17 行 (6 ヘルパ呼び出し + Game1 専用の終了メッセージのみ)
- **Game2Scene::renderGame2Scene 圧縮**: 130 行 → 39 行 (6 ヘルパ呼び出し + Game2 専用の Round 表示と終了メッセージのみ)
- **`<stdlib.h>` include 削除** (Game1Scene/Game2Scene): `_itoa_s` の参照が `rbDrawCountPanel` 経由になったため、シーンファイルから `_itoa_s` 直接参照消滅 → include 不要
- **動作不変**: 盤面色 (Game1=暗緑/Game2=明緑) は引数化、コマ描画/メッセージ/カウントパネル/ターンインジケータの座標・色・条件分岐すべて等価
- **コード等価性の微改善**: `rbDrawCountPanel` 内の優勢色選択を三項演算子 `(winning == N) ? ColorRed : ColorWhite` に簡略化、`rbDrawMsg` の status 判定を定数 `GAME_STATUS_PLAYING` 経由に
- **検証**: 中間ビルド 2 回 (GameSceneMain 拡張後 / render 圧縮後) + 最終 Debug + Release リビルド: いずれも警告 0 / エラー 0

### フェーズ γ-3: Game3 (あまちゃん次元) 独自モード化 (完了, 2026-06-26)

ユーザー判断で方針 2 (独自モード化) 採用、スコープは「弱い CPU 実装」に絞り (ヒント表示・「待った」機能は将来追加)、Game3Scene を全面再構築。

- **新関数 `rbThinkRandom` を [GameSceneMain.cpp](Project2/GameSceneMain.cpp) に追加**: 置ける場所を全マス走査 → 候補リストに集めて `GetRand(candCount - 1)` でランダム選択。Game1/Game2 の `rbThinkCpu` (貪欲: 最多取得) より明確に弱く、初心者でも勝てる難易度
- **Game3Scene 内部 2 フェーズ構造**: `typedef enum _GAME3_PHASE { GAME3_PHASE_NAME_ENTRY, GAME3_PHASE_PLAYING }` をファイル内に定義、`static GAME3_PHASE phase` で管理。`initGame3Scene` で `GAME3_PHASE_NAME_ENTRY` にリセット
- **名前入力フェーズ**: `KeyInputSingleCharString` は `renderGame3Scene` で毎フレーム呼ぶ (描画と入力受付を兼ねるため)、`moveGame3Scene` で Enter キー検出 + `nameTmp[0] != '\0'` 判定で `GAME3_PHASE_PLAYING` に遷移。**元コードの 1 文字入力即座遷移バグ解消** (旧 `if (nameTmp[0] != NULL) changeScene(SCENE_GAME2)` → Enter キー待ちに変更)
- **対局フェーズ**: Game1Scene と同じ盤面進行ロジック (`switch (status)` で `PLAYING`/`TURN_MSG`/`PASS_MSG`/`FINISHED` 遷移)。思考テーブルだけ `{ rbThinkPlayer, rbThinkRandom }` に差し替え。盤面色は Game1 と同じ暗緑 `(0, 100, 20)`
- **Game3 専用 UI**: 対局画面の右パネル (`PANEL_ROUND_LABEL_Y` 位置) に `PLAYER:` ラベル + `nameTmp` (`ColorSky` で強調)。Game2 の Round 表示位置を再利用
- **メニュー復帰**: 名前入力フェーズで X キー押下 → `changeScene(SCENE_MENU)`、対局終了 (`GAME_STATUS_FINISHED`) で X キー → `changeScene(SCENE_MENU)`。Game1/Game2 と同じ TwistTimeStopper 流
- **リソース管理 (γ-1 と同じ)**: `LoadDivGraph` → `initGame3Scene`、`DeleteGraph` → `releaseGame3Scene`、`PlaySoundFile` → init、`StopSoundFile` → release。BGM は `loop_95.wav` (Game1 と同じ)
- **行数差分**: Game3Scene.cpp 65 → 168 行 (+103)、GameSceneMain.h +3、GameSceneMain.cpp +22。1 文字入力即遷移バグ修正と独自対局ロジック追加で純増
- **検証**: 中間ビルド 2 回 (rbThinkRandom 追加後 / Game3Scene 書換後) + 最終 Debug + Release リビルド: いずれも警告 0 / エラー 0

### フェーズ δ-2: マウス入力安全化 (完了, 2026-06-26)

`rbThinkPlayer` ([GameSceneMain.cpp](Project2/GameSceneMain.cpp)) の盤面外クリック時の配列範囲外アクセス UB を解消。

- **問題**: 旧実装は `GetMousePoint(&mx, &my)` 後に `rbPutPiece(state, mx / CELL_PX, my / CELL_PX, ...)` で直接渡していた。盤面領域は (5, 5)–(580, 580) だが、画面は 800×700 のため右パネル領域 (`mx >= 576`) や下部メッセージ領域 (`my >= 576`) クリックで `bx >= 12 = BOARD_SIZE` となり `state->board[by][bx]` が配列範囲外アクセス
- **修正**: 範囲ガードを 2 段で追加
  1. **負値ガード**: `if (mx < 0 || my < 0) return false;` — C++ の整数除算はゼロ向き切り捨て (`-10 / 48 = 0`) のため、負値クリックは `bx=0` にマップされて単純な上限チェックでは捕捉できない。前段で必須
  2. **上限ガード**: `if (bx >= BOARD_SIZE || by >= BOARD_SIZE) return false;` — 盤面右下を超える領域 (右パネル / 下部メッセージ / ESC キーガイド) クリックを安全に無視
- **動作差分**: 盤面範囲内のクリックは従来通り (動作不変)。盤面外クリックは「無視」になる (UB は解消、ユーザーから見ても「クリックしても何も起きない」のは妥当な挙動)
- **検証**: Debug + Release リビルド、警告 0 / エラー 0

### フェーズ δ-3: x64 構成追加 + Common.props 化 (完了, 2026-06-26)

DxLib パスを `.props` に集約 + x64 ターゲット追加で 4 構成対応。

- **新規ファイル**: [Project2/Common.props](Project2/Common.props) — `<DxLibDir>C:\DxLib</DxLibDir>` UserMacro と `ItemDefinitionGroup` で `AdditionalIncludeDirectories` / `AdditionalLibraryDirectories` を一元管理。`<BuildMacro>` 宣言で Visual Studio のマクロ補完にも認識される
- **[Project2.sln](Project2.sln) 更新**: `SolutionConfigurationPlatforms` に `Debug|x64` / `Release|x64` 追加、`ProjectConfigurationPlatforms` に 4 件 (ActiveCfg + Build.0 × 2) 追加
- **[Project2.vcxproj](Project2/Project2.vcxproj) 更新**:
  - `ProjectConfigurations` に Debug|x64 / Release|x64 を追加
  - `PropertyGroup Label="Configuration"` を 2 → 4 構成分に (x64 用の `PlatformToolset v145` 等を追加、Win32 と同設定)
  - `ImportGroup Label="PropertySheets"` を 2 → 4 構成分に、全構成で `<Import Project="Common.props" />` を追記
  - `ItemDefinitionGroup` を 2 → 4 構成分に複製、**`AdditionalIncludeDirectories` / `AdditionalLibraryDirectories = C:\DxLib;...` の直接指定を撤去** (Common.props に集約済)
  - 旧 `<LibraryPath>$(LibraryPath)</LibraryPath>` 死蔵 PropertyGroup も削除
- **C4267 警告解消** ([GameSceneMain.cpp `rbDrawMsg`](Project2/GameSceneMain.cpp)): x64 では `std::string::size()` の戻り型 `size_t` が 64-bit、`GetDrawStringWidth` 第 2 引数は `int` のため C4267 (`size_t→int` 暗黙縮小) が発生。メッセージは数文字なので `(int)state->msg.size()` 明示キャストで安全に解消
- **DxLib リンクは自動**: [DxDataTypeWin.h](file:///C:/DxLib/DxDataTypeWin.h) が `_MSC_VER >= 1900` + `_WIN64` + `_MT` を判定して `#pragma comment(lib, "DxLib_vs2015_x64_MT.lib")` 等を発行するため、`Common.props` の `AdditionalLibraryDirectories` 指定だけで Win32/x64 両方が解決される
- **検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0。出力サイズ: Debug|Win32 約 11.7 MB / Release|Win32 約 6.1 MB / Debug|x64 約 13.6 MB / Release|x64 約 6.9 MB

### Game3 ヒント表示追加 (完了, 2026-06-26)

§10 将来予定の「ヒント表示」を実装。Game3 (あまちゃん) 専用の初心者向けヒント機能。

- **新関数 `rbDrawHints` を [GameSceneMain.cpp](Project2/GameSceneMain.cpp) に追加**: 全マスを `rbPutPiece(..., put_flag=false)` でシミュレーションして置けるマスを判定、`SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128)` (50% 半透明) に切替えてマス中央に `DrawCircle` でオレンジ (`GetColor(255, 165, 0)`、関数冒頭で 1 度キャッシュ) 塗り潰し丸 (半径 = `CELL_PX / 3`) を描画。最後に `SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0)` で元に戻す (以降の描画に影響しないように必須)。色選定: 既存 `ColorSky` 流用案から再検討し、黒コマ/白コマ/暗緑盤面のいずれとも混同しにくく既存 `← Now` 矢印 (`ColorSky`) と区別される暖色を選択
- **Game3 renderGame3Scene PLAYING フェーズで条件付き呼び出し**: `status == GAME_STATUS_PLAYING && turn == GAME_TURN_BLACK` 時のみ。TURN_MSG/PASS_MSG/FINISHED 中や CPU 手番中は混乱を招くため非表示
- **Game1/Game2 では非表示**: ヒント表示は Game3 (あまちゃん) の差別化要素。Game1 (ふつう) / Game2 (まきもどり) は従来通りヒントなしで難易度を維持
- **検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### B1 オプション設定のトグル化 (完了, 2026-06-26, 1.5.7)

§10 将来予定 (Game3 拡張) の「オプション設定のトグル化」を実装。Game3 (あまちゃん) 専用の 4 トグル UI + settings.ini 永続化。プランモードでユーザー承認を得てから着手。

- **新規シーン `SCENE_OPTIONS`** ([Project2/GameSceneMain.h](Project2/GameSceneMain.h) enum 追加、[GameSceneMain.cpp](Project2/GameSceneMain.cpp) sceneTable[] に 1 行追加。新規シーン追加手順は β-D-1 で確立した「(1) enum + (2) sceneTable」のみで完結)
- **新規ファイル** ([Project2/OptionsScene.h](Project2/OptionsScene.h) / [OptionsScene.cpp](Project2/OptionsScene.cpp)) — 5 関数 (init/move/render/release/CollideCallback)。`OPTION_COUNT = 4`、`static int selected` でカーソル管理、↑↓ で選択、Enter/Space でトグル反転 + `saveOptions()` 即呼出、X でメニュー復帰
- **新規構造体 `ReversiOptions`** ([GameSceneMain.h](Project2/GameSceneMain.h)) — `bool showHints/showGain/weakCpu/allowUndo` の 4 メンバ。デフォルトは全 true で現状の Game3 動作と完全一致
- **グローバル `g_game3Options`** ([GameMain.cpp](Project2/GameMain.cpp) 定義 + [GameSceneMain.h](Project2/GameSceneMain.h) extern) — `ReversiOptions g_game3Options = { true, true, true, true };`
- **永続化** ([GameMain.cpp](Project2/GameMain.cpp)): `loadOptions()` / `saveOptions()` 関数追加。`fopen_s` + `sscanf_s` でセキュア、`settings.ini` (key=value テキスト形式) を起動時に読込、トグル変更ごとに書込。ファイル無し/不正値はデフォルト維持で安全
- **WinMain で `loadOptions()` を呼出** — `InitGame()` 直後の 1 回のみ ([GameMain.cpp](Project2/GameMain.cpp))
- **`rbDrawHints` シグネチャ拡張** ([GameSceneMain.h/.cpp](Project2/GameSceneMain.cpp)): `void rbDrawHints(ReversiBoard*, int turn, bool showGain)` に変更。Pass 2 (取得コマ数の数字描画) を `if (showGain)` でガード、フォントサイズ退避/復元も条件付き
- **Game3Scene 内 4 箇所のトグル参照** ([Game3Scene.cpp](Project2/Game3Scene.cpp)):
  - R キー判定: `g_game3Options.allowUndo && ...` (FINISHED ガードと AND)
  - 思考テーブル: `{ rbThinkPlayer, g_game3Options.weakCpu ? rbThinkRandom : rbThinkCpu }`
  - スナップショット persist: `if (g_game3Options.allowUndo && isPlayerTurn) ...`
  - ヒント描画呼出: `if (g_game3Options.showHints && ...) rbDrawHints(&state, turn, g_game3Options.showGain);`
  - 「R: 待った」ガイド: `g_game3Options.allowUndo && undoAvailable && status != FINISHED`
- **MenuScene 拡張** ([MenuScene.cpp](Project2/MenuScene.cpp)): `MENU_MAX` 3 → 4、`menu[]` / `menuList[]` に `SCENE_OPTIONS` / "オプション設定" 追加
- **Project2.vcxproj / .filters 更新**: `OptionsScene.cpp` / `OptionsScene.h` を ClCompile/ClInclude に追加、フィルタは「ソース ファイル\Scene」「ヘッダー ファイル\Scene」配下
- **スコープは Game3 のみ**: Game1 (ふつう) / Game2 (まきもどり) は据え置き、Game3 専用 `g_game3Options` を参照しない
- **デフォルト全 ON は現状の Game3 動作と一致**: settings.ini を作らず起動しても挙動同じ
- **`showGain && !showHints`**: UI 上は独立トグルだが描画コードで showHints が外側ガードなので showGain も実質無効 (将来 OPTIONS UI で依存関係を明示する余地あり)
- **版数**: 機能追加 (挙動変化あり) なので PATCH バンプ 1.5.6.1 → 1.5.7 ([GameMain.cpp:53](Project2/GameMain.cpp#L53) タイトル + [MenuScene.cpp:66](Project2/MenuScene.cpp#L66) メニュー)
- **検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### B3 カラーパレット増設 初期 7 色 (完了, 2026-06-26, 1.5.6.1)

§10 将来予定 (プロジェクト全体の拡張) 旧候補 2「カラーパレット増設」の初期分を実装。プランモードでユーザー承認を得てから着手 (待った機能と同じ手順)。**継続的タスク**として位置付け、本リリースは初期 7 色追加のみ。

- **新規 7 色を [GameMain.cpp](Project2/GameMain.cpp) 定義 + [GameMain.h](Project2/GameMain.h) extern 追加** (既存 `ColorWhite`/`Red`/`Sky` パターンに沿う):
  - **ランク章用** (B2 ランクシステム伏線): `ColorBronze` (205,127,50) / `ColorSilver` (192,192,192) / `ColorGold` (255,215,0) / `ColorPlatinum` (229,228,226)。CSS / Material Design / メダル色配色準拠、プラチナは「白に近い淡銀」で銀と差別化
  - **汎用 UI 用** (B1 オプショントグル + テーマ化伏線): `ColorWarn` (255,235,0、純黄より少しオレンジ寄りで視認性高) / `ColorOverlay` (128,128,128、半透明オーバーレイ用中間グレー、`DX_BLENDMODE_ALPHA` 128 想定) / `ColorHover` (180,220,255、ColorSky より淡い水色でホバー強調)
- **本リリースでは未参照** — 新規 7 色はいかなる描画コードからも使われていない。B1/B2/テーマ化等の将来機能で使用される伏線として配置のみ
- **既存インライン `GetColor()` 呼び出しの extern 化は別タスクに** (スコープ外): 盤面色 (Game1=暗緑 / Game2=明緑) / メッセージ箱グレー / ヒントオレンジ / Menu/Game4 の白赤リテラルは現状維持
- **新規 4 セグメント版数ルール `MAJOR.MINOR.PATCH.SUBPATCH` を導入**: コードのみ変更で挙動不変な場合 (本リリースのような未使用 extern 追加、内部リファクタ) は PATCH ではなく SUBPATCH を +1。CHANGELOG 冒頭の採番ルールに明記
- **版数**: 挙動不変だがコード変更なので 1.5.6 → 1.5.6.1 にバンプ ([GameMain.cpp:41](Project2/GameMain.cpp#L41) タイトル + [MenuScene.cpp:66](Project2/MenuScene.cpp#L66) メニュー)
- **継続性ノート**: §10 で「カラーパレット増設」を完了マークではなく **継続的タスク** として残置。今後の機能拡張で新色が必要になり次第追加し、SUBPATCH または機能側 PATCH に巻き込む方針を §8 / §10 / CHANGELOG に明文化
- **検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### Game3「待った」機能追加 (完了, 2026-06-26, 1.5.6)

§10 将来予定 (旧候補 1) の「待った」機能を実装。Game3 (あまちゃん) 専用のあまちゃん向けストレス軽減機能。プランモードでユーザー承認を得てから着手 (前回はプラン提示を飛ばして実装してリバートになった反省を踏まえる)。

- **新規 file-scope static** ([Game3Scene.cpp](Project2/Game3Scene.cpp)): `prevState` (ReversiBoard) / `prevStatus` (GAME_STATUS) / `prevTurn` (GAME_TURN) / `undoAvailable` (bool)
- **`initGame3Scene` でリセット**: 4 つすべてゼロ/初期値に戻す (再入場時の前回値残留防止、ゲーム開始直後は undo 不可)
- **`moveGame3Scene` PHASE_PLAYING 冒頭で R キー判定**: `status != GAME_STATUS_FINISHED && undoAvailable && CheckHitKey(KEY_INPUT_R) == 1` で `state/status/turn` を一括復元、`undoAvailable=false` に戻す。**FINISHED 状態では無効** (勝敗を尊重)
- **思考前スナップショット + 確定時 persist**: PLAYING の else (思考) ブランチでプレイヤー手番のみ pre-move 状態を一時保持、`think` が `true` 返却したら `prev*` に persist して `undoAvailable=true`。CPU 手番では保存しない (CPU の応手も含めて巻き戻る設計)
- **renderGame3Scene でガイド表示**: `undoAvailable && status != GAME_STATUS_FINISHED` 時のみ `PANEL_ROUND_LABEL_Y + 70` (= 290) 位置に「R: 待った」を `ColorSky` で描画。`PANEL_END_MSG_Y` (330) との衝突なし、FINISHED 中は非表示で誤解防止。テキストは初期実装の「Rキー: 待った」が 800x700 解像度の右端 (PANEL_X=590 開始でフォント 32 だと約 208px 必要、画面右端まで 210px しかなく「た」が見切れた) からはみ出したためプレイテスト後に短縮 — 1.5.6 リリース時の最終仕様
- **スコープは Game3 のみ**: Game1 (ふつう) / Game2 (まきもどり) は据え置き。難易度維持と「あまちゃん」モードの差別化を兼ねる
- **強制パス時の挙動**: `rbIsPass` true 時は `status = PASS_MSG` へ遷移し思考分岐に入らないため `prevState` は更新されない (パスは「手番をスキップ」であり「手を打った」ではない)。過去のプレイヤー手の `undoAvailable` は維持されるため、寛大な救済として PASS_MSG 中も R 押下で過去のプレイヤー手まで戻れる (CPU 応手 + パス込みで)
- **バージョン**: 本体変更ありなので 1.5.5 → 1.5.6 にバンプ ([GameMain.cpp:41](Project2/GameMain.cpp#L41) タイトル + [MenuScene.cpp:66](Project2/MenuScene.cpp#L66) メニュー)。CHANGELOG `[Unreleased]` セクション (LICENSE/README 分) を `[1.5.6] - 2026-06-26` に確定して待った機能と同梱公開
- **検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### Game3 ヒントマスへの取得コマ数表示追加 (完了, 2026-06-26)

§10 将来予定 (旧候補 1) の「ヒントマスへの取得コマ数表示」を実装。`rbDrawHints` をオレンジ丸描画のみから 2 パス構成に拡張。

- **新規定数** ([GameMain.h](Project2/GameMain.h)): `HINT_GAIN_FONT_SIZE = 20` を追加 (FONT_SIZE_DEFAULT=32 のすぐ下、CELL_PX=48 のマス内に 2 桁数値が収まる小さめサイズ)
- **`rbDrawHints` を 2 パス化** ([GameSceneMain.cpp](Project2/GameSceneMain.cpp)):
  - **パス 1**: 従来通り `SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128)` でオレンジ半透明丸を描画
  - **`SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0)` でブレンド解除** (パス 2 のテキストを不透明白で描画するため)
  - **パス 2**: 全マスを再走査し `int gain = rbPutPiece(..., false)` の戻り値を捕捉、`GetDrawFormatStringWidth("%d", gain)` で実描画幅を取得して中央寄せ、`DrawFormatString(cx - textW/2, cy - HINT_GAIN_FONT_SIZE/2, ColorWhite, "%d", gain)` で白文字描画
  - **フォントサイズの退避/復元**: 関数冒頭で `GetFontSize()` 退避 → パス 2 直前で `SetFontSize(HINT_GAIN_FONT_SIZE)` → 関数末尾で復元。後続の `rbDrawMsg`/`rbDrawCountPanel`/`rbDrawTurnIndicator` への副作用を遮断
- **色選定**: オレンジ円 (255,165,0) の上で白 (`ColorWhite`) が最も視認性が高い。黒文字も検討対象だが暗緑盤面とオレンジの境界で潰れやすいため不採用
- **2 パス化の理由**: 1 パスでテキストも同時描画するとアルファブレンド中のため白文字が 50% 半透明となりオレンジに溶けて読みにくい。`rbPutPiece` の 2 回呼び出しは 144 マス走査でも計算量無視できる範囲
- **検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### 盤面サイズ縮小モード + 雛形シーン名固定化 (完了, 2026-06-27, 1.5.8)

§10 将来予定 (Game3 拡張、旧候補 1) の「盤面サイズ縮小モード」を実装。同梱で **雛形シーン名 `Game4Scene` → `GameSceneTemplate` リネーム** を実施 (ユーザー指示、シーン番号スライドで陳腐化する名前を避ける固定名化)。プランモードでユーザー承認を得てから着手。

**盤面サイズ縮小モード本体**:

- **`ReversiOptions.boardSize` 追加** ([GameSceneMain.h](Project2/GameSceneMain.h)): 6/8/10/12 の 4 段階、デフォルト 12 で 1.5.7 動作と完全一致
- **`BOARD_SIZE_CHOICES[] = { 6, 8, 10, 12 }`** ([GameSceneMain.h](Project2/GameSceneMain.h) extern + [GameSceneMain.cpp](Project2/GameSceneMain.cpp) 定義): OPTIONS の ←→ 循環と loadOptions の有効値検証で共有参照
- **`ReversiBoard` に 4 フィールド追加** ([GameSceneMain.h](Project2/GameSceneMain.h)): `size` / `cellPx` / `originX` / `originY`。ストレージ `board[BOARD_SIZE][BOARD_SIZE]` は 12×12 固定維持 (有効領域は左上 size×size のみ)、メモリ使用量変化なし
- **サイズ別メトリクス計算**: cellPx = 576/size の規則 (10×10 のみ 57、他は割り切れる)。originX/Y は (576 - size×cellPx)/2 で中央寄せ補正 (10×10 のみ 8、他は 5)。初期 4 駒位置は size/2-1, size/2
- **`rbInit` シグネチャ拡張** `void rbInit(ReversiBoard*, int size)` — size から cellPx/origin を自動計算、想定外値は 12 フォールバック
- **全 `rb*` 関数を動的化**: rbPutPiece の境界 / rbIsPass / rbThinkCpu / rbThinkRandom / rbCheckResult / rbCountPieces の二重 for を `< state->size` に統一。rbThinkPlayer のマウス座標変換を `(mx - state->originX) / state->cellPx` に変更 (小盤面の中央寄せ補正対応、UB なく盤面外を無視)。rbRemovePieces の GetRand 範囲を `state->size - 1` に動的化 (Game2 専用呼出で size=12 のため挙動不変)
- **`rbDrawBoard` / `rbDrawGrid` シグネチャに `ReversiBoard*` 追加**: 終端座標を state->size × state->cellPx で動的計算
- **`rbDrawPieces` を `DrawExtendGraph` に変更**: コマ画像 (元 47px) を state->cellPx で拡大 (95/71/56/47px)。`right = left + state->cellPx - 1` でグリッド線跨ぎを回避
- **`rbDrawHints` 動的化**: 丸半径 = state->cellPx/3、取得コマ数フォントサイズ = state->cellPx * 5 / 12 (12→20 / 10→23 / 8→30 / 6→40)。1.5.7 の showGain ガードはそのまま
- **OPTIONS シーン拡張** ([OptionsScene.cpp](Project2/OptionsScene.cpp)): OPTION_COUNT 4→5、5 行目「盤面サイズ」追加。Enter/Space は 1〜4 でトグル、5 で次サイズ循環。**←→ 矢印キーで前/次サイズ** (5 行目選択時のみ)。`cycleBoardSize(current, step)` ヘルパで循環。右側 320×320 領域に盤面プレビュー (`renderBoardPreview(boardSize)` 静的描画、暗緑背景 + グリッド + 初期 4 駒の小円)。ガイド文言を 3 行→4 行に拡張
- **settings.ini boardSize キー** ([GameMain.cpp](Project2/GameMain.cpp)): `g_game3Options` 初期化に 12 追加、`loadOptions` で `sscanf_s("boardSize=%d", &v)` パース + 有効値 (6/8/10/12) のみ受理、`saveOptions` で `boardSize=%d` 行を書込
- **シーン側呼出修正**: Game1Scene `rbInit(&state, 12)` / Game2Scene `rbInit(&state, 12)` / Game3Scene `rbInit(&state, g_game3Options.boardSize)`。Game1/Game2 は 12 固定で完全挙動不変

**雛形シーン名固定化**:

- **ファイル物理リネーム**: `Project2/Game4Scene.cpp/.h` → `Project2/GameSceneTemplate.cpp/.h` (PowerShell `mv` で BOM 保持)
- **識別子置換**: `initGame4Scene` → `initGameSceneTemplate` 等の 5 関数 + ガード `GAMESCENE4_H_` → `GAMESCENETEMPLATE_H_` + 描画文字列 "ゲーム画面４です" → "Template Scene"
- **enum 変更**: `SCENE_GAME4` → `SCENE_TEMPLATE` ([GameSceneMain.h](Project2/GameSceneMain.h))
- **GameSceneMain.cpp**: `#include "Game4Scene.h"` → `#include "GameSceneTemplate.h"`、sceneTable[] エントリの識別子置換
- **Project2.vcxproj / .filters**: ClCompile/ClInclude の参照変更
- **新シーン追加手順を明文化**: §4 シーンアーキテクチャに「(1) GameSceneTemplate コピー → (2) 識別子置換 → (3) SCENE_NO enum 追加 → (4) sceneTable[] 追加 → (5) vcxproj 追加」の 5 ステップを記載

**版数**: 機能追加 (挙動変化あり) なので PATCH バンプ 1.5.7 → 1.5.8 ([GameMain.cpp](Project2/GameMain.cpp) タイトル + [MenuScene.cpp](Project2/MenuScene.cpp) メニュー)。雛形リネームは挙動不変だが本リリースに同梱で巻き込み

**動作上の留意点**:
- Game3 対局中に OPTIONS でサイズを変更しても、対局中は init 時のサイズで継続。サイズ変更を反映するにはメニューに戻って Game3 を再エントリする必要あり (4 トグルは毎フレーム参照で即時反映、boardSize は init 時確定)
- Game1/Game2 は `rbInit(&state, 12)` で完全挙動不変、難易度・盤面・思考すべて 1.5.7 と同等
- 初回起動時 (settings.ini なし) や不正値時は `g_game3Options.boardSize = 12` がデフォルト維持

**検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### 画面サイズ拡張 800×700 → 1280×768 + 全シーンリレイアウト (完了, 2026-06-27, 1.5.9)

1.5.6 (待った) → 1.5.7 (OPTIONS) → 1.5.8 (盤面サイズ縮小) と Game3 拡張機能を蓄積するうちに 800×700 が窮屈になり、**OPTIONS シーンでプレビュー領域 (450,100)-(770,420) がトグル列の `[ ON ]/[ OFF ]` 表示と完全に被って読めなくなる** バグをユーザーが報告。フォントの拡縮だけでは抜本対策にならず、画面解像度自体を引き上げて全シーンを再配置する大規模 UI リフォーム。プランモードでユーザー承認を得てから着手。

**画面高は二段階リファイン**: 初回は 1280×900 / プレビュー 500×500 で実装 → ユーザー検証で「上下が見切れタイトルバー操作不能」「プレビューは小ぶりでよい」のフィードバックを受けて 1280×**768** / プレビュー **320×320** に再調整。Y 座標と msg ボックスフォントを詰め直し、msg 専用に `MSG_FONT_SIZE=28` を新設して 768px 高に収めた。

**adversarial layout 検証 (Workflow 3 並列)**: 768px 化後の全シーンを 3 並列エージェントで critical/minor/edge の 3 段階重なり/オーバーフローを厳密検査。その結果 1.5.8 以前から潜在していた重なり問題も同時発見・修正:
- Game1 終局時に WHITE カウント数値と「Xキーで」ガイドが x=745 で 15px 重なる (font 32 当時は 7px 重なり、font 40 化で悪化) → `PANEL_END_MSG_GAME1_Y` 200 → 230
- Game3 PLAYER 名 nameTmp と「R: 待った」ガイドの line spacing が font 40 高さより 5px 不足 → spacing +35 → +50、ガイド +70 → +110

**画面・盤面サイズ拡張**:

- **SCREEN_WIDTH 800 → 1280 / SCREEN_HEIGHT 700 → 768** ([GameMain.h](Project2/GameMain.h)): 1.6× / 1.097× 拡張。Y は控えめで、Windows DPI 125% の 1080p ディスプレイ (物理 960px) でもタイトルバー + タスクバー込みで見切れない安全値
- **BOARD_TARGET_PX = 720** ([GameMain.h](Project2/GameMain.h) 新規定数): 旧 576px から 1.25× 拡張、6/8/10/12 すべて綺麗に割り切れる (120/90/72/60)
- **CELL_PX 48 → 60** (12×12 デフォルト時、`state->cellPx = BOARD_TARGET_PX / size` で動的算出)
- 6×6 のグリッド「粗さ」解消: 96px → 120px のセル、グリッド線間隔が広がって視覚的にスッキリ
- **PIECE_SIZE_PX 47 不変** (piece.png ソース画像サイズ、`DrawExtendGraph` で表示時に拡大)
- **FONT_SIZE_DEFAULT 32 → 40** (1.25× 拡張、画面解像度の拡張に伴う)
- **HINT_GAIN_FONT_SIZE 20 → 24** (上限固定値、動的算出 `state->cellPx * 5 / 12` は維持で 12→25 / 10→30 / 8→37 / 6→50 に自然スケール)

**rbInit 簡略化** ([GameSceneMain.cpp](Project2/GameSceneMain.cpp)):

- 1.5.8 の 10×10 特殊ケース (`(size == 10) ? 57 : 576/size`) を `BOARD_TARGET_PX / size` の単一式に統合
- `originX/Y` の中央寄せ補正計算 (576-totalPx)/2 を撤廃、全サイズで `BOARD_ORIGIN_X / BOARD_ORIGIN_Y` 固定 (720 = size × cellPx ぴったりのため)

**右パネル定数の更新** ([GameMain.h](Project2/GameMain.h)):

- `PANEL_X` 590 → 745 (盤面右端 725 + 20px 余白) / `PANEL_TURN_X` 680 → 900
- `PANEL_BLACK_LABEL_Y` 5→20 / `PANEL_BLACK_VALUE_Y` 40→65 / `PANEL_WHITE_LABEL_Y` 90→130 / `PANEL_WHITE_VALUE_Y` 125→175
- `PANEL_ROUND_LABEL_Y` 220 → 240 (font 40 化で WHITE_VALUE との重なり解消) / `PANEL_ROUND_VALUE_X` 710 → 925
- `PANEL_END_MSG_Y` 330 → 410 (Game2/Game3 終了メッセージ、Game3 R:待った 350..390 と独立) / `PANEL_END_MSG_GAME1_Y` 150 → 240 (1.5.8 から残っていた WHITE_VALUE との重なりも解消、25px gap)

**メッセージ箱の追従** ([GameMain.h](Project2/GameMain.h)):

- `MSG_BOX_CENTER_X` 192 → 365 (盤面中心 `BOARD_ORIGIN_X + BOARD_TARGET_PX/2 = 5 + 360`)
- `MSG_BOX_Y_TOP/BOTTOM` 630/655 → 730/762 (盤面下端 725 直下にタイトに収め)
- `MSG_TEXT_Y` 620 → 728 / `MSG_BOX_PADDING_X` 30 → 40
- 新規 `MSG_FONT_SIZE = 28`: rbDrawMsg 内で SetFontSize 退避/復元、msg を 28px フォントで描画して 768px 高に収める

**各シーンのレイアウト再配置** (768px 高に収まるよう Y を詰め直し済):

- **MenuScene** ([MenuScene.cpp](Project2/MenuScene.cpp)): タイトル (120,50) → (280,60)、メニュー項目 (130,140,gapY=80) → (260,170,gapY=90)、Credits (160,480) → (820,580) で SetFontSize(28) を局所適用 (5×28=140px 縦使用、768 内に収まる)、版数 1.5.8 → 1.5.9
- **Game3Scene NAME_ENTRY** ([Game3Scene.cpp](Project2/Game3Scene.cpp)): 全座標を 1280×768 に再配置 + フォントサイズ 45→55 / 30→36 / 40→50 に拡張。"NAME ENTRY" (280,50)→(480,80)、指示文 (160,100)→(280,180)、入力欄 (270,280)→(490,410)、下線 (265,330,515,330)→(485,470,795,470)、完了案内 (50,370)→(120,540)、X ガイド (50,600)→(120,700) (画面下端 768 内)
- **OptionsScene** ([OptionsScene.cpp](Project2/OptionsScene.cpp)): タイトルフォント 45→50 / 位置 (120,50)→(180,40)、項目列 (130,180,gapY=60)→(130,140,gapY=70) で 5×70=350px 縦使用、操作ガイド y=570 + SetFontSize(28) で 4×28=112px、`renderBoardPreview` プレビューサイズ **320 維持** (1.5.8 同じ) / 位置 (450,100)→(810,200) / ラベルフォント 20→22 (トグル列との水平距離 240px 確保で被り解消)
- **GameSceneTemplate** ([GameSceneTemplate.cpp](Project2/GameSceneTemplate.cpp)): "Template Scene" (30,50)→(60,100)、ボタン案内 (30,100)→(60,200)
- **Game1Scene / Game2Scene の本体**: 定数 (PANEL_X 等) 経由のみで、cpp 内のハードコード座標なし → 自動追従、ファイル直接変更なし

**版数**: 機能追加 (画面拡張は視覚的に大幅変化) なので PATCH バンプ 1.5.8 → 1.5.9 ([GameMain.cpp](Project2/GameMain.cpp) タイトル + [MenuScene.cpp](Project2/MenuScene.cpp) メニュー)

**動作不変保証**:
- ゲームロジック (rbPutPiece / rbIsPass / rbThinkCpu / rbThinkRandom / rbCheckResult / rbCountPieces / rbRemovePieces) は完全に挙動不変、`state->size`/`cellPx`/`originX/Y` の値が変わるだけで関数本体のコードは変更なし
- 思考テーブル / Game1/2/3 の状態遷移 / 「待った」機能 / OPTIONS の 4 トグル仕様 / settings.ini フォーマットも完全不変
- 1.5.8 形式の `settings.ini` (showHints/showGain/weakCpu/allowUndo/boardSize) はそのまま読込可能、互換性維持

**留意点**:
- piece.png (47×47 ソース) は `DrawExtendGraph` で 60/72/90/120px 表示に拡大される。約 2.5× 拡大で多少ジャギーが出る可能性 (将来高解像度差し替え余地、本リリースでは画像維持)
- `SetWindowSizeChangeEnableFlag(true)` で実行中リサイズも引き続き機能

**検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### カラーパレット拡張 ランク 10 ティア対応 + 汎用 UI 用途分離 (完了, 2026-06-27, 1.5.9.1)

§10 将来予定 (プロジェクト全体の拡張) 項目 2「カラーパレット増設 (継続的)」の第 2 弾。1.5.6.1 (初期 7 色) に続き 10 色をバッチ追加。プランモードでユーザー承認を得てから着手 (スコープ Option B: ランク章 +6 + 汎用 UI +4)。**動機**: B2 プレイヤーランクシステム本実装時に「ランク段階数を 6→8 に変えたい → 新色が足りない」となる順番を避けるための **準備的タスク**。テーマ化伏線も同梱して、後でテーマ周りを起こすときに色追加とテーマ実装を切り分ける。

**追加 10 色**:

- **ランク章 +6 色** (B2 ランクシステム 10 ティア対応の余裕枠): `ColorIron` (110,120,140、鋼鉄グレー、Bronze 下位のエントリーティア) / `ColorDiamond` (130,240,255、シアン水色、Platinum 上位の宝石系トップ) / `ColorEmerald` (50,200,130、ミントグリーン寄り) / `ColorRuby` (224,50,95、深紅ピンク寄り、ColorRed=純赤=「選択中」用と区別) / `ColorSapphire` (40,90,200、深サファイア青) / `ColorAmethyst` (170,100,200、アメジスト紫、既存色に紫系なし唯一性高)
- **汎用 UI +4 色** (テーマ化伏線、状態通知の用途分離): `ColorSuccess` (40,180,60、純緑寄り、Emerald ミントと用途分離) / `ColorError` (255,80,80、警報赤、ColorRed=純赤=「選択中」用と用途分離) / `ColorInfo` (120,180,240、明るい青、Sapphire 深青/Hover 淡水色と段階分離) / `ColorAccent` (255,130,200、マゼンタ寄り、既存にピンク系なし)

**色相区別の根拠** (近隣色との衝突回避):

- `ColorDiamond` (130,240,255) vs `ColorSky` (40,235,255) vs `ColorHover` (180,220,255): R 値で段階を分離 (40 / 130 / 180)、Sky=純シアン / Diamond=シアン水色 / Hover=淡水色
- `ColorRuby` (224,50,95) vs `ColorRed` (255,0,0) vs `ColorError` (255,80,80): Ruby=深紅ピンク寄り (彩度低) / Red=純赤 (「選択中」用) / Error=警報赤 (R 255 で輝度高)
- `ColorEmerald` (50,200,130) vs `ColorSuccess` (40,180,60): Emerald=ミント (B=130) / Success=純緑 (B=60)、用途分離 (Emerald はランク章、Success は状態通知)
- `ColorSapphire` (40,90,200) vs `ColorInfo` (120,180,240): Sapphire=深青 (彩度高) / Info=明るい青 (彩度低)、明度で段階分離

**ファイル変更**:

- [GameMain.cpp](Project2/GameMain.cpp): 既存 `ColorHover` 定義 (line 34) の直後に 2 ブロック追加 (ランク章 6 + 汎用 UI 4)、各色に用途分離コメント付与
- [GameMain.h](Project2/GameMain.h): 既存 `ColorWarn/Overlay/Hover` extern (line 32) の直後に 2 ブロック追加 (ランク章 6 を 1 行、汎用 UI 4 を 1 行)
- [GameMain.cpp](Project2/GameMain.cpp): ウィンドウタイトル `Reverse Reversi 1.5.9` → `1.5.9.1`
- [MenuScene.cpp](Project2/MenuScene.cpp): メニュー版数 `Ver 1.5.9` → `Ver 1.5.9.1`
- [CHANGELOG.md](CHANGELOG.md): [1.5.9.1] - 2026-06-27 セクション新設 + link table 追加
- §1 / §8 / §10 / §13 (本セクション) を更新

**バージョン**: 機能変更なし (全 10 色いずれの描画コードからも未参照、純粋に extern 定数追加のみ) なので **SUBPATCH バンプ 1.5.9 → 1.5.9.1**。CLAUDE.md §1 採番ルール「挙動不変なコード変更は SUBPATCH」 + §10 項目 2「まとまった色バッチでも SUBPATCH 1 つに集約可」(1.5.6.1 の 7 色バッチ前例) に沿う

**動作不変保証**: 全シーンの描画 / メニュー操作 / 対局進行 / 「待った」 / OPTIONS / settings.ini 永続化はすべて 1.5.9 と完全同一。新規 10 色は B1/B2/テーマ化等の将来機能で参照される伏線

**`ColorXxx` extern 総数**: 10 → **20** (ランク章 4 → 10 / 汎用 UI 3 → 7 / 基本 3 不変)

**検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### MenuScene Credits 見切れ修正 (完了, 2026-06-27, 1.5.9.2)

1.5.9.1 リリース後のユーザープレイテストで、メニュー画面右下の Credits 表示 (Made with DX-Library 3.24f / BGM / by Senses Circuit / URL) の URL 末尾 (".com/") が画面右端で見切れる事象を確認。フォントを小さくする方針で対策 (ユーザー指示)。

**原因**: 1.5.9 で 1280×768 に画面拡張した際 Credits を画面右下 (x=820, y=580) に再配置し SetFontSize(28) を採用したが、URL `https://www.senses-circuit.com/` (31 文字、ASCII チャンク) を MS 明朝で描画すると 1 文字 ~16px × 31 ≈ 496px の幅となり、画面右端まで残された 460px (= 1280 - 820) に収まらず約 36px ぶん見切れた

**修正** ([MenuScene.cpp](Project2/MenuScene.cpp)): `SetFontSize(28)` → **`SetFontSize(22)`** に変更
- 1 文字幅 16px → ~12.6px (22/28 = 0.786×) で 31 文字 ≈ 391px に縮小、460px 余白に 69px マージン込みで収まる
- 5 行高さ 140px → **110px** に圧縮、y=580 開始 → 690 終了で 768 内に 78px 下マージン確保
- 視認性: 22px はメニュー本文 40px の約半分だが、Credits は補足情報のためマージンを取って小さくしても問題なし

**スコープは Credits のみ**: タイトル "まきもどリバーシ Ver 1.5.9.2" (フォント 40)、メニュー項目 4 行 (フォント 40)、ゲーム本体シーンはすべて 1.5.9.1 と完全同一

**バージョン**: コード変更で挙動 (Credits 見え方) が変わるが、視覚的 fit 修正のみで機能追加/削除なし、SUBPATCH バンプ 1.5.9.1 → **1.5.9.2** ([GameMain.cpp](Project2/GameMain.cpp) タイトル + [MenuScene.cpp](Project2/MenuScene.cpp) メニュー)

**検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

### B2 プレイヤーランクシステム実装 (完了, 2026-06-27, 1.6.0)

§10 将来予定 (プロジェクト全体の拡張) 旧候補 1 「プレイヤーランクシステム」の本実装。1.5.6.1 (B3 カラーパレット初期 7 色) → 1.5.9.1 (B2 準備として +10 色 = 計 20 色) と段階的に伏線を回収し、いよいよ本体機能を実装。プランモードで Ultracode Workflow を使い、3 並列エージェントで設計案を比較 (案 A シンプル 160 行 / 案 B 完全 950 行 / 案 C エンゲージメント 360 行) → ユーザー判断で **案 C エンゲージメントベース + 全モード降格 (案 B より厳格) + 案 B 多段アニメ演出 + OPTIONS リセットトグル** のハイブリッド構成を採用 → ユーザー指示でバージョンを **MINOR バンプ 1.5.9.2 → 1.6.0** (フェーズ ε 相当扱い、§1 採番ルールも更新)。

**実装内容 (詳細は [CHANGELOG.md](CHANGELOG.md) [1.6.0] 参照)**:

- **10 ティアシステム** (NOVICE → ETERNAL、累積 XP 閾値 0/50/150/350/700/1200/2000/3200/5000/8000、各ティアに ColorIron 〜 ColorAmethyst の 10 ランク色を割当)
- **XP 計算式** ([GameSceneMain.cpp `calcXpGain`](Project2/GameSceneMain.cpp)): 勝=30+コマ差ボーナス(上限+30)+短手数+10+完封+15 / 引分=15 / 負=10、モード倍率 Game1=×1.0 / Game2=×1.4 / Game3=×0.6、Game3 オプションペナルティ (ヒント/取得数/弱CPU/待った の ON 数 × 0.1 を 1.0 から減算してさらに乗算)
- **全モード降格** ([`applyXpAndCheck`](Project2/GameSceneMain.cpp)): 敗北時に「降格圧力」 5+tier×2 XP 減算、現ティア閾値 - DEMOTE_BUFFER_XP (50) を下回ったら 1 ティア降格 (Game1/Game2/Game3 全モード、ユーザー指示で案 B の Game3 保護を撤回)
- **多段ランクアップ演出** ([`rbDrawRankUpAnimation`](Project2/GameSceneMain.cpp)) 240f アニメ: (1) 0-30f 半透明オーバーレイフェードイン + 効果音 res/loop_68.wav 流用、(2) 30-90f 中央 (640,384) にランク章を半径 10→120px でスケールアップ + 回転、(3) 90-180f ティア名 1 文字 12f ずつフェードイン、(4) 180-240f NEW RANK! を 10f 周期で点滅。X キー / ボタン 1 でスキップ可
- **降格演出** ([`rbDrawDemoteAnimation`](Project2/GameSceneMain.cpp)) 120f: (1) 0-30f 赤フラッシュ、(2) 30-90f DEMOTED... 表示、(3) 90-120f 新ティア章
- **MenuScene 右上にランク章 + 進捗バー + 次閾値 XP 表示** ([MenuScene.cpp](Project2/MenuScene.cpp)): 中心 (1080,80) 半径 32 章 + ティア名フォント 28 + 進捗バー 200×12px (背景 ColorOverlay + 進捗 ColorSuccess) + 「次まで N XP」フォント 18。ETERNAL 到達時は ColorAccent 満タン + 「MAX TIER」
- **対局画面右パネル下部に小ランク章** ([`rbDrawRankBadgeSmall`](Project2/GameSceneMain.cpp)): 半径 24 + ティア名フォント 22、Y=480、FINISHED/RANK_UP/DEMOTED 中は非表示
- **終局時 XP オーバーレイ** ([`rbDrawResultOverlay`](Project2/GameSceneMain.cpp)): 「+47 XP」ティア色フォント 48 中央 (640,350) + 「TOTAL: 423 / NEXT: 277 XP」フォント 28
- **新規 GAME_STATUS 拡張** ([GameSceneMain.h](Project2/GameSceneMain.h)): `GAME_STATUS_RANK_UP` (=5) / `GAME_STATUS_DEMOTED` (=6) を追加。FINISHED 後に演出状態へ遷移
- **永続化 settings.ini 拡張 4 キー** ([GameMain.cpp](Project2/GameMain.cpp)): totalXp / currentTier / totalGames / totalWins。1.5.x 形式と完全後方互換 (新キー不在時は 0 初期値で NOVICE 起動)
- **OptionsScene 6 行目「ランクをリセット」** ([OptionsScene.cpp](Project2/OptionsScene.cpp)): OPTION_COUNT 5→6、Enter 1 回目で 180f 確認モード「もう一度 Enter で確定」(ColorWarn)、2 回目で全リセット + 「リセット完了!」60f 表示 (ColorSuccess)
- **ReversiBoard に 2 フィールド追加**: `moveCount` (rbPutPiece の put_flag=true 確定時に ++) + `passCount` (将来のヘッドルーム)
- **Game3 専用 `undoUsedInMatch` ファイルスコープ static**: R キー押下時に true、XP 計算で g3Undo=true 扱い (×0.9 ペナルティ)

**カラーパレット 20 色の伏線回収**:
1.5.9.1 まで未参照だった `ColorIron/Bronze/Silver/Gold/Platinum/Diamond/Emerald/Ruby/Sapphire/Amethyst` 10 ランク色を `TIER_COLORS[10]` で 1:1 対応、`ColorSuccess` を進捗バー、`ColorOverlay` を背景半透明、`ColorWarn` をリセット確認文言、`ColorError` を降格演出、`ColorAccent` を ETERNAL 進捗バー満タン、`ColorGold` を NEW RANK! テキストと MAX TIER 表示にそれぞれ活用。

**バージョン**: ユーザー指示で **MINOR バンプ 1.5.9.2 → 1.6.0** (フェーズ ε 扱い、§1 採番ルールを微更新: 大型機能リリースも MINOR バンプ対象に追加)

**実装パターン**: 6 段階ビルド検証 (各段階で Debug|Win32 リビルド警告 0/エラー 0 確認) で安全に積み上げ:
1. 基盤拡張 (PlayerStats / TIER 定数 / GAME_STATUS 拡張 / settings.ini 4 キー / 8 新規関数プロトタイプ)
2. 8 新規関数の本実装 (calcXpGain / applyXpAndCheck / rbDrawRankBadgeSmall / rbDrawResultOverlay / rbDrawRankUpAnimation / rbDrawDemoteAnimation / getTierName / getTierColor)
3. ReversiBoard に moveCount/passCount 追加 + rbInit ゼロクリア + rbPutPiece の put_flag=true パスで ++
4. Game1/2/3Scene の FINISHED 突入時に XP 集計 + 状態振り分け + RANK_UP/DEMOTED case 追加 + render 拡張 (Game2 は Round 2 のみ XP 計算)
5. MenuScene 右上ランク表示 + OptionsScene 6 行目「ランクをリセット」 + 確認/完了タイマー
6. CHANGELOG + CLAUDE.md 更新 + 版数 1.6.0 バンプ

**留意点**:
- Game2 は **Round 2 終了時のみ XP 計算** (Round 1 は中間状態として扱う、ユーザー指示)
- Game3 オプション全 ON で mode mult ≈ 0.36 と最低 XP 効率、全 OFF で ×0.6 で Game1 (×1.0) より低いが難易度的に同等のため「あえて Game3」プレイヤー向け選択肢
- ETERNAL (8000 XP) 到達目安: Game1 標準勝利 30〜50 XP なら 200〜250 試合
- 既存リソース流用 (新規 wav 追加なし): ランクアップ効果音は既存 `res/loop_68.wav` を `DX_PLAYTYPE_BACK` で流用
- ゲームロジック (rbPutPiece の盤面操作・rbIsPass・rbThinkCpu/rbThinkRandom・rbCheckResult・rbCountPieces・rbRemovePieces) は **完全に挙動不変** — `state->moveCount++` の 1 行追加と `rbInit` での 2 フィールド初期化のみ
- 1.5.x 既存 settings.ini は完全後方互換、新 4 キー不在時は g_playerStats = { 0, 0, 0, 0 } のデフォルトで NOVICE 起動

**検証**: 全 4 構成リビルド (Debug|Win32 / Release|Win32 / Debug|x64 / Release|x64) いずれも警告 0 / エラー 0

---
