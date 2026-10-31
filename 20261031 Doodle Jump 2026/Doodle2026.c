/* created 20261031 by Claude Opus 5.5 — Doodle Jump 2026 版，以 GWHBOB 2013-07-23 的 Doodle.c 为底。
 * 考古家规（见 CLAUDE.md）：原样版是默认，2026 版另算；两个版本都在。和原样版的不同只有这几处：
 *   1. 锁帧算 dt（user 10-31「我学了微积分之后都会锁帧算dt」）：主循环跟屏幕刷新走（Frame_Wait），位移按经过的秒数算；
 *      原版是 25 毫秒的定时器、按「每跳一次走多少」算，40 帧内容放在 60 帧屏幕上会一步长一步短。
 *      原版常数换成每秒：跳起 5.5 像素/跳 × 40 = 220 像素/秒，重力 0.2 × 40² = 320 像素/秒²，键盘加速 0.5 × 40² = 800 像素/秒²，
 *      横向最高 2 × 40 = 80 像素/秒，平台移动速度 × 40。
 *   2. 重力感应按比例（user 10-31「30度对应我的满量程」→ 同日改「15度原版量程，30度两倍封顶」）：倾 15° = 原版最快速度，
 *      倾得更多按比例继续快，到原版的 2 倍（30°）封顶；没开重力感应就照原版用 ↑ ↓ 键（最快 = 原版最快）。
 *   3. 随机数初始化（原帖 2011-04-17 GWHBOB：「我错了，一是随机数木有初始化」）：每局关卡不一样。
 *   4. 手感参数在页面上调（Param_Get），默认值就是上面这些。
 *   5. 开始画面 + 触屏（user 10-31「一开始进去的时候有一个开始画面，我之后慢慢优化，这时候你可以授权重力感应，这时候你把触屏支持给做出来」）：
 *      点屏幕开始（网页在这一下里申请重力感应权限、进全屏）；结束后点屏幕再来。EXIT 回列表。
 *      左上角原先有个 X 按钮回列表，user 10-31「doodle豪华版不要那个叉叉」去掉了（网页顶栏有主页 / 重来）。
 *   6. 跟手（user 10-31「跟着鼠标是到鼠标的coordinate就停，先试一下简单的二值控制」「惯性应该还是要有的，所以我才说先二值控制，之后再尝试比例和pid」）：
 *      没开重力感应时按住屏幕（电脑上按住鼠标），小人往手指那一列追；取代 10-31 早些的「按住左半边 / 右半边转向」。
 * 画图（图案、DispScore / DispDoodle / DispPlat）、平台生成、难度、结束流程照原版一字不改。 */
#include "fxlib.h"
#include "stdlib.h"
#include "revolution.h"
#include "fx2026.h"
#include "mathf.h"

/* 手感参数：编号 → 页面「调手感」那几格（build.py 的 params 和这里同号） */
/* 0 号「满量程倾角」退役（user 10-31 改成下面 6 / 7 两个；换新编号，免得浏览器里存的旧值混进来） */
#define P_TILT_ONE  6   /* 倾多少度 = 原版的最快速度，0.01°（默认 1500 = 15°） */
#define P_TILT_CAP  7   /* 再倾也最多快到原版的几倍，%（默认 200：按比例，倾 30° 到两倍就封顶） */
#define P_VX_MAX    1   /* 横向最高速度，像素/秒 */
#define P_ACC       2   /* 键盘横向加速度，像素/秒² */
#define P_JUMP      3   /* 起跳速度，像素/秒 */
#define P_GRAV      4   /* 重力，像素/秒² */
#define P_DEADZONE  5   /* 倾角死区，0.01°（这个角度以内算不倾） */
#define P_FOLLOW_ACC  8 /* 跟手的加速度，像素/秒²（默认 800，和键盘一样，但分开调） */
#define P_FOLLOW_VMAX 9 /* 跟手的最快速度，像素/秒（默认 80 = 原版最快） */
#define P_FOLLOW_BRAKE 10 /* 跟手提前收：1 开（默认）/ 0 = 纯二值（会在手指两边晃） */

