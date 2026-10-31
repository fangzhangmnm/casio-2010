# created 20261031 by Claude Opus 5.5 — 科研账 R3。
# R3 原型：128×64 1-bit 白面黑线（CatsUp 线稿的效果），不用深度缓冲：背面剔除 + 从远到近先涂白再描边。
import numpy as np, sys
from PIL import Image
W, H, P = 128, 64, 88.0          # 竖直视野 40°（CatsUp 默认），32/tan(20°) ≈ 88
def box(c, s):           # 中心、尺寸 → 顶点 + 面（逆时针朝外）
    x, y, z = c; a, b, d = (s[0]/2, s[1]/2, s[2]/2)
    V = [(x-a,y-b,z-d),(x+a,y-b,z-d),(x+a,y+b,z-d),(x-a,y+b,z-d),(x-a,y-b,z+d),(x+a,y-b,z+d),(x+a,y+b,z+d),(x-a,y+b,z+d)]
    F = [(0,3,2,1),(4,5,6,7),(0,1,5,4),(2,3,7,6),(1,2,6,5),(0,4,7,3)]
    return V, F
def roof(c, s, h):        # 房身顶上的三棱柱：底 4 点 + 屋脊 2 点
    x, y, z = c; a, d = s[0]/2, s[2]/2; top = y + s[1]/2
    V = [(x-a,top,z-d),(x+a,top,z-d),(x+a,top,z+d),(x-a,top,z+d),(x-a,top+h,z),(x+a,top+h,z)]
    F = [(0,1,2,3),(0,1,5,4),(3,2,5,4),(0,3,4),(1,2,5)]
    return V, F
def cylinder(c, r, h, n=16):
    x, y, z = c; V = []
    for k in range(n):
        t = 2*np.pi*k/n; V += [(x+r*np.cos(t), y-h/2, z+r*np.sin(t)), (x+r*np.cos(t), y+h/2, z+r*np.sin(t))]
    F = [(2*k, 2*((k+1)%n), 2*((k+1)%n)+1, 2*k+1) for k in range(n)]
    F += [tuple(2*k for k in range(n)), tuple(2*k+1 for k in range(n))[::-1]]
    return V, F
def scene():
    objs = [box((-1.2, 0.75, 6.0), (2.4, 1.5, 2.0)), roof((-1.2, 0.75, 6.0), (2.4, 1.5, 2.0), 0.9), cylinder((1.9, 1.0, 5.0), 0.6, 2.0), box((0.6, 0.35, 3.6), (0.7, 0.7, 0.7))]
    return objs
def render(cam_y=1.6, pitch=-0.18, yaw=0.0, outline_px=1, out='a.png', cam_x=0.0, cam_z=0.0):
    img = np.ones((H, W), np.uint8)                               # 1 = 白
    cy, sy = np.cos(yaw), np.sin(yaw); cp, sp = np.cos(pitch), np.sin(pitch)
    def cam(p):
        x, y, z = p[0]-cam_x, p[1]-cam_y, p[2]-cam_z
        x, z = cy*x - sy*z, sy*x + cy*z
        y, z = cp*y - sp*z, sp*y + cp*z
        return np.array([x, y, z])
    faces = []
    for V, F in scene():
        Vn = np.array(V, float); ctr = Vn.mean(axis=0); F2 = []
        for f in F:
            a, b, c = Vn[f[0]], Vn[f[1]], Vn[f[2]]; n = np.cross(b-a, c-a)
            F2.append(f if np.dot(n, Vn[list(f)].mean(axis=0) - ctr) > 0 else tuple(reversed(f)))
        F = F2
        C = [cam(np.array(v, float)) for v in V]
        S = [(W/2 + P*c[0]/c[2], H/2 - P*c[1]/c[2]) for c in C]
        # 每条边两侧的面
        side = {}
        for fi, f in enumerate(F):
            for k in range(len(f)):
                e = tuple(sorted((f[k], f[(k+1) % len(f)]))); side.setdefault(e, []).append(fi)
        N, front = [], []
        for f in F:
            a, b, c = (C[f[0]], C[f[1]], C[f[2]])
            n = np.cross(b-a, c-a); N.append(n/np.linalg.norm(n))
            front.append(np.dot(n, a) < 0)                         # 法线朝镜头 = 正面
        for fi, f in enumerate(F):
            if not front[fi]: continue
            edges = []
            for k in range(len(f)):
                i, j = f[k], f[(k+1) % len(f)]; e = tuple(sorted((i, j))); adj = side[e]
                other = [g for g in adj if g != fi]
                if not other: kind = 'open'
                else:
                    g = other[0]
                    if not front[g]: kind = 'outline'              # 一正一反 = 外轮廓
                    elif np.dot(N[fi], N[g]) < np.cos(np.radians(30)): kind = 'hard'
                    else: kind = 'soft'                            # 软边：正好不是外轮廓 → 不画
                edges.append((S[i], S[j], kind))
            depth = max(C[v][2] for v in f)
            faces.append((depth, [S[v] for v in f], edges))
    faces.sort(key=lambda t: -t[0])                                # 从远到近
    def fill(poly):
        xs = [p[0] for p in poly]; ys = [p[1] for p in poly]
        for y in range(max(0, int(min(ys))), min(H, int(max(ys))+2)):
            for x in range(max(0, int(min(xs))), min(W, int(max(xs))+2)):
                px, py = x+0.5, y+0.5; s = 0; ok = True
                for k in range(len(poly)):
                    (x1, y1), (x2, y2) = poly[k], poly[(k+1) % len(poly)]
                    cr = (x2-x1)*(py-y1) - (y2-y1)*(px-x1)
                    if cr != 0:
                        if s == 0: s = 1 if cr > 0 else -1
                        elif (cr > 0) != (s > 0): ok = False; break
                if ok: img[y, x] = 1
    def line(p, q, thick):
        x0, y0, x1, y1 = int(np.floor(p[0])), int(np.floor(p[1])), int(np.floor(q[0])), int(np.floor(q[1]))
        dx, dy = abs(x1-x0), -abs(y1-y0); sx, sy2 = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1); err = dx+dy
        while True:
            for ox in range(thick):
                for oy in range(thick):
                    if 0 <= x0+ox < W and 0 <= y0+oy < H: img[y0+oy, x0+ox] = 0
            if x0 == x1 and y0 == y1: break
            e2 = 2*err
            if e2 >= dy: err += dy; x0 += sx
            if e2 <= dx: err += dx; y0 += sy2
    for depth, poly, edges in faces:
        fill(poly)
        for p, q, kind in edges:
            if kind == 'soft': continue
            if kind == 'hard' and np.hypot(q[0]-p[0], q[1]-p[1]) < 2: continue   # 很短的内部边不描
            line(p, q, outline_px if kind == 'outline' else 1)
    lcd = np.where(img[..., None] == 1, np.array([200, 206, 186], np.uint8), np.array([34, 38, 32], np.uint8))
    Image.fromarray(lcd).resize((W*4, H*4), Image.NEAREST).save(out)
    return img
import os
d = os.path.dirname(os.path.abspath(__file__))
render(cam_x=0.2, cam_y=2.2, cam_z=-1.5, pitch=-0.22, yaw=0.0, out=d+'/lineart-front.png')
render(cam_x=-3.2, cam_y=3.0, cam_z=-0.5, pitch=-0.32, yaw=0.45, out=d+'/lineart-three-quarter.png')
print('ok')
