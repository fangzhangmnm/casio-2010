# created 20261031 by Claude Opus 5.5 — 把魔塔的图块总表（user 2012 年画的「魔塔黑白 - 副本.bmp」，528×86，31×5 格、格距 17、蓝色格线）
# 原像素 1:1（不放大）打包成 OpenRaster（.ora），给 WeebPaint 打开改。一层，像素和 BMP 一模一样。
# user 10-31：「帮我包一下ora把魔塔的sprites推一下onedrive的weebpaint……包两个，一个是历史时间戳，一个时间戳是今天，2026版我优化一下sprites」
#            「我就是说你发像素，而不是放大的，就可以了」
# 布局照 ORA 规范 + WeebPaint 读端（src/backend/ora.ts）：mimetype 第一（不压缩），Thumbnails/thumbnail.png 最后、最长边 ≤ 256。
# edited by Claude Opus 5.5 2026-10-31：搬进 casio-2010，原图直接读 originals/。
# 用法：python3 -I 2026/magtower/make_ora.py <输出.ora> <YYYY-MM-DDTHH:MM:SS+08:00>   （zip 条目时间和文件修改时间都用这个时刻）
import datetime, os, struct, sys, zipfile, zlib

SHEET = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), 'originals', 'magtower', 'material', '魔塔黑白 - 副本.bmp')


def read_bmp(path):
    b = open(path, 'rb').read()
    w, h = struct.unpack_from('<ii', b, 18); off = struct.unpack_from('<I', b, 10)[0]
    assert struct.unpack_from('<H', b, 28)[0] == 24
    stride = (w * 3 + 3) & ~3
    rows = []
    for y in range(h):
        r = b[off + (h - 1 - y) * stride: off + (h - 1 - y) * stride + w * 3]
        rows.append(bytes(c for x in range(w) for c in (r[x * 3 + 2], r[x * 3 + 1], r[x * 3], 255)))
    return w, h, rows


def png(w, h, rows):
    def chunk(t, d): return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    raw = b''.join(b'\0' + r for r in rows)
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')


def main(out, when):
    t = datetime.datetime.fromisoformat(when)
    zt = t.timetuple()[:6]
    w, h, rows = read_bmp(SHEET)
    layer = png(w, h, rows)
    s = min(1.0, 256 / max(w, h)); tw, th = max(1, round(w * s)), max(1, round(h * s))  # 缩略图：最近邻缩小
    thumb = png(tw, th, [b''.join(rows[int(y / s)][int(x / s) * 4:int(x / s) * 4 + 4] for x in range(tw)) for y in range(th)])
    stack = (f'<?xml version="1.0" encoding="UTF-8"?>\n<image version="0.0.3" w="{w}" h="{h}" xres="72" yres="72">\n'
             f' <stack>\n  <layer name="魔塔黑白" src="data/layer0.png" x="0" y="0" opacity="1.0" visibility="visible" composite-op="svg:src-over"/>\n'
             f' </stack>\n</image>\n')
    with zipfile.ZipFile(out, 'w') as z:
        def put(name, data, comp):
            zi = zipfile.ZipInfo(name, zt); zi.compress_type = comp; z.writestr(zi, data)
        put('mimetype', b'image/openraster', zipfile.ZIP_STORED)
        put('stack.xml', stack.encode('utf-8'), zipfile.ZIP_DEFLATED)
        put('data/layer0.png', layer, zipfile.ZIP_STORED)
        put('mergedimage.png', layer, zipfile.ZIP_STORED)
        put('Thumbnails/thumbnail.png', thumb, zipfile.ZIP_STORED)
    os.utime(out, (t.timestamp(), t.timestamp()))
    print(out, os.path.getsize(out), 'bytes', w, 'x', h, 'thumb', tw, 'x', th, 'time', t.isoformat())


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])
