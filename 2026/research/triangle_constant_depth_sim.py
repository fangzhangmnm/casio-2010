# created 20261031 by Claude Opus 5.5 — 科研账 R1。
# R1 模拟：任意斜三角形按「深度相等的斜线」画（每条线一次除法、线内只做加法），和逐点精确透视算出的贴图格比。
import numpy as np
rng = np.random.default_rng(20261031)
W, H, P, CX, CY, TEX = 128, 64, 63.0, 64.0, 32.0, 16      # 屏幕、焦距（同 2012 引擎）、贴图 16×16 棋盘格
def project(p): return np.array([CX + P*p[0]/p[2], CY - P*p[1]/p[2]])
def inside(pts, x, y):
    def e(a, b): return (b[0]-a[0])*(y-a[1]) - (b[1]-a[1])*(x-a[0])
    s = [e(pts[0],pts[1]), e(pts[1],pts[2]), e(pts[2],pts[0])]
    return (all(v >= 0 for v in s) or all(v <= 0 for v in s))
tot = mism = lines = pixels = 0; half = aff = 0; maxdev = 0.0; parallel_err = 0.0
for trial in range(300):
    V = np.c_[rng.uniform(-6, 6, 3), rng.uniform(-3, 3, 3), rng.uniform(1.5, 12, 3)]   # 三个顶点（相机空间，镜头前）
    UV = rng.uniform(0, 4*TEX, (3, 2))                                                  # 贴图坐标
    S = np.array([project(v) for v in V])
    n = np.cross(V[1]-V[0], V[2]-V[0]); d = n @ V[0]
    if abs(d) < 1e-6: continue
    # 1/z 是屏幕坐标的一次函数：1/z = a·sx + b·sy + c
    a = n[0]/(P*d); b = -n[1]/(P*d); c = (n[2] - n[0]*CX/P + n[1]*CY/P)/d
    # 屏幕点 + 深度 → 相机空间点 → 重心 → 贴图坐标（精确）
    A3 = V.T                                                                              # 3×3：列 = 顶点
    def uv_at(sx, sy, z):
        X = np.array([(sx-CX)*z/P, (CY-sy)*z/P, z]); w = np.linalg.solve(A3, X); return w @ UV
    xs = range(max(0, int(S[:,0].min())), min(W, int(S[:,0].max())+1))
    ys = range(max(0, int(S[:,1].min())), min(H, int(S[:,1].max())+1))
    # 等深线方向：a·sx + b·sy = 常数 → 方向 (b, -a)。选长轴
    xmajor = abs(b) >= abs(a)
    k = (-a/b) if xmajor else (-b/a)                       # 斜率，|k| ≤ 1
    # 检查「同一面上的等深线互相平行」：在三角形里随便取两条等深线，比方向
    T = lambda t: int(np.floor(k*t + 0.5))                 # 同一条点阵模板
    used = {}
    for x in xs:
        for y in ys:
            if not inside(S, x, y): continue
            tot += 1
            # 精确：逐点算深度
            z = 1.0/(a*x + b*y + c); ue = uv_at(x, y, z)
            # 等深线：这个点属于第 j 条线；用这条线自己的深度、在精确线上的位置算贴图
            if xmajor:
                j = y - T(x); key = j; ly = k*x + j; zl = 1.0/(a*x + b*ly + c); ul = uv_at(x, ly, zl); dev = abs(y - ly)
            else:
                j = x - T(y); key = j; lx = k*y + j; zl = 1.0/(a*lx + b*y + c); ul = uv_at(lx, y, zl); dev = abs(x - lx)
            # 对照 1：精确算法本身采样点挪半个像素（像素里换个位置采样）
            zh = 1.0/(a*x + b*(y+0.5) + c); uh = uv_at(x, y+0.5, zh)
            if (np.floor(ue/TEX*2) != np.floor(uh/TEX*2)).any(): half += 1
            # 对照 2：不做透视（PS1 式 affine）：屏幕上按重心线性插值贴图坐标
            S3 = np.c_[S, np.ones(3)].T; w2 = np.linalg.solve(S3, np.array([x, y, 1.0])); ua = w2 @ UV
            if (np.floor(ue/TEX*2) != np.floor(ua/TEX*2)).any(): aff += 1
            used.setdefault(key, 0); used[key] += 1
            maxdev = max(maxdev, dev)
            if (np.floor(ue/TEX*2) != np.floor(ul/TEX*2)).any(): mism += 1   # 落在不同的棋盘格 = 看得出的错
    lines += len(used); pixels += sum(used.values())
print(f'三角形 300 个，像素 {tot}，等深线 {lines} 条（平均每条 {pixels/max(lines,1):.1f} 个点 = 每这么多点一次除法）')
print(f'对照：精确算法采样点挪半个像素 {100*half/tot:.2f}%；不做透视（affine）{100*aff/tot:.2f}%')
print(f'贴图格和逐点精确不同的像素 {mism}（{100*mism/tot:.2f}%），像素离精确等深线最远 {maxdev:.2f} 像素')
