# casio-2010

GWHBOB 2010–2013 年在卡西欧 fx-9750GII（刷 9860 系统）上写的游戏，在浏览器里跑：

- 卡西欧 BASIC 原件：贪吃蛇（2010）、21 点（2010）、扫雷（2012）
- C 写的 Add-in，当年的源码编成 WebAssembly：魔塔（2012）、3D 引擎（2012）、Doodle Jump（2013）
- 魔塔和 Doodle Jump 另有 2026 版（修当年的 bug、锁帧算 dt、重力感应、触屏），不是默认，和原样版共用存档

平时是照真机布局的整台计算器；「全屏」是手游模式，只放用得到的键。

在线玩：https://fangzhangmnm.github.io/casio-2010/

这个仓只放编好的网页（`index.html` 一个文件）。程序当年发在 cnCalc 计算器论坛（https://www.cncalc.org/ ）。出处见网页底部「出处和致谢」，第三方许可全文见 `THIRD_PARTY.md`。

## 仓里有什么

- `index.html`：网页版（上面那个链接就是它）。
- `originals/`：当年的原件，一个字节没改；人写的工程、工具、素材都在这里。
  - `basic/`：卡西欧 BASIC 程序（G1M）——贪吃蛇、21 点、扫雷（两版）、三国杀简版、塔防（1.0 / 1.1）；`listings/` 是解码出来的文字版。
  - `magtower/`：魔塔的 C 源码（2012-08-07）、08-09 更新的 G1A、中文字库、图块和界面素材、当年自己写的地图编辑器和图片转换器、截图。
  - `doodle/`：Doodle Jump 2013 年的工程（源码 + G1A）和 2011 年论坛上的 V0.9。
  - `3d-engine/`：3D 引擎的源码和四个 G1A（2011-12 → 2012-02）、截图。
  - `ogame/`：Ogame（2013-02）。
  - `visualfx/`：VisualFX 界面库和测试程序（2013-02），用了 diameter 的 fxChsLib 中文库。
  - `df/`：东方风格弹幕游戏的骨架（2013-04），带几张 16×16 角色小像。
  - `chinese-input/`：拼音输入法（2012-08），在 diameter 的 SimSpell 上改的。
- `2026/`：2026 年改的版本——Doodle Jump 2026 版的源码，魔塔 2026 版的图块补丁。
- `MANIFEST.md`：每个文件从哪来、原始时间、sha256。

源码是 GBK 编码。网页壳、模拟器这些 2026 年写的工具还没放进来。

> 网页壳、BASIC 解释器、SDK 模拟层由 Claude（Anthropic）在 2026 年写。本文件 created 20261031 by Claude Opus 5.5。
