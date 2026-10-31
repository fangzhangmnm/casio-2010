# 20261030 卡西欧网页模拟器（公用工具链）

> created 20261031 by Claude Opus 5.5。从 casio-2010 拆出（2026-10-31 吃书重组；来源 = `20261031 casio-2010 宣发` 仓的 8057515，更早的历史在那里）。属于 casio-2010 家族（`../`，约定见 `../CLAUDE.md`）。

2026 年写的网页模拟器和工具链，家族里各作品共用（作者 10-31「你手搓的模拟器工具链可以做一些公用仓」）。时间戳 = 网页版开工 2026-10-30。

- `shell.html`：页面本体——卡西欧 BASIC 解释器、计算器外壳、触屏、重力感应、存储内存。
- `fxsdk/`：卡西欧 fx-9860G SDK + Revolution-FX 的替身（只写了用到的函数），`fx2026.h` 是 2026 版专用的扩展接口。
- `build_g1a.py`：把各作品子仓里 C 写的 Add-in 源码连同 `fxsdk/` 编成 WebAssembly（要 emsdk，环境变量 `EMSDK`）。
- `build.py`：把一切填进 `shell.html`，产出本仓的 `index.html`（约 5 秒）。原件按子仓名去兄弟文件夹读。
- `casio-font.json` / `casio-font-small.json`：从卡西欧 SDK 字符码表采的大字 / 小字。
- `tools/g1m_listing.py`：G1M 原件解码成文字清单，写进各 BASIC 子仓的 `listings/`（参数 = BIDE 的指令表）。
- `ai-docs/20261031-emulator-design.md`：模拟器的设计。`THIRD_PARTY.md`：网页里别人的东西和许可。

```
python3 -I build.py
python3 -I tools/g1m_listing.py <BIDE 的 opcodes.txt>
```

`index.html` 是产物：迭代时推私有预览（命令在家族约定里）；作者说「发」时拷进 `../20261031 casio-2010 宣发/` 再推 GitHub。