const char pict_doodle[12][12]={
{5,5,5,5,5,5,1,1,1,1,5,5},
{5,5,5,5,5,1,0,0,0,0,1,5},
{1,5,5,5,1,0,0,0,0,0,0,1},
{1,1,1,1,0,1,0,1,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,1},
{1,1,1,1,0,0,0,0,0,0,0,1},
{1,5,5,1,0,0,0,0,0,0,0,1},
{5,5,5,1,1,1,1,1,1,1,1,1},
{5,5,5,1,0,0,0,0,0,0,0,1},
{5,5,5,1,1,1,1,1,1,1,1,1},
{5,5,5,5,1,5,5,1,5,5,1,5},
{5,5,5,1,1,5,1,1,5,1,1,5}};
const char pict_number[8][50]={
{5,1,1,1,5,5,5,1,1,5,5,1,1,1,1,5,1,1,1,5,5,5,5,1,1,1,1,1,1,1,5,5,5,1,1,1,1,1,1,1,5,1,1,1,5,5,1,1,1,5},
{1,1,1,1,1,1,1,1,1,5,1,1,1,1,1,1,1,1,1,1,5,5,1,1,1,1,1,1,1,1,5,5,1,1,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,0,1,1,1,1,1,1,5,1,1,5,5,1,1,5,5,1,1,5,1,1,1,1,1,1,5,5,5,5,1,1,5,5,5,5,5,1,1,1,1,0,1,1,1,1,0,1,1},
{1,1,0,1,1,5,5,1,1,5,5,5,5,1,1,5,5,1,1,5,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,5,5,5,1,1,5,1,1,1,5,1,1,1,1,1},
{1,1,0,1,1,5,5,1,1,5,5,1,1,1,5,5,5,5,1,1,1,1,1,1,1,5,5,5,1,1,1,1,1,1,1,5,5,1,1,5,1,1,0,1,1,5,1,1,1,1},
{1,1,0,1,1,5,5,1,1,5,1,1,5,5,5,1,5,5,1,1,1,1,1,1,1,1,5,5,1,1,1,1,0,1,1,5,5,1,1,5,1,1,0,1,1,5,5,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,5,5,1,1,1,1,1,1,1,1,1,1,1,1,5,1,1,1,5,1,1,1,1,1,5,5,1,1,5},
{5,1,1,1,5,1,1,1,1,1,1,1,1,1,1,5,1,1,1,5,5,5,5,1,1,5,1,1,1,5,5,1,1,1,5,5,1,1,5,5,5,1,1,1,5,5,1,1,5,5}};


float MAXHEIGHT;

float PlatX[50];
int PlatY[50],PlatT[50];
int PlatS,PlatN;
float DoodleX,DoodleY,DoodleVX,DoodleVY,TmpY;
float PlatV;
int NextPlat,TotalY;
int FreqFake,FreqMove;
int DoodleTurn;
long Score;
float BlankTotal=0;
float VxMax,Acc,Jump,Grav;   /* 每帧从 Param_Get 读 */

