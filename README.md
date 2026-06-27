# まきもどリバーシ (Reverse Reversi)

12×12 盤面のリバーシ亜種。Visual Studio + DxLib で実装された Windows 用 C++ ゲーム。

3 つのゲームモードと、初心者向けのヒント表示機能を搭載しています。

---

## 特徴

- **12×12 拡張盤面** — 通常リバーシ (8×8) より大きい盤面で長時間プレイ
- **3 モード構成** — 通常 / まきもどり / 初心者向けの 3 種類のゲームモードから選択
- **まきもどり次元** — 2 ラウンド制 + ラウンド間の盤面リセット (96 マスをランダムに削除) + BGM 切替の独自演出
- **あまちゃん次元** — 初心者向けに弱い CPU + 置けるマスを示すヒント表示 + 取得できるコマ数を可視化

---

## ゲームモード

### ふつうの次元 — 頭をほどよく使う
プレイヤー (黒) vs CPU (白)。CPU は貪欲アルゴリズム (最多取得選択) の中程度の難易度。一般的なリバーシ対局を 12×12 で楽しみたい方向け。

### まきもどり次元 — 頭をかなり使う
プロジェクト名の由来となったモード。2 ラウンド構成:

- **ラウンド 1**: 通常のリバーシ進行
- **ラウンド間** (約 4 秒): 盤面 96 マスをランダム削除して巻き戻し風にリセット
- **ラウンド 2**: 2 局目を開始、終局でゲーム終了

BGM はラウンドで切り替わります。長時間 + 高難度プレイ向け。

### あまちゃん次元 — 頭をあまり使わない
初心者向け 1 戦完結モード。

- 最初にプレイヤー名 (アルファベット) を入力
- 対局相手の CPU は **弱い** (置ける場所からランダム選択)
- プレイヤー手番中、置けるマスがオレンジ半透明丸でハイライト
- 各ヒント丸の中に「そのマスに置いた場合に裏返るコマ数」を白字で表示

初めてリバーシに触れる方や、気軽に勝ちたい方向け。

---

## ビルド要件

