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
 *      点屏幕开始（网页在这一下里申请重力感应权限、进全屏）；左上角 X 回列表；没开重力感应时按住左半边 / 右半边转向；结束后点屏幕再来。
 *      触屏是立即模式（Touch_Hit 吃掉用过的手指）：先问 X 按钮，再问左右半边，一根手指不会同时算两样。
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
/* 横向：开了重力感应就按倾角给速度（30° = 最高速度，中间按比例）；没开就照原版用 ↑ ↓ 键（或按住屏幕左右半边）加速、松开就停 */
void MoveDoodle(float dt, int touchLeft, int touchRight)
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
	}else{
		if(IsKeyDown(KEY_CTRL_UP)||touchLeft){
			if(DoodleVX-Acc*dt>=-VxMax)DoodleVX=DoodleVX-Acc*dt;
		}else{if(DoodleVX<0)DoodleVX=0;}
		if(IsKeyDown(KEY_CTRL_DOWN)||touchRight){
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
void TextRotCenter(int gy, const char *s) { int w = TextRotWidth(s), gx = (64 - w) / 2; if (gx < 8) gx = 8; TextRot(gx, gy, s); }
/* 左上角（玩家视角）的 X 按钮：液晶 x 117–127、y 0–10 */
#define EXIT_X1 114
#define EXIT_Y2 13
void DrawExitButton()
{
	int i;
	for (i = 0; i <= 10; i++) { Bdisp_SetPoint_VRAM(117, i, 1); Bdisp_SetPoint_VRAM(127, i, 1); Bdisp_SetPoint_VRAM(117 + i, 0, 1); Bdisp_SetPoint_VRAM(117 + i, 10, 1); }
	for (i = 2; i <= 8; i++) { Bdisp_SetPoint_VRAM(117 + i, i, 1); Bdisp_SetPoint_VRAM(127 - i, i, 1); }
}
int ExitPressed() { return Touch_Hit(EXIT_X1, 0, 127, EXIT_Y2, TOUCH_DOWN) || IsKeyDown(KEY_CTRL_EXIT); }
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
		DrawExitButton();
		Bdisp_PutDisp_DD();
		if (ExitPressed()) return -1;
		if (Touch_Hit(0, 0, 127, 63, TOUCH_DOWN) || IsKeyDown(KEY_CTRL_EXE)) return 0;
	}
}
/* 一局：返回 0 = 掉下去了，-1 = 回列表 */
int Play()
{
	float dt;
	int left, right;
	Init();
	WaitKeysUp();
	Frame_Wait();
	while (1) {
		dt = Frame_Wait() / 1000000.0f;
		ReadParams();
		if (ExitPressed()) return -1;                                               /* 先问按钮，吃掉那根手指 */
		left = Touch_Hit(0, 0, 127, 31, TOUCH_DOWN | TOUCH_HELD);                   /* 再问左右半边 */
		right = Touch_Hit(0, 32, 127, 63, TOUCH_DOWN | TOUCH_HELD);
		Bdisp_AllClr_VRAM();
		MovePlate(dt);
		MoveDoodle(dt, left, right);
		CheckPlate();
		NewPlate();
		LevelSet();
		DispPlat();
		DispDoodle(DoodleX, DoodleY, DoodleTurn);
		DispScore();
		DrawExitButton();
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
		DrawExitButton();
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