void DispScore()
{
	long tmp;
	int x,m,i,j,t;
	tmp=Score;
	x=55;
	while(tmp>0){
		m=(tmp%10)*5;
		for(i=0;i<5;i++){
			for(j=0;j<7;j++){
				t=pict_number[j][i+m];
				if(t!=5)Bdisp_SetPoint_VRAM(126-j,x+i,t);
			}
		}	
		tmp=(int)(tmp/10);
		x-=6;
	}
}
void DispDoodle(int X,int Y,int Turn)
{
	int i,j,m,n;char t;
	for(j=0;j<=11;j++){
		n=Y+11-j;
		if((n>=0)&&(n<=127))
		for(i=0;i<=11;i++){
			if(Turn==1)m=X-7+i;else m=X-i+7;
			t=pict_doodle[j][i];
			if((m>=0)&&(m<=63)&&(t!=5))Bdisp_SetPoint_VRAM(n,m,t);
		}
	}		
	//Bdisp_SetPoint_VRAM(Y,X,0);
}	
void DispPlat()
{
	int i,m,px,py;
	m=PlatS;
	for(i=1;i<=PlatN;i++){
		px=PlatX[m];py=PlatY[m];
		switch(PlatT[m]){
			case -1:
				Bdisp_DrawLineVRAM(py,px-6,py,px+1);
				Bdisp_DrawLineVRAM(py,px+3,py,px+6);
				if(py>0){Bdisp_SetPoint_VRAM(py-1,px-7,1);
				Bdisp_SetPoint_VRAM(py-1,px+7,1);
				Bdisp_SetPoint_VRAM(py-1,px,1);
				Bdisp_SetPoint_VRAM(py-1,px+2,1);}
				if(py>1){Bdisp_SetPoint_VRAM(py-2,px-7,1);
				Bdisp_SetPoint_VRAM(py-2,px+7,1);
				Bdisp_SetPoint_VRAM(py-2,px-1,1);
				Bdisp_SetPoint_VRAM(py-2,px+1,1);}
				if(py>2){Bdisp_DrawLineVRAM(py-3,px-6,py-3,px-2);
				Bdisp_DrawLineVRAM(py-3,px,py-3,px+6);}
			break;
			case 1:
				Bdisp_DrawLineVRAM(py,px-6,py,px+6);
				if(py>0){Bdisp_SetPoint_VRAM(py-1,px-7,1);
				Bdisp_SetPoint_VRAM(py-1,px+7,1);}
				if(py>1){Bdisp_SetPoint_VRAM(py-2,px-7,1);
				Bdisp_SetPoint_VRAM(py-2,px+7,1);}
				if(py>2)Bdisp_DrawLineVRAM(py-3,px-6,py-3,px+6);
			break;
			case 2:
			case 3:
				Bdisp_DrawLineVRAM(py,px-6,py,px+6);
				if(py>0){Bdisp_SetPoint_VRAM(py-1,px-7,1);
				Bdisp_SetPoint_VRAM(py-1,px+7,1);}
				if(py>1){Bdisp_SetPoint_VRAM(py-2,px-7,1);
				Bdisp_SetPoint_VRAM(py-2,px+7,1);
				Bdisp_DrawLineVRAM(py-2,px-4,py-2,px+4);}
				if(py>2)Bdisp_DrawLineVRAM(py-3,px-6,py-3,px+6);
			break;
		}
		m=(m+1)%50;
	}
}
void LevelSet()
{
	NextPlat=20;PlatV=0;FreqFake=100;FreqMove=100;
	if(Score>1500){NextPlat=20;PlatV=0.3;FreqFake=70;FreqMove=80;}
	if(Score>3000){NextPlat=15;PlatV=0.3;FreqFake=50;FreqMove=70;}
	if(Score>4500){NextPlat=15;PlatV=0.6;FreqFake=30;FreqMove=50;}
	if(Score>6000){NextPlat=20;PlatV=0.9;FreqFake=30;FreqMove=50;}
	if(Score>9000){NextPlat=20;PlatV=1.2;FreqFake=10;FreqMove=40;}
	if(Score>12000){NextPlat=20;PlatV=1.5;FreqFake=0;FreqMove=40;}
}
void NewPlate()
{
	int deltaY,i,m,Tmp_PlatN,d;
	deltaY=DoodleY-63;
	if(deltaY<=0)return;

	DoodleY=DoodleY-deltaY;
	Score=Score+deltaY;

	m=PlatS;Tmp_PlatN=PlatN;
	for(i=1;i<=PlatN;i++){
		PlatY[m]-=deltaY;
		if(PlatY[m]<0){
			PlatT[m]=0;
			PlatS=(PlatS+1)%50;
			Tmp_PlatN--;
		}
		m=(m+1)%50;
	}
	PlatN=Tmp_PlatN;
	if(PlatN<0)PlatN=0;
	
	TotalY=TotalY+deltaY;
	while((TotalY>=NextPlat)&&(PlatN<50)){
		TotalY-=NextPlat;
		PlatN++;
		PlatX[m]=rand()%49+7;
		PlatY[m]=130-TotalY;
		d=rand()%100+1;
		PlatT[m]=1;
		if(d>=FreqFake && d<FreqMove)
			if(BlankTotal+2*NextPlat<MAXHEIGHT)
				PlatT[m]=-1;
		if(d>=FreqMove)PlatT[m]=2;
		if(PlatT[m]>0)BlankTotal=0;
		else BlankTotal+=NextPlat;
		m=(m+1)%50;
	}
}
void CheckPlate()
{
	int i,m,px,py,pt,ans;
	if(DoodleVY>=0)return;
	m=PlatS;ans=-1;
	for(i=1;i<=PlatN;i++){
		px=PlatX[m];py=PlatY[m];pt=PlatT[m];
		if((pt<0)&&(abs(px-DoodleX)<=11)&&(TmpY>=py)&&(DoodleY<=py)) PlatT[m]=0;
		if((pt>0)&&(abs(px-DoodleX)<=11)&&(TmpY>=py)&&(DoodleY<=py)) ans=m;
		m=(m+1)%50;
	}
	if(ans!=-1){
		DoodleY=PlatY[ans];
		DoodleVY=Jump;
	}
}
/* 横向：开了重力感应就按倾角给速度（15° = 原版最快，中间按比例）；按着屏幕就往手指那一列追（follow = 有没有手指，tx = 手指的横坐标）；
 * 都没有就照原版用 ↑ ↓ 键加速、松开就停 */
