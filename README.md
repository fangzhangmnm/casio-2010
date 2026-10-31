# casio-2010

GWHBOB 2010–2013 年在卡西欧 fx-9750GII（刷 9860 系统）上写的游戏，在浏览器里跑：

- 卡西欧 BASIC 原件：贪吃蛇（2010）、21 点（2010）、三国杀简版（2010）、塔防 / 植物大战僵尸（2010）、扫雷（2012）
- C 写的 Add-in，当年的源码编成 WebAssembly：魔塔（2012）、3D 引擎（2012）、Doodle Jump（2013）
- 魔塔和 Doodle Jump 另有 2026 版（修当年的 bug、锁帧算 dt、重力感应、触屏），不是默认，和原样版共用存档

平时是照真机布局的整台计算器；「全屏」是手游模式，只放用得到的键。

在线玩：https://fangzhangmnm.github.io/casio-2010/

程序当年发在 cnCalc 计算器论坛（https://www.cncalc.org/ ）。出处见网页底部「出处和致谢」，第三方许可全文见 `THIRD_PARTY.md`。

## 仓里有什么

- `index.html`：网页版（上面那个链接就是它）。`emulator/build.py` 生成，别手改。
- `originals/`：当年的原件，一个字节没改；人写的工程、工具、素材都在这里。每个文件从哪来、原始时间、sha256 见仓根的 `MANIFEST.md`。源码是 GBK 编码。
  - `basic/`：卡西欧 BASIC 程序（G1M）——贪吃蛇、21 点、扫雷（两版）、三国杀简版、塔防（1.0 / 1.1）；`listings/` 是解码出来的文字版；塔防帖子里的 6 张截图（`TDGAME-screenshot.jpg` + `TDGAME-screenshots/`），网页的图形屏就是照它们逐像素对的。
  - `magtower/`：魔塔的 C 源码（2012-08-07）、08-09 更新的 G1A、中文字库、图块和界面素材、当年自己写的地图编辑器和图片转换器、截图。
  - `doodle/`：Doodle Jump 2013 年的工程（源码 + G1A）和 2011 年论坛上的 V0.9。
  - `3d-engine/`：3D 引擎的源码和四个 G1A（2011-12 → 2012-02）、截图。
  - `ogame/`：Ogame（2013-02）。
  - `visualfx/`：VisualFX 界面库和测试程序（2013-02），用了 diameter 的 fxChsLib 中文库。
  - `df/`：东方风格弹幕游戏的骨架（2013-04），带几张 16×16 角色小像。
  - `chinese-input/`：拼音输入法（2012-08），在 diameter 的 SimSpell 上改的。
- `2026/`：2026 年的版本。
  - `doodle/Doodle2026.c`：Doodle Jump 2026 版。
  - `magtower/`：魔塔 2026 版的图块补丁（`fix-mapedit-hex.json`）和生成它的 `make_fix.py`；`make_ora.py` 把图块总表打包成 .ora 给 WeebPaint 改。
- `emulator/`：网页模拟器（2026 年写的）。
  - `shell.html`：页面本体——卡西欧 BASIC 解释器、计算器外壳、触屏、重力感应、存储内存。
  - `fxsdk/`：卡西欧 fx-9860G SDK + Revolution-FX 的替身（只写了这几个游戏用到的函数），`fx2026.h` 是 2026 版专用的扩展接口。
  - `build_g1a.py`：把 C 源码编成 WebAssembly；`build.py`：把一切填进 `shell.html`，产出 `index.html`。
  - `casio-font.json` / `casio-font-small.json`：从卡西欧 SDK 字符码表采的大字 / 小字。
- `tools/g1m_listing.py`：G1M 原件解码成文字清单。
- `ai-docs/`：2026 年的设计文档，先读 `ai-docs/20261031-emulator-design.md`；3D 引擎当年的算法（伪代码层次）在 `ai-docs/20261031-3d-engine-algorithm.md`。
- `wishlist.md`：还没做的。

## 怎么编

要 Python 3 和 Emscripten（emsdk；环境变量 `EMSDK` 指向它）。

```
python3 -I emulator/build.py              # → index.html，约 5 秒
python3 -I 2026/magtower/make_fix.py      # 重算魔塔 2026 版的图块补丁（一般不用跑）
python3 -I tools/g1m_listing.py <BIDE 的 opcodes.txt>   # 重新生成 originals/basic/listings/
```

> 网页壳、BASIC 解释器、SDK 替身、2026 版由 Claude（Anthropic）在 2026 年写。本文件 created 20261031 by Claude Opus 5.5 · edited 2026-10-31（源码、工具、设计文档搬进本仓）。
