# created 20261031 by Claude Opus 5.5 — 数 2012 年 3D 引擎每帧做了多少事（性能报告 ai-docs/20261031-3d-engine-2012-performance.md 的数据来源）。
# 做法：把 originals/3d-engine/ 的 .h 原样复制到临时目录，只在函数入口和最里层循环插计数器，用电脑上的 gcc 编，
# 照 3DEngine.c 主循环画一帧（不读键、不显示），开局位置原地转一圈（32 个朝向），每帧数一遍。原件不动。
# 用法：python3 -I 2026/research/count_2012_frame.py   （要 gcc）
import os, re, subprocess, sys, tempfile, shutil
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SRC = os.path.join(ROOT, 'originals', '3d-engine')
C = ['T1','T2','T20','T21','T22','T23','ABC','AB','LRI','STex','SFloor','LineH','LineHmap','Tri','Pic','Wall',
     'wallcol','wallpx','wallpxw','lhpx','lhpxw','lhmpx','lhmpxw','picpx','picpxw','iter','cmp','deep','swap','draw','whl']
def fn(src, name, ctr):
    m = re.search(r'\b' + name + r'\s*\([^;{]*\)\s*\{', src); assert m, name
    return src[:m.end()] + ' C_' + ctr + '++;' + src[m.end():]
def rep(src, a, b):
    assert src.count(a) == 1, a; return src.replace(a, b)
def patch(d):
    rd = lambda f: open(os.path.join(d, f), 'rb').read().decode('latin1')
    wr = lambda f, s: open(os.path.join(d, f), 'wb').write(s.encode('latin1'))
    open(os.path.join(d, 'counters.h'), 'w').write('extern long ' + ','.join('C_' + c for c in C) + ';\n')
    p = rd('Public.h')
    for name, c in [('TransferPoint1S','T1'),('TransferPoint2S','T2'),('TransferPoint20S','T20'),('TransferPoint21S','T21'),
                    ('TransferPoint22S','T22'),('TransferPoint23S','T23'),('GetABC','ABC'),('GetAB','AB')]:
        p = fn(p, name, c)
    wr('Public.h', '#include "counters.h"\n' + p)
    s = rd('Painter.h')
    for name, c in [('SetFloor','SFloor'),('LineH_map','LineHmap'),('LineH','LineH'),('Triangle','Tri'),('RenderPicture','Pic'),('RenderWall','Wall')]:
        s = fn(s, name, c)
    s = rep(s, 'for(Xrr=Xs;Xrr<=Xe;Xrr++){', 'for(Xrr=Xs;Xrr<=Xe;Xrr++){ C_wallcol++;')
    s = rep(s, 'while(Yrr<=Ye){', 'while(Yrr<=Ye){ C_wallpx++;')
    s = rep(s, 'if((picturebuffer[p3+Y]&mark)){', 'if((picturebuffer[p3+Y]&mark)){ C_wallpxw++;')
    i = s.index('void LineH_map'); j = s.index('void LineH('); k = s.index('void Triangle')
    lm, lh = s[i:j], s[j:k]
    a = 'for(i=l;i<=r;i++)\r\n  {' if 'for(i=l;i<=r;i++)\r\n  {' in lm else 'for(i=l;i<=r;i++)\n  {'
    lm = rep(lm, a, a + ' C_lhmpx++;')
    lm = rep(lm, 'if((picturebuffer[picStart3+pB]&mark)){', 'if((picturebuffer[picStart3+pB]&mark)){ C_lhmpxw++;')
    lh = rep(lh, 'for(i=l;i<=r;i++){', 'for(i=l;i<=r;i++){ C_lhpx++;')
    lh = rep(lh, 'if((picturebuffer[picStart3+pB]&mark)){', 'if((picturebuffer[picStart3+pB]&mark)){ C_lhpxw++;')
    s = s[:i] + lm + lh + s[k:]
    s = rep(s, 'for(x=xs;x<=xe;x++){', 'for(x=xs;x<=xe;x++){ C_picpx++;')
    s = rep(s, 'if((picturebuffer[p3+pB]&mark)){', 'if((picturebuffer[p3+pB]&mark)){ C_picpxw++;')
    wr('Painter.h', s)
    d2 = rd('DataStruct.h'); d2 = fn(d2, 'SetTexture', 'STex'); d2 = fn(d2, 'LoadRenderItem', 'LRI'); wr('DataStruct.h', d2)
    r = rd('PainterSort.h')
    r = rep(r, 'if(maxZ[j]<minZ[i]) continue; else{', 'if((C_iter++,maxZ[j]<minZ[i])) continue; else{ C_cmp++;')
    r = rep(r, 'if(b<=c||d<=a) continue;', 'if(b<=c||d<=a) continue; C_deep++;')
    r = rep(r, 'flags[i]=flags[j];flags[j]=1;', 'C_swap++; flags[i]=flags[j];flags[j]=1;')
    r = rep(r, 'if(flag||flags[i]){', 'if(flag||flags[i]){ C_draw++;')
    r = rep(r, 'while(RQNumber>0){', 'while(RQNumber>0){ C_whl++;')
    wr('PainterSort.h', r)