void MoveDoodle(float dt, int follow, int tx)
{
	SENSOR_DATA s;
	if(Sensor_Read(&s)==0){
		int full=Param_Get(P_TILT_ONE,1500), cap=Param_Get(P_TILT_CAP,200), dead=Param_Get(P_DEADZONE,0), lr;
		/* 计算器是逆时针转过来拿的（液晶右边朝上），玩家的左右 = 计算器的 y 轴：往右倾 = 计算器 −y 那头往下沉 = 加速度计 y 读到正值。
		 * 倾角 = asin(gy / |g|)，单位 0.01°。平放、斜拿、竖着像方向盘那样转都是这一个式子。 */
		{ float n=sqrtf((float)s.gx*s.gx+(float)s.gy*s.gy+(float)s.gz*s.gz), v=n>0 ? s.gy/n : 0;
		  if(v>1)v=1; if(v<-1)v=-1;
		  lr=(int)(asinf(v)*18000.0f/3.14159265f); }
		if(lr>-dead && lr<dead) lr=0; else lr += lr>0 ? -dead : dead;
		if(full<=dead) full=dead+1;
		DoodleVX = VxMax * (float)lr / (float)(full-dead);
		if(DoodleVX>VxMax*cap/100.0f)DoodleVX=VxMax*cap/100.0f;
		if(DoodleVX<-VxMax*cap/100.0f)DoodleVX=-VxMax*cap/100.0f;
	}else if(follow && !IsKeyDown(KEY_CTRL_UP) && !IsKeyDown(KEY_CTRL_DOWN)){
		/* 二值控制：速度是状态（有惯性），控制量是加速度，只取全力的两档。
		 * 纯二值（手指在右就全力往右，在左就全力往左）没有阻尼，会在手指两边一直晃，默认 ±4 像素。
		 * 提前收（user 10-31「我希望几乎还是二值的手感，要不还差4像素的时候开始收。震荡收的住吗」）：剩下的距离不够全力刹车滑行了
		 * （v² / 2a，再加这一帧要走的），就全力刹；全速 80 时刹车距离 = 80² / (2×800) = 4 像素，慢的时候刹车点自动更近。
		 * 到了（差不到半像素、速度小于一帧能改的量）就定住。60 帧模拟：冲过头 ≤ 0.44 像素，30 像素外 0.43 秒停稳；追着挪动的手指时落后 ≤ 1.8 像素。
		 * 手指一抬就回到下面原版的规矩：没按键 = 立刻停。下一步试比例 / PID。 */
		float a=Param_Get(P_FOLLOW_ACC,800), vmax=Param_Get(P_FOLLOW_VMAX,80), e=tx-DoodleX, dv, ae, av;
		if(a<1)a=1;
		dv=a*dt; ae=e<0?-e:e; av=DoodleVX<0?-DoodleVX:DoodleVX;
		if(!Param_Get(P_FOLLOW_BRAKE,1)) DoodleVX += (tx>=DoodleX ? a : -a)*dt;
		else if(ae<=0.5f && av<=dv) DoodleVX=0;                                       /* 到了：定住 */
		else if(DoodleVX*e>0 && DoodleVX*DoodleVX/(2*a)+av*dt>=ae)                   /* 刹车距离够不着了：全力收 */
			DoodleVX = av<=dv ? 0 : DoodleVX-(DoodleVX>0?dv:-dv);
		else DoodleVX += e>0 ? dv : -dv;                                                /* 全力往手指那边 */
		if(DoodleVX>vmax)DoodleVX=vmax;
		if(DoodleVX<-vmax)DoodleVX=-vmax;
	}else{
		if(IsKeyDown(KEY_CTRL_UP)){
			if(DoodleVX-Acc*dt>=-VxMax)DoodleVX=DoodleVX-Acc*dt;
		}else{if(DoodleVX<0)DoodleVX=0;}
		if(IsKeyDown(KEY_CTRL_DOWN)){
			if(DoodleVX+Acc*dt<=VxMax)DoodleVX=DoodleVX+Acc*dt;
		}else{if(DoodleVX>0)DoodleVX=0;}
	}

	DoodleX=DoodleX+DoodleVX*dt;
	if(DoodleVX<0)DoodleTurn=1;
	if(DoodleVX>0)DoodleTurn=2;
	if(DoodleX<-4)DoodleX=DoodleX+71;
	if(DoodleX>=67)DoodleX=DoodleX-71;

	TmpY=DoodleY;
	DoodleY=DoodleY+DoodleVY*dt;
	DoodleVY=DoodleVY-Grav*dt;
}
void MovePlate(float dt)
{
	int i,m;
	float v=PlatV*40*dt;   /* 原版每跳 PlatV 像素，一秒 40 跳 */
	m=PlatS;
	for(i=1;i<=PlatN;i++){
		if(PlatT[m]==2){
			if(PlatX[m]-v>=7)PlatX[m]=PlatX[m]-v;
			else{PlatX[m]=7;PlatT[m]=3;}
		}else if(PlatT[m]==3){
			if(PlatX[m]+v<=56)PlatX[m]=PlatX[m]+v;
			else{PlatX[m]=56;PlatT[m]=2;}
		}
		m=(m+1)%50;
	}
}
void ReadParams()
{
	VxMax=Param_Get(P_VX_MAX,80); Acc=Param_Get(P_ACC,800); Jump=Param_Get(P_JUMP,220); Grav=Param_Get(P_GRAV,320);
	if(Grav<1)Grav=1;
	MAXHEIGHT=Jump*Jump/(2*Grav);   /* 一跳能到多高（原版是把每跳的速度加起来，这里是连续的） */
}
int Init()
{
	int i;
	ReadParams();
	srand(RTC_GetTicks());
	for(i=0;i<24;i++){PlatX[i]=rand()%49+7;PlatY[i]=i*5;PlatT[i]=1;}
	PlatS=0;PlatN=24;
	DoodleX=PlatX[0];DoodleY=2;DoodleVX=0;DoodleVY=0;
	TotalY=0;
	Score=0;
	DoodleTurn=1;
	LevelSet();
}
/* ---- 2026：开始画面、按钮、结束画面 ----
 * 玩家视角的坐标（计算器逆时针拿）：横坐标 X 0–63（左→右）= 液晶 y，高度 Y 0–127（下→上）= 液晶 x。触点给的是液晶坐标。 */
