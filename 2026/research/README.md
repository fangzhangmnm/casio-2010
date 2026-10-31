# 2026/research · 3D 引擎科研用的测算脚本

> created 20261031 by Claude Opus 5.5。都是在电脑上跑的测算，不是计算器代码。题目和结论见 `ai-docs/20261031-3d-engine-research-ledger.md`，2012 原样版的性能报告见 `ai-docs/20261031-3d-engine-2012-performance.md`。

| 文件 | 干什么 | 要什么 |
|---|---|---|
| `count_2012_frame.py` | 2012 原样版每帧做了多少事（坐标变换、两两比较、各处画了多少点）。原件复制到临时目录插计数器，不动原件 | gcc |
| `verify_2012_formulas.py` | 用 sympy 推导核对当年手推的画墙、画地板公式（9 条） | sympy |
| `triangle_constant_depth_sim.py` | 任意斜三角形按「深度相等的斜线」画，和逐点精确透视比贴图对不对 | numpy |
| `lineart_proto.py` | 128×64 1-bit 白面黑线原型（背面剔除 + 从远到近先涂白再描边），出 `lineart-front.png`、`lineart-three-quarter.png` | numpy、Pillow |

```
python3 -I 2026/research/count_2012_frame.py
python3 -I 2026/research/verify_2012_formulas.py          # 用装了 sympy 的 Python
python3 -I 2026/research/triangle_constant_depth_sim.py   # 用装了 numpy 的 Python
python3 -I 2026/research/lineart_proto.py                 # 用装了 numpy、Pillow 的 Python
```
