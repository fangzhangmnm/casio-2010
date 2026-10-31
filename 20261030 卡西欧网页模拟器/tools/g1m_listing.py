# created 20261031 by Claude Opus 5.5 — 把 originals/basic/ 里的 G1M 原件解码成文字清单（originals/basic/listings/<名字>.txt），给人读。
# 指令名对照表用 BIDE 的 opcodes.txt（Zezombye，GPL-3.0，不放进本仓）：https://github.com/Zezombye/BIDE 的 src/main/resources/opcodes.txt
# 用法：python3 -I tools/g1m_listing.py <opcodes.txt 的路径>
# edited by Claude Opus 5.5 2026-10-31：从一次性脚本改成仓里的工具（对照表路径走参数）。
import json, os, sys, re
arr = json.loads(open(sys.argv[1], encoding='utf-8').read(), strict=False)
tab = {int(a['hex'], 16): a['text'] for a in arr if a.get('hex')}
SYM = {0x0E: '→', 0x13: '⇒', 0x10: '≤', 0x11: '≠', 0x12: '≥', 0x89: '+', 0x99: '-', 0xA9: '×', 0xB9: '÷', 0x87: '(−)', 0x0C: '◢', 0xCE: 'θ', 0xCD: 'r'}
ENT = {'&_10;': '⏨', '&black_square;': '■', '&white_square;': '□', '&small_square;': '▫', '&white_rectangle;': '▯', '&black_diamond;': '◆', '&ascii_*;': '*', '&i;': 'i', '&||;': '‖',
       '&negative_=;': '▤', '&negative_<;': '◀', '&negative_>;': '▶', '&divide_on_times;': '※', '&=;': '=', '&-;': '-', '&theta;': 'θ', '&disp;': '◢', '&r;': 'r', '&yen;': '¥', '&white_circle;': '○', '&black_circle;': '●', '&inverted_!;': '¡', '&fact;': '!', '&amp;': '&'}
MULTI = {0x7F, 0xF7, 0xF9, 0xE5, 0xE6, 0xE7}
def subs(path):
    b = open(path, 'rb').read(); i = 0x20; out = []
    while i + 0x2C <= len(b):
        typ = b[i:i+16].split(b'\0')[0]; name = b[i+0x1C:i+0x24].split(b'\0')[0]; size = int.from_bytes(b[i+0x25:i+0x29], 'big')
        if typ == b'PROGRAM': out.append((name, b[i+0x2C+10:i+0x2C+size]))
        i += 0x2C + size
    return out
def name_text(nb): return ''.join(SYM.get(c, chr(c) if 32 <= c < 127 else f'<{c:02X}>') for c in nb)
def decode(d):
    s, i = [], 0
    while i < len(d) and d[i] != 0:
        c = d[i]
        if c in MULTI:
            w = (c << 8) | d[i+1]; s.append(chr(d[i+1]) if c == 0xE7 else tab.get(w, f'<{w:04X}>')); i += 2
        elif c == 0x0D: s.append('\n'); i += 1
        else: s.append(SYM.get(c) or tab.get(c, chr(c) if 32 <= c < 127 else f'<{c:02X}>')); i += 1
    t = ''.join(s)
    t = re.sub(r'&_([A-Za-z]);', r'\1', t)
    for k, v in ENT.items(): t = t.replace(k, v)
    return t
fam = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))  # 家族根；2026-10-31 吃书重组：每个 BASIC 小游戏一个子仓
BASIC = ['20100829 贪吃蛇', '20100829 21点', '20100918 三国杀简版', '20101022 塔防', '20120629 扫雷']
for g, site in sorted((g, f'{fam}/{r}') for r in BASIC for g in os.listdir(f'{fam}/{r}/originals')):
    if not g.endswith('.G1M'): continue
    ps = subs(f'{site}/originals/{g}')
    head = f"' {g}：从 G1M 原件解码成文字（指令名照卡西欧的写法；◢ = 显示后暂停，→ = 赋值，⇒ = 条件成立就做；卡西欧自己的特殊字符换成了长得像的 Unicode：▤◀▶ = 反白的 = < >，※ = 花纹块，⏨ = 10 的乘方）\n' 共 {len(ps)} 个程序：{', '.join(name_text(n) for n, _ in ps)}\n"
    body = '\n\n'.join(f"'==== Prog \"{name_text(n)}\" ====\n{decode(d)}" for n, d in ps)
    os.makedirs(f'{site}/listings', exist_ok=True); open(f'{site}/listings/{g[:-4]}.txt', 'w', encoding='utf-8').write(head + '\n' + body + '\n')
    left = sorted(set(re.findall(r'&[^;\s]{1,20};|<[0-9A-F]{2,4}>', body)))
    print(g, len(ps), 'progs; unresolved:', left[:10])