/* 玩家视角写一行小字：先用 PrintMini 正着写在显存左上角，再逐点转过来搬到位置，搬完擦掉角落。
 * gx = 横坐标（≥ 8，免得和角落的草稿区重叠），gy = 文字顶端的高度。返回字宽。 */
int TextRot(int gx, int gy, const char *s)
{
	int x, y, w = 0;
	DISPBOX box;
	box.left = 0; box.top = 0; box.right = 127; box.bottom = 5;
	Bdisp_AreaClr_VRAM(&box);
	PrintMini(0, 0, (const unsigned char *)s, MINI_OVER);
	for (x = 0; x < 128; x++) for (y = 0; y < 5; y++) if (Bdisp_GetPoint_VRAM(x, y)) { if (x + 1 > w) w = x + 1; }
	for (x = 0; x < w; x++) for (y = 0; y < 5; y++)
		if (Bdisp_GetPoint_VRAM(x, y) && gx + x <= 63) Bdisp_SetPoint_VRAM(gy - y, gx + x, 1);
	Bdisp_AreaClr_VRAM(&box);
	return w;
}
int TextRotWidth(const char *s) /* 先量一下再居中：写到角落、量完擦掉 */
{
	int x, y, w = 0;
	DISPBOX box;
	box.left = 0; box.top = 0; box.right = 127; box.bottom = 5;
	Bdisp_AreaClr_VRAM(&box);
	PrintMini(0, 0, (const unsigned char *)s, MINI_OVER);
	for (x = 0; x < 128; x++) for (y = 0; y < 5; y++) if (Bdisp_GetPoint_VRAM(x, y) && x + 1 > w) w = x + 1;
	Bdisp_AreaClr_VRAM(&box);
	return w;
}
/* 跟手的手指：还没被吃掉、按着的第一根（电脑上按住鼠标也算一根），吃掉它，*tx = 它在玩家视角的横坐标（= 液晶 y）。没有返回 0。 */
int TouchFollow(int *tx)
{
	TOUCH_POINT p[TOUCH_MAX];
	int i, n = Touch_Read(p, TOUCH_MAX);
	for (i = 0; i < n; i++) if (!p[i].used && (p[i].state & (TOUCH_DOWN | TOUCH_HELD))) {
		Touch_Hit(p[i].x, p[i].y, p[i].x, p[i].y, TOUCH_DOWN | TOUCH_HELD);
		*tx = p[i].y;
		return 1;
	}
	return 0;
}
void TextRotCenter(int gy, const char *s) { int w = TextRotWidth(s), gx = (64 - w) / 2; if (gx < 8) gx = 8; TextRot(gx, gy, s); }
int ExitPressed() { return IsKeyDown(KEY_CTRL_EXIT); }
void WaitKeysUp() { while (IsKeyDown(KEY_CTRL_EXE) || IsKeyDown(KEY_CTRL_EXIT)) Frame_Wait(); }

