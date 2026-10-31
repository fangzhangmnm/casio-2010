# created 20261031 by Claude Opus 5.5 — 科研账 R2 / 性能报告。
# 用 sympy 核 2012 年 RenderWall / SetFloor 的手推公式（Painter.h），只做符号推导。
from sympy import symbols, simplify, solve, Eq
P, Deg, s, r, i, y = symbols('P Deg s r i y')            # P = PIXELZOOM；s = 屏幕列；r = 屏幕行
X1, Z1, X2, Z2, Y1, Y2, W, R, H = symbols('X1 Z1 X2 Z2 Y1 Y2 W R H')
u, v = symbols('u v')
dX, dZ, dY = (X2-X1)/W/R, (Z2-Z1)/W/R, (Y2-Y1)/H
ok = lambda e: simplify(e) == 0
# —— 墙：墙上贴图第 u 列的点投到第 s 列，反解 u ——
X, Z = X1 + u*dX, Z1 + u*dZ
u_sol = solve(Eq(s, 64 + P*X/Z), u)[0]
A = -Z1*s + 64*Z1 + P*X1                     # 代码：A=-Z1*Xs+64*Z1+PIXELZOOM*X1
B = dZ*s - 64*dZ - P*dX                      # 代码：B=dZ*Xs-64*dZ-PIXELZOOM*dX
print('u = A/B（透视正确的贴图列）      ', ok(u_sol - A/B))
print('换下一列：A 减 Z1、B 加 dZ          ', ok(A.subs(s, s+1) - A + Z1), ok(B.subs(s, s+1) - B - dZ))
C = P*(Z2*X1 - Z1*X2)/W/R                    # 代码：C=PIXELZOOM*(Z2*X1-Z1*X2)/W/Repeat
Zs = Z.subs(u, A/B)
print('这一列的深度 = C/B（1/z 沿列线性）  ', ok(Zs - C/B))
D = Deg - P*Y1*B/C;  E = -P*Y1*dZ/C          # 代码：D=Deg-PIXELZOOM*Y1*(...)/C，E=-PIXELZOOM*Y1*dZ/C
print('墙顶在第 D 行、每列加 E             ', ok(D - (Deg - P*Y1/Zs)), ok(D.subs(s, s+1) - D - E))
# 列内：第 r 行对应贴图第 v 行（M / MULTI）
v_true = ((Deg - r)*Zs/P - Y1)/dY
F = -C/(P*dY); G = -(Y1/dY)                  # 代码里再乘 MULTI = 2^20（定点数）
v_code = (F/B)*(r - Deg) + G                 # 代码：P=F/B；M=P*(Yrr-Deg)+G
print('列内 v 随行号线性（M += N）          ', ok(v_true - v_code))
# —— 地板：高度 Dh 的水平面，第 y 行、第 i 列 → 贴图坐标 ——
Dh, X0, Z0, dxx, dzx, dxy, dzy = symbols('Dh X0 Z0 dxx dzx dxy dzy')
t = 1/(Deg - y)
z = P*Dh*t; x = (i - 64.5)*z/P               # 地板用 64.5（像素中心）
uu, vv = symbols('uu vv')
sol = solve([Eq(x, X0 + uu*dxx + vv*dxy), Eq(z, Z0 + uu*dzx + vv*dzy)], [uu, vv])
T = 1/(dxx*dzy - dzx*dxy)                    # 代码 t=MULTI/(dxx*dzy-dzx*dxy)，这里不乘 MULTI
fA = Dh*dzy*T; fB = -T*(64.5*Dh*dzy + P*Dh*dxy); fC = T*(-X0*dzy + Z0*dxy)
fD = -Dh*dzx*T; fE = T*(64.5*Dh*dzx + P*Dh*dxx); fF = -T*(-X0*dzx + Z0*dxx)
print('地板 u = (fA·i + fB)·t + fC          ', ok(sol[uu] - ((fA*i + fB)*t + fC)))
print('地板 v = (fD·i + fE)·t + fF          ', ok(sol[vv] - ((fD*i + fE)*t + fF)))