- Windows 10 / 11
- Visual Studio 2026 Insiders (PlatformToolset **v145**)
- [DXライブラリ (DxLib)](https://dxlib.xsrv.jp/) 3.24f を `C:\DxLib` にインストール
- 文字セット: MultiByte (CP932)
- ランタイム: `/MT` (静的 CRT)
- 対応構成: Debug / Release × Win32 / x64 の 4 構成

DxLib パスは [Project2/Common.props](Project2/Common.props) の `<DxLibDir>` で集約管理しています。別の場所にインストールしている場合は同ファイルを 1 行書き換えるだけで全 4 構成に反映されます。

---

## ビルド方法

1. 本リポジトリをクローン
2. DxLib 3.24f を `C:\DxLib` にインストール (別パスの場合は [Project2/Common.props](Project2/Common.props) を編集)
3. Visual Studio で [Project2.sln](Project2.sln) を開く
4. 構成 (例: `Debug | Win32`) を選択
5. メニュー → ビルド → ソリューションのリビルド (`Ctrl+Alt+F7`) または ビルド (`Ctrl+Shift+B`)
6. 出力: `Debug/Project2.exe` / `Release/Project2.exe` / `x64/Debug/Project2.exe` / `x64/Release/Project2.exe`

---

## 操作方法

### メニュー画面
- **↑ / ↓ キー (またはゲームパッド)**: メニュー項目を移動
- **Enter / ゲームパッドボタン 1**: 決定

### ゲーム画面 (3 モード共通)
- **マウス左クリック**: コマを置く (置けるマスのみ反応)
- **X キー**: ゲーム終了後にメニューへ戻る
- **ESC キー**: アプリケーション終了

### あまちゃん次元の名前入力画面
- **アルファベットキー**: 名前を入力 (最大 11 文字)
- **Enter キー**: 名前を確定して対局開始
- **X キー**: メニューへ戻る

---

## ライセンス

本ソフトウェア本体は **[MIT License](LICENSE.md)** で配布しています。Copyright (c) 2026 OutRose。

ただし本ソフトウェアはサードパーティの **[DXライブラリ (DxLib)](https://dxlib.xsrv.jp/)** (作者: 山田 巧 氏) を使用しています。**バイナリ再配布時には DXライブラリの著作権表記とライセンス全文の同梱が必要**です。詳細は [LICENSE.md](LICENSE.md) 末尾の「第三者ライブラリのライセンス」と、DxLib 公式サイトの「DXライブラリ著作権表記及び使用条件」を参照してください。

---

## ドキュメント

- [CHANGELOG.md](CHANGELOG.md) — バージョン履歴 (Keep a Changelog 形式)
- [CLAUDE.md](CLAUDE.md) — プロジェクト詳細仕様 + リファクタリング履歴 + 設計判断ログ
- [LICENSE.md](LICENSE.md) — MIT License + サードパーティライセンス情報

---

## クレジット

- **ゲーム実装**: OutRose
- **使用ライブラリ**: [DXライブラリ (DxLib)](https://dxlib.xsrv.jp/) 3.24f (山田 巧 氏)
- **BGM**: hitoshi & ambnience ([Senses Circuit](https://www.senses-circuit.com/))

---

# Reverse Reversi (English Summary)

A 12×12 reversi variant written in C++ using DxLib, for Windows. Three game modes are included.

## Game Modes

- **Normal Dimension (ふつうの次元)** — Player (black) vs greedy CPU (white). Standard reversi gameplay on an expanded board.
- **Rewind Dimension (まきもどり次元)** — Two-round play; between rounds, 96 random squares are cleared and BGM switches. Long, high-difficulty play.
- **Novice Dimension (あまちゃん次元)** — Beginner-friendly mode. Enter your name, then play against a weak CPU that picks moves randomly. Placeable squares are highlighted with translucent orange circles, and the number of pieces you would flip is shown inside each hint circle.

## Build Requirements

- Windows 10 / 11
- Visual Studio 2026 Insiders (PlatformToolset **v145**)
- [DxLib](https://dxlib.xsrv.jp/) 3.24f installed to `C:\DxLib`
- Character set: MultiByte (CP932)
- Runtime: `/MT` (static CRT)
- Configurations: Debug / Release × Win32 / x64 (all four supported)

DxLib path is consolidated in [Project2/Common.props](Project2/Common.props) — edit `<DxLibDir>` on one line to use a different install location.

## Build Steps

1. Clone this repository
2. Install DxLib 3.24f to `C:\DxLib` (or edit [Project2/Common.props](Project2/Common.props) for a custom path)
3. Open [Project2.sln](Project2.sln) in Visual Studio
4. Select a configuration (e.g. `Debug | Win32`)
5. Build → Rebuild Solution (`Ctrl+Alt+F7`) or Build Solution (`Ctrl+Shift+B`)
6. Output: `Debug/Project2.exe`, `Release/Project2.exe`, `x64/Debug/Project2.exe`, or `x64/Release/Project2.exe`

## Controls

### Menu
- **↑ / ↓ arrow keys (or gamepad)** — Navigate menu items
- **Enter / gamepad button 1** — Confirm

### In-game (all modes)
- **Mouse left click** — Place a piece (only on legal squares)
- **X key** — Return to menu (after the game ends)
- **ESC** — Quit application

### Novice Dimension name entry
- **Alphabet keys** — Type your name (up to 11 characters)
- **Enter** — Confirm name and start the match
- **X key** — Return to menu

## License

The game itself is distributed under the **[MIT License](LICENSE.md)** — Copyright (c) 2026 OutRose.

This software uses the third-party **[DxLib](https://dxlib.xsrv.jp/)** library by Takumi Yamada. Binary redistributions must include the DxLib copyright notice and license terms. See the "Third-Party Licenses" section at the end of [LICENSE.md](LICENSE.md) and the DxLib official site for details.

## Documentation

- [CHANGELOG.md](CHANGELOG.md) — Version history (Keep a Changelog format)
- [CLAUDE.md](CLAUDE.md) — Detailed project specification and refactoring log (Japanese)
- [LICENSE.md](LICENSE.md) — MIT License plus third-party license notes

## Credits

- **Game implementation**: OutRose
- **Library**: [DxLib](https://dxlib.xsrv.jp/) 3.24f by Takumi Yamada
- **BGM**: hitoshi & ambnience ([Senses Circuit](https://www.senses-circuit.com/))
