# created 20261031 by Claude Opus 5.5 — 魔塔修复版的图块补丁：只补当年 MapEdit 转换时丢掉的字节。
# 当年 MapEdit.frm 的 OutputPicture 用 "0x" & Format(Hex(b), "00") 写十六进制；VB 把 "1A"–"9A" 当成凌晨 1–9 点读，
# 格式化成 "00"，这 9 种值全变成了 0x00（53 个图块、103 个字节）。这里照 MapEdit 同一套算法（pid 0–159，第 pid÷32 行、
# 第 pid%32 列，格距 17，每行 2 字节高位在左）从原图重算，只挑「原样是 0x00、原图是 0x1A–0x9A」的字节写进 fix-mapedit-hex.json；
# 有任何别的差异就停下（说明原图和当年转换用的那张对不上）。原图：originals/magtower/material/魔塔黑白 - 副本.bmp（2012-07-05）。
# 用法：python3 -I make_fix.py（在本仓根跑）
# edited by Claude Opus 5.5 2026-10-31：搬进 casio-2010，原图和 pictures.h 直接读 originals/。
import json, os, re, struct

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(os.path.dirname(HERE), '20120703 魔塔')  # 原件在兄弟子仓（2026-10-31 吃书重组）
SHEET = os.path.join(ROOT, 'originals', 'material', '魔塔黑白 - 副本.bmp')

src = open(os.path.join(ROOT, 'originals', 'src', 'pictures.h'), encoding='latin1').read()
body = re.search(r'const unsigned char pictures\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
orig = [int(t, 0) for t in re.findall(r'0[xX][0-9A-Fa-f]+|\d+', body)]
assert len(orig) == 5120

b = open(SHEET, 'rb').read()
w, h = struct.unpack_from('<ii', b, 18); off = struct.unpack_from('<I', b, 10)[0]; stride = (w * 3 + 3) & ~3
def black(x, y):  # VB 的 Picture2.Point 出界返回 -1，不算黑
    if not (0 <= x < w and 0 <= y < h): return False
    p = off + (h - 1 - y) * stride + x * 3
    return b[p:p + 3] == b'\0\0\0'

fix = []
for pid in range(160):
    for k, y in enumerate(range(pid // 32 * 17 + 1, pid // 32 * 17 + 17)):
        for x in range(2):
            v = 0
            for i in range(8):
                v = v * 2 + black(pid % 32 * 17 + 1 + 8 * x + i, y)
            ix = pid * 32 + k * 2 + x
            if v != orig[ix]:
                assert orig[ix] == 0 and v >> 4 in range(1, 10) and v & 0xF == 0xA, (pid, k, x, hex(v), hex(orig[ix]))
                fix.append([ix, v])
json.dump({'_source': '魔塔黑白 - 副本.bmp 按 MapEdit OutputPicture 同一算法重算；只含被 Format(Hex(b),"00") 吞掉的字节（原样 0x00、原图 0x1A–0x9A）。'
                      '格式：[pictures[] 里第几个字节, 正确的值]。make_fix.py 生成，created 20261031 by Claude Opus 5.5',
           'bytes': fix}, open(os.path.join(HERE, 'fix-mapedit-hex.json'), 'w', encoding='utf-8'), ensure_ascii=False)
print(len(fix), 'bytes in', len({i // 32 for i, _ in fix}), 'tiles')