HARNESS = r'''/* 计数用的桌面跑法：照 3DEngine.c 主循环的一帧（不读键、不显示），从开局位置原地转一圈，每帧数一遍。 */
#include <stdio.h>
#include <string.h>
#include <math.h>
long C_T1,C_T2,C_T20,C_T21,C_T22,C_T23,C_ABC,C_AB,C_LRI,C_STex,C_SFloor,C_LineH,C_LineHmap,C_Tri,C_Pic,C_Wall,
     C_wallcol,C_wallpx,C_wallpxw,C_lhpx,C_lhpxw,C_lhmpx,C_lhmpxw,C_picpx,C_picpxw,C_iter,C_cmp,C_deep,C_swap,C_draw,C_whl;
#include "Textures.h"
#include "Maps.h"
#include "Public.h"
#include "Painter.h"
#include "DataStruct.h"
#include "PainterSort.h"
#define N 31
long *ctr[N]={&C_T1,&C_T2,&C_T20,&C_T21,&C_T22,&C_T23,&C_ABC,&C_AB,&C_LRI,&C_STex,&C_SFloor,&C_LineH,&C_LineHmap,&C_Tri,&C_Pic,&C_Wall,
     &C_wallcol,&C_wallpx,&C_wallpxw,&C_lhpx,&C_lhpxw,&C_lhmpx,&C_lhmpxw,&C_picpx,&C_picpxw,&C_iter,&C_cmp,&C_deep,&C_swap,&C_draw,&C_whl};
const char *nm[N]={"T1","T2","T20","T21","T22","T23","ABC","AB","LRI","STex","SFloor","LineH","LineHmap","Tri","Pic","Wall",
     "wallcol","wallpx","wallpxw","lhpx","lhpxw","lhmpx","lhmpxw","picpx","picpxw","iter","cmp","deep","swap","draw","whl"};
int main(int argc,char**argv){
  int frames=argc>1?atoi(argv[1]):32, k, i; long sum[N]={0}, mx[N]={0}, rq=0, rqmax=0;
  PIXELZOOM=63;Deg=32;RENDERTIMER=0;WALKTIMER=0;pos_X=1.714502;pos_Y=-5.863173;pos_Z=.5;
  for(k=0;k<frames;k++){
    long before[N]; for(i=0;i<N;i++) before[i]=*ctr[i];
    Theta=2*pi*k/frames; cos_T=cosf(Theta); sin_T=sinf(Theta);
    memset(&buffer1,0,1024);memset(&buffer2,0,1024);CleanRQ();SetFloorSet(0,-pos_Z);
    w_map=3;h_map=5;Start_map=0;zoom_map=32;
    for(i=Deg;i<=63;i++)LineH_map(1,127,i);
    AddStaticWalls(0,21);AddStaticFloors(0,11);AddStaticObjects(0,7);
    rq+=RQNumber; if(RQNumber>rqmax)rqmax=RQNumber;
    Render();
    SetTexture(8);RenderPicture(64,68,1,1);
    RENDERTIMER++;
    for(i=0;i<N;i++){ long d=*ctr[i]-before[i]; sum[i]+=d; if(d>mx[i])mx[i]=d; }
    if(k==0){ FILE*f=fopen("frame0.bin","wb"); fwrite(buffer2,1,1024,f); fclose(f); }
  }
  printf("frames %d  queue avg %.1f max %ld\n",frames,(double)rq/frames,rqmax);
  for(i=0;i<N;i++) printf("%-9s avg %9.1f  max %7ld\n",nm[i],(double)sum[i]/frames,mx[i]);
  return 0;
}
'''
d = tempfile.mkdtemp(prefix='count2012-')
try:
    for f in ['Textures.h','Maps.h','Public.h','Painter.h','DataStruct.h','PainterSort.h']:
        shutil.copy(os.path.join(SRC, f), d)
    patch(d)
    open(os.path.join(d, 'harness.c'), 'w').write(HARNESS)
    subprocess.run(['gcc','-std=gnu89','-O0','-w','-fsigned-char','-include','stdlib.h','-o','harness','harness.c','-lm'], cwd=d, check=True)
    subprocess.run(['./harness', sys.argv[1] if len(sys.argv) > 1 else '32'], cwd=d, check=True)
finally:
    shutil.rmtree(d)