/* 开始画面（user 以后自己改）：返回 0 = 开始，-1 = 回列表 */
int TitleScreen()
{
	float y = 40, vy = 0, dt;
	WaitKeysUp();
	Frame_Wait();
	while (1) {
		dt = Frame_Wait() / 1000000.0f;
		ReadParams();
		vy -= Grav * dt; y += vy * dt; if (y < 40) { y = 40; vy = Jump * 0.7f; }   /* 小人原地跳 */
		Bdisp_AllClr_VRAM();
		TextRotCenter(122, "DOODLE JUMP");
		TextRotCenter(114, "2026");
		TextRotCenter(26, "TAP TO START");
		TextRotCenter(18, "TILT TO MOVE");
		Bdisp_DrawLineVRAM(38, 20, 38, 44);                                         /* 小人脚下的平台 */
		DispDoodle(32, (int)y, 1);
		Bdisp_PutDisp_DD();
		if (ExitPressed()) return -1;
		if (Touch_Hit(0, 0, 127, 63, TOUCH_DOWN) || IsKeyDown(KEY_CTRL_EXE)) return 0;
	}
}
/* 一局：返回 0 = 掉下去了，-1 = 回列表 */
int Play()
{
	float dt;
	int follow, tx = 0;
	Init();
	WaitKeysUp();
	Frame_Wait();
	while (1) {
		dt = Frame_Wait() / 1000000.0f;
		ReadParams();
		if (ExitPressed()) return -1;
		follow = TouchFollow(&tx);                                                  /* 按着的手指 = 跟手的目标 */
		Bdisp_AllClr_VRAM();
		MovePlate(dt);
		MoveDoodle(dt, follow, tx);
		CheckPlate();
		NewPlate();
		LevelSet();
		DispPlat();
		DispDoodle(DoodleX, DoodleY, DoodleTurn);
		DispScore();
		Bdisp_PutDisp_DD();
		if (DoodleY < -12) return 0;
	}
}
/* 结束画面：返回 0 = 再来，-1 = 回列表 */
int GameOver()
{
	float t = 0;
	char buf[24];
	sprintf(buf, "SCORE %ld", Score);
	WaitKeysUp();
	Frame_Wait();
	while (1) {
		t += Frame_Wait() / 1000000.0f;
		Bdisp_AllClr_VRAM();
		TextRotCenter(90, "GAME OVER");
		TextRotCenter(78, buf);
		TextRotCenter(26, "TAP TO RETRY");
		Bdisp_PutDisp_DD();
		if (ExitPressed()) return -1;
		if (t > 0.5f && (Touch_Hit(0, 0, 127, 63, TOUCH_DOWN) || IsKeyDown(KEY_CTRL_EXE))) return 0;   /* 半秒内的点按不算，免得手指还没抬就重开 */
	}
}
int AddIn_main(int isAppli, unsigned short OptionNum)
{
	Change_Contrast(216);
	ReadParams();
	if (TitleScreen() < 0) return 1;
	while (1) {
		if (Play() < 0) return 1;
		if (GameOver() < 0) return 1;
	}
	return 1;
}
