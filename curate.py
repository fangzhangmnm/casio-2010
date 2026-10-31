# created 20261031 by Claude Opus 5.5 — 策展：从 casio-2010 家族的子仓拷一份精简快照进宣发仓（博物馆的思路，不 bloat）。
# 作者 10-31：「网页源代码文档，先不用太蛋疼管理，反正就是博物馆策展的思路，你自己看着办」「反正就是策展，不要太bloat」。
# 展出：能玩的网页 + 每个作品一个展厅（原件、标签 README、来历 MANIFEST、文字版、整理文档）+ 修复工作室（模拟器源码、2026 版）。
# 不展出：内部的（科研账、测算脚本、愿望单）；3D 引擎 2026 毕业了再发（作者「毕业了发gh和cncalc」）。
# 用法：作者说「发」之前跑 python3 -I curate.py → 看 git diff → 提交 → 推 GitHub。展厅别手改，改子仓再重跑。
import os, shutil, subprocess
HERE = os.path.dirname(os.path.abspath(__file__)); FAM = os.path.dirname(HERE)
EMU = '20261030 卡西欧网页模拟器'
SKIP_HALLS = {os.path.basename(HERE), '20261031 3D 引擎 2026'}
SKIP_FILES = {'.gitignore', 'index.html'}            # 网页不进展厅（仓根是合集页、play/ 是每个游戏一页）
KEEP = {'.git', 'curate.py', 'README.md', 'CLAUDE.md', '.nojekyll'}
halls = sorted(d for d in os.listdir(FAM) if os.path.isdir(os.path.join(FAM, d, '.git')) and d not in SKIP_HALLS)
for n in os.listdir(HERE):
    if n in KEEP: continue
    p = os.path.join(HERE, n); shutil.rmtree(p) if os.path.isdir(p) else os.remove(p)
for h in halls:
    src = os.path.join(FAM, h)
    for f in filter(None, subprocess.run(['git', '-C', src, 'ls-files', '-z'], capture_output=True, check=True).stdout.decode().split('\0')):
        if os.path.basename(f) in SKIP_FILES or f.startswith(('play/', 'site/')): continue
        d = os.path.join(HERE, h, f); os.makedirs(os.path.dirname(d), exist_ok=True); shutil.copy2(os.path.join(src, f), d)
shutil.copy2(os.path.join(FAM, EMU, 'site', 'index.html'), os.path.join(HERE, 'index.html'))   # 合集页（1-bit 液晶风格）
shutil.copy2(os.path.join(FAM, EMU, 'THIRD_PARTY.md'), os.path.join(HERE, 'THIRD_PARTY.md'))
shutil.copytree(os.path.join(FAM, EMU, 'play'), os.path.join(HERE, 'play'))                    # 每个游戏一页，打开就玩
print(len(halls), '个展厅')
