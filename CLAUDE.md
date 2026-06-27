# CLAUDE.md — ReversedReversi プロジェクト情報

Visual Studio (MSVC) + DxLib による C++ リバーシゲーム。本体は [Project2.sln](Project2.sln) / [Project2/](Project2/) 配下。タイトルバー表記は「Reverse Reversi 1.5.7」、メニュー描画は「まきもどリバーシ Ver 1.5.7」([Project2/MenuScene.cpp:66](Project2/MenuScene.cpp#L66))。日本語 Windows 環境 (コードページ 932) でビルドする前提。バージョン履歴は [CHANGELOG.md](CHANGELOG.md)、ライセンスは [LICENSE.md](LICENSE.md) (MIT)、公開向け案内は [README.md](README.md) を参照 (採番ルール: フェーズ MINOR + サブターゲット PATCH + **挙動不変なコード変更は SUBPATCH** (4 セグメント、1.5.6.1 で導入)、ドキュメント/メタファイルのみの変更は据え置き — CHANGELOG 冒頭参照)。

姉妹プロジェクトに [TwistTimeStopper](d:\Repositories\TwistTimeStopper) があり、シーン管理の雛形を共有している (元は同じ「Scene管理付き空プロジェクトRev2」テンプレート)。TwistTimeStopper は既に α/β/γ/δ のリファクタを完走しており、共通基盤化のリファレンスとして本ファイル中で頻繁に参照する。

---

## 1. プロジェクト概要

**まきもどリバーシ** は 12×12 盤面のリバーシ亜種。メニューから 3 モードを選択する構成で、γ-3 (2026-06-26) で **3 モード全て完成**:

| メニュー位置 | 表示名 | SCENE_NO | ファイル | 状態 |
|---|---|---|---|---|
| 0 | `ふつうの次元：頭をほどよく使う` | `SCENE_GAME1` | [Game1Scene.cpp](Project2/Game1Scene.cpp) | **完成** (プレイヤー vs `rbThinkCpu` 貪欲) |
| 1 | `あまちゃん次元：頭をあまり使わない` | `SCENE_GAME3` | [Game3Scene.cpp](Project2/Game3Scene.cpp) | **完成** (名前入力 → プレイヤー vs `rbThinkRandom` ランダム CPU、γ-3 完了) |
| 2 | `まきもどり次元：頭をかなり使う` | `SCENE_GAME2` | [Game2Scene.cpp](Project2/Game2Scene.cpp) | **完成** (2 ラウンド制、`rbRemovePieces` でラウンド間 96 マス削除) |

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
    SCENE_OPTIONS,      // オプション設定画面 (1.5.7 で導入、Game3 のトグル設定)
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
| Game4Scene.h | `GAMESCENE4_H_` |
| GameStatus.h | `GAMESTATUS_H_` |
| GameSceneMain.h | `GAMESCENEMAIN_H_` |

Game1〜4Scene は **`GAMESCENE<N>_H_`** (数字は GAMESCENE の後)。`GAME<N>SCENE_H_` ではない点に注意 (TwistTimeStopper 流)。

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
- **色**: `GetColor(R, G, B)` の戻り値 `unsigned int` はキャッシュ。β-D-2 で [GameMain.cpp:20-22](Project2/GameMain.cpp#L20) に `ColorWhite/Red/Sky` を集約定義 + [GameMain.h](Project2/GameMain.h) で `extern` 公開 (旧 `ColorXxx2` 接尾辞は撤廃済)。1.5.6.1 (B3) で **ランク章用 4 色** (`ColorBronze/Silver/Gold/Platinum`) + **汎用 UI 用 3 色** (`ColorWarn/Overlay/Hover`) を追加、計 10 色。新規色を追加する際は同じ `ColorXxx` 命名 + extern 宣言 (GameMain.h) + 定義 (GameMain.cpp) の構造に合わせる。**カラーパレットは継続的拡張対象** — 新色 1 つ追加ごとに SUBPATCH バンプ (1.5.6.1 → 1.5.6.2 → ...)、機能と同時追加なら機能側 PATCH バンプに巻き込む方針 ([CHANGELOG.md](CHANGELOG.md) 採番ルール参照)
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
- **`SCENE_GAME3` (あまちゃん次元、旧 SCENE_GAME5)** — γ-3 (2026-06-26) で独自モード化完了。内部 2 フェーズ構造 (`GAME3_PHASE_NAME_ENTRY` → `GAME3_PHASE_PLAYING`)。名前入力は `KeyInputSingleCharString` + Enter キー検出で確定 (元の 1 文字入力即遷移バグ解消)。対局は Game1 と同じ盤面ロジック + 思考テーブル `{ rbThinkPlayer, rbThinkRandom }` で**弱い CPU** (置ける場所からランダム選択) を採用、初心者でも勝てる難易度。対局画面の右パネルに `PLAYER:` として `nameTmp` を表示。後続セッション (2026-06-26) で **ヒント表示** (`rbDrawHints`、置けるマスにオレンジ半透明丸、プレイヤー手番中のみ) + さらに後続 (2026-06-26) で **ヒントマスへの取得コマ数表示** (オレンジ円の中心に白で裏返り枚数を中央寄せ表示、`HINT_GAIN_FONT_SIZE=20`) を追加。1.5.6 (2026-06-26) で **「待った」機能** (R キー、`prevState`/`prevStatus`/`prevTurn`/`undoAvailable` static で前手スナップショット保持、CPU 応手も含めた巻き戻し、FINISHED 中は無効、`undoAvailable && status != FINISHED` 時のみ「R: 待った」ガイド表示) を追加。1.5.7 (2026-06-26) で **オプション設定のトグル化** (`SCENE_OPTIONS` 新設、ヒント / 取得コマ数 / 弱い CPU / 待った の 4 トグルを `g_game3Options` で管理、`settings.ini` 永続化) を追加

### 未完成 (要対応)

(現状なし — 全 3 モード γ-3 で完成)

### 将来実装予定 (Game3 あまちゃん次元の拡張)

γ-3 + 後続 (2026-06-26) + 1.5.7 で「弱い CPU + ヒント表示 + 取得コマ数表示 + 待った機能 + オプション設定 (Game3 専用、`SCENE_OPTIONS` 経由で 4 トグル ON/OFF、`settings.ini` 永続化)」を実装済。以下は未着手の追加候補:

1. (任意) **盤面サイズ縮小モード** — 12×12 → 8×8 など。簡単な対局向け。実装規模: 大 (盤面サイズを定数固定から動的化、`rbInit` 拡張、`rbDraw*` 関数群のパラメータ化が必要)

### 将来予定 (プロジェクト全体の拡張)

Game3 専用ではなく、ふつう/まきもどり/あまちゃんを横断するシステム機能:

1. **プレイヤーランクシステム** — プレイの技巧 (勝敗、取得コマ差、無パス継続、勝った相手の強さ等) に応じて経験値を蓄積、目標値到達でランクアップ。ランク階層案 (英語表記): `NOVICE → APPRENTICE → ADEPT → EXPERT → MASTER → GRAND MASTER` または将棋風 (`級位 → 段位`)。**プレイヤー名 (Game3 の `nameTmp`) はあくまでハンドルネーム**、ランクは別軸の称号として右パネル `PLAYER:` 行と並べて表示する案。実装規模: 大 — 永続化が必須 (`save.dat` 等のバイナリ/INI/JSON、または既存 `settings.ini` の拡張、`GameMain` 起動時に読込・終了時に書込)、XP 計算ロジック (`int calcXpGain(int winnerColor, int gain, int passes, ...)` 等)、ランク閾値テーブル、ランクアップ演出、メニュー表示への反映 (タイトル横にランク章を出す等)。要設計検討事項多数 (各モード別に XP 倍率を変えるか / 取り戻しなしモードのみ XP 計上にするか / リセット手段 / モデレーション)
2. **カラーパレット増設 (継続的)** — 1.5.6.1 (2026-06-26) で初期 7 色 (`ColorBronze`/`Silver`/`Gold`/`Platinum`/`Warn`/`Overlay`/`Hover`) を `ColorXxx` 命名で extern 追加済 ([CHANGELOG.md](CHANGELOG.md) [1.5.6.1] 参照)。本リリースでは未参照、B1/B2/テーマ化等の将来機能で使用される伏線。**継続的タスク** として残置: 今後の機能拡張で新色が必要になり次第、同じ `ColorXxx` 命名で [GameMain.cpp](Project2/GameMain.cpp) 定義 + [GameMain.h](Project2/GameMain.h) extern 追加を継続。1 色追加ごとに SUBPATCH バンプ (1.5.6.1 → 1.5.6.2 → ...)、機能と同時追加の場合は機能側 PATCH バンプに巻き込み。**未対応**: 既存インライン `GetColor()` 呼び出し (盤面色 `(0,100,20)` / `(0,140,20)`、メッセージ箱グレー `(150,150,150)`、ヒントオレンジ `(255,165,0)`、MenuScene/Game4Scene の白赤リテラル) の extern 化 — 別タスクで一括対応する場合 `BoardColorDark/Light` `MsgBoxBg` `ColorHintOrange` 等の命名統一を要設計検討

### 将来予定 (リポジトリ整備)

ゲーム機能とは別系統のリポジトリ/ドキュメント整備タスクは **全完了**:

- **CHANGELOG.md** 分割 → 1.5.5 で完了 ([CHANGELOG.md](CHANGELOG.md))
- **LICENSE.md** (MIT) → 1.5.6 で公開 ([LICENSE.md](LICENSE.md))、第三者ライブラリ (DxLib) ライセンス遵守注記を末尾セクションに併記 (作成自体は LICENSE/README コミット時、本体未変更だったため一時的に `[Unreleased]` に記録 → 1.5.6 で待った機能と同梱公開)
- **README.md** (GitHub 公開用、JA 上 / EN 下) → 1.5.6 で公開 ([README.md](README.md))、テキストのみ (スクリーンショットは将来追加余地、同上)

追加候補 (任意): 将来 README にスクリーンショットを追加する場合は `docs/screenshots/` ディレクトリを設置、メニュー画面 + Game3 ヒント表示画面の 2 枚程度が想定。

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

---
