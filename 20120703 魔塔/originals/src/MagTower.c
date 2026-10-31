#include "fxlib.h"
#include "stdarg.h"
#include "timer.h"
#include "stdlib.h"

#include "pictures.h"
#include "dialog.h"
#include "monsters.h"
#include "map.h"

//system data
int CHEATMODE;
int GAME;

int mapwidth;
int mapheight;
int mapnumber;
int viewportx;
int viewporty;
int viewportf;
const screenwidth=96;
const screenheight=64;
const directions[5][2]={{0,1},{0,-1},{-1,0},{1,0},{0,0}};
int clock;
typedef struct{
	int pid;
	int life;
	int crood;	//0-none	1-map	2-map16	3-screen
	int x;
	int y;
	int width;
	int height;
} sprite;
sprite sprites[10];

//game data
unsigned char map[22][11][11];
typedef struct{
	int key[3];
	int hoe;
	int cross;
	int book;
	int elevator;
	int maxfloor;
	int plot;
} inventory;
typedef struct{
	int f;
	int x;
	int y;
	int t;
	int c;

	int hp;
	int atk;
	int dec;
	int lv;
	int gold;
	int exp;

	inventory inv;
} character;
character player;



unsigned short * chr_to_fnt(const char *c,unsigned short *f){int i,len = strlen(c);for(i=0; i<len ;++i)f[i] = c[i];f[i]=0;return f;}
int fh_HZK = 0;
int open_chs_font(){unsigned short file_name[16];chr_to_fnt("\\\\fls0\\HZK12.df",file_name);fh_HZK = Bfile_OpenFile(file_name,_OPENMODE_READ);return fh_HZK;}

int getpictureHZK(unsigned char* mat,unsigned char c1,unsigned char c2)
{
	int sec,pot; 
	sec = c1-0xa0;
	pot = c2-0xa0;
	Bfile_ReadFile(fh_HZK,mat,24,(94*(sec-1)+(pot-1))*24);
//mat[12][2]
}
int getpicture(unsigned char* mat,int pid)
{
	int i,j;
	j=pid<<5;
	for(i=0;i<32;i++)mat[i]=pictures[j++];
}
int putpicture(unsigned char* mat,int xs,int ys,int w,int h)
{
	int xe,ye,i,j,x,y,ii,jj,p;
	unsigned char mask;
	xe=xs+w-1;if(xe>127)xe=127;
	ye=ys+h-1;if(ye>63)ye=63;
	ii=jj=0;
	if(xs<0){ii=-xs;xs=0;}
	if(ys<0){jj=-ys;ys=0;}
	if(w%8)w=(w>>3)+1;else w=w>>3;
	i=ii;
	for(x=xs;x<=xe;x++){
		j=jj;
		for(y=ys;y<=ye;y++){
			mask=128>>(i%8);
			p=(j*w)+(i>>3);
			Bdisp_SetPoint_VRAM(x,y,mat[p]&mask?1:0);
			j++;
		}
	i++;
	}
}
int printf5(int x,int y,char * msg,...)
{
	unsigned char c1,c2;
	unsigned char buf[128];
	va_list  arg_list;

	va_start	(arg_list,msg);
	vsprintf	((char*)buf,msg,arg_list);
	va_end		(arg_list);
	
	PrintMini (x,y,buf,0);

}
int printf7(int x,int y,char * msg,...)
{
	unsigned char c1,c2;
	unsigned char buf[128];
	va_list  arg_list;

	va_start	(arg_list,msg);
	vsprintf	((char*)buf,msg,arg_list);
	va_end		(arg_list);
	
	PrintXY (x,y,buf,0);

}
int printf12 (int x,int y,unsigned char * msg,...)
{
	unsigned char c1,c2;
	unsigned char buf[128],*p=buf;
	unsigned char mat[12][2];
	int lan=0;
	va_list  arg_list;

	va_start	(arg_list,msg);
	vsprintf	((char*)buf,(char*)msg,arg_list);
	va_end		(arg_list);

	while (*p)
	{
		if ((c1=*p) & 0x80)
		{
			if(lan==2)x++;
			p++;c2 = *p++;
			getpictureHZK (mat,c1,c2);
			putpicture(mat,x,y,12,12);
			x+=12;
			lan=1;
		}
		else
		{
			char s[2];
			DISPBOX box;
			s[0]=c1;s[1]='\0';
			p++;
			box.left = x;box.top = y;box.right = x+6;box.bottom = y+11; 
			Bdisp_AreaClr_VRAM (&box);
			PrintXY (x,y+3,(unsigned char*)s,0);
			x+=6;
			lan=2;
		}
	}
}

int msgbox(unsigned char * msg,...)
{
	unsigned char c1,c2;
	unsigned char buf[128],*p;
	unsigned char mat[12][2];
	unsigned char lan,length[5],maxlength,i;
	int x,y;
	va_list  arg_list;
	DISPBOX box;

	va_start	(arg_list,msg);
	vsprintf	((char*)buf,(char*)msg,arg_list);
	va_end		(arg_list);

	lan=0;
	p=buf;
	i=0;length[i]=0;
	maxlength=0;
	while (*p){
		if ((c1=*p) & 0x80){
			if(lan==2)length[i]++;
			length[i]+=12;
			lan=1;
			p+=2;
		}else{
			if(c1=='\n' && i<4){
				if(length[i]>maxlength)maxlength=length[i];
				i++;length[i]=0;
				lan=0;
			}else{
				length[i]+=6;
				lan=2;
			}
			p++;
		}
	}
	if(length[i]>maxlength)maxlength=length[i];
	i++;
	box.left = 63-maxlength/2;box.top = 31-i*6;box.right = 65+maxlength/2;box.bottom = 33+i*6; 
	Bdisp_AreaClr_VRAM (&box);	
	y=32-i*6;
	lan=0;
	p=buf;
	i=0;x=64-length[i]/2;
	while (*p)
	{
		if ((c1=*p) & 0x80)
		{
			if(lan==2)x++;
			p++;c2 = *p++;
			getpictureHZK (mat,c1,c2);
			putpicture(mat,x,y,12,12);
			x+=12;
			lan=1;
		}
		else
		{
			if(c1=='\n' && i<4){
				i++;x=64-length[i]/2;y+=12;
				lan=0;
			}else{
				char s[2];
				s[0]=c1;s[1]='\0';
				box.left = x;box.top = y;box.right = x+6;box.bottom = y+11; 
				Bdisp_AreaClr_VRAM (&box);
				PrintXY (x,y+3,(unsigned char*)s,0);
				x+=6;
				lan=2;
			}
			p++;
		}
	}
	return maxlength;
}
int menu(int tips,int rows,int maxlength)
{
	int i,y0,t;
	unsigned int key;

	i=0;key=0;y0=32+tips*6-rows*6;
	Bdisp_AreaReverseVRAM(63-maxlength/2,y0+i*12,65+maxlength/2,y0+i*12+12);
	while(key!=KEY_CTRL_EXE){
		GetKey(&key);
		Bdisp_AreaReverseVRAM(63-maxlength/2,y0+i*12,65+maxlength/2,y0+i*12+12);
		switch(key){
		case KEY_CTRL_UP:case KEY_CTRL_LEFT:
			i=(i+rows-1)%rows;
		break;
		case KEY_CTRL_DOWN:case KEY_CTRL_RIGHT:
			i=(i+1)%rows;
		break;
		}
		t=key-KEY_CHAR_1;
		if(t>=0&&t<rows){i=t;key=KEY_CTRL_EXE;}
		Bdisp_AreaReverseVRAM(63-maxlength/2,y0+i*12,65+maxlength/2,y0+i*12+12);
	}
	return i;
}
int fixviewport(int x,int y,int f)
{
	viewportx=x-screenwidth/2;
	viewporty=y-screenheight/2;
	if(viewportx<0)viewportx=0;
	if(viewportx>16*mapwidth-screenwidth)viewportx=16*mapwidth-screenwidth;
	if(viewporty<0)viewporty=0;
	if(viewporty>16*mapheight-screenheight)viewporty=16*mapheight-screenheight;
	viewportf=f;
}
int getpid(int i)
{
	if(i==231)i=229;
	if(i>=200)return i-200+0+(clock?32:0);
	if(i>=100)return i-100+64;
	if(i>=50)return (i-50)*2+96+(clock?1:0);
	if(i==10||i==13||i==15)return i+128+(clock?1:0);
	return i+128;
}
int putmap()
{
	int i,j;
	int xs,xe,ys,ye;
	unsigned char mat[32];

	xs=viewportx>>4;
	ys=viewporty>>4;
	if(viewportx%16==0) xe=xs+5; else xe=xs+6;
	if(viewporty%16==0) ye=ys+3; else ye=ys+4;

	for(i=xs;i<=xe;i++)
	for(j=ys;j<=ye;j++){
		getpicture(mat,getpid(map[viewportf][i][j]));
		putpicture(mat,(i<<4)-viewportx,(j<<4)-viewporty,16,16);
		//printf7((i<<4)-viewportx,(j<<4)-viewporty,"%d",map[viewportf][i][j]);
	}
}
int log10(int n)
{
	if(n<0)n*=-10;

	if(n>=10000)return 5;
	if(n>=1000)return 4;
	if(n>=100)return 3;
	if(n>=10)return 2;
	return 1;
}
int puthud()
{
	putpicture(hud_left,96,0,32,64);
	
	if(player.x<=2)	printf7(78,0,"%02dF",player.f);
	else			printf7(0,0,"%02dF",player.f);
	//printf12(97,1,(unsigned char*)"勇者");
	if(log10(player.lv)==1)	printf7(121,6,"%d",player.lv);
	else				printf5(120,6,"%2d",player.lv);
	if(log10(player.hp)<=4)
		printf7(120-4*log10(player.hp),13,"%d",player.hp);
	else
		printf5(104,14,"%d",player.hp);
	printf7(120-4*log10(player.atk),21,"%d",player.atk);
	printf7(120-4*log10(player.dec),29,"%d",player.dec);
	printf7(120-4*log10(player.gold),37,"%d",player.gold);
	printf7(120-4*log10(player.exp),45,"%d",player.exp);
	
	printf5(101,57,"%02d",player.inv.key[0]);
	printf5(110,57,"%02d",player.inv.key[1]);
	printf5(119,57,"%02d",player.inv.key[2]);
}
int putplayer()
{
	int pid,x,y;
	unsigned char mat[32];

	if(player.f!=viewportf)return 1;
	pid=96+player.t*2;
	x=16*player.x-viewportx;
	y=16*player.y-viewporty;
	if(player.c==0){
		pid+=clock?1:0;
	}else{
		pid+=player.c>2?0:1;
		x+=directions[player.t][0]*4*(player.c-1);
		y+=directions[player.t][1]*4*(player.c-1);
	}
	getpicture(mat,pid);
	putpicture(mat,x,y,16,16);
}
int putsprites()
{
	int i;int mat[32];
	for(i=0;i<10;i++) if(sprites[i].life>0){
		getpicture(mat,sprites[i].pid);
		switch(sprites[i].crood){
		case 1:
			putpicture(mat,16*sprites[i].x-viewportx,16*sprites[i].y-viewporty,sprites[i].width,sprites[i].height);
		break;case 2:
			putpicture(mat,sprites[i].x-viewportx,sprites[i].y-viewporty,sprites[i].width,sprites[i].height);
		break;case 3:
			putpicture(mat,sprites[i].x,sprites[i].y,sprites[i].width,sprites[i].height);
		}
	}
}
int putsprite(int pid,int life,int crood,int x,int y,int width,int height)
{
	int i,min,p;
	p=0;min=sprites[0].life;
	for(i=1;i<10;i++) if(sprites[i].life<min){ p=i; min=sprites[i].life; }
	sprites[p].pid=pid; sprites[p].life=life; sprites[p].crood=crood; sprites[p].x=x; sprites[p].y=y; sprites[p].width=width; sprites[p].height=height;
	return p;
}
int delsprite(int id)
{
	if(id<0||id>9)return 0;
	sprites[id].life=0;
}
int putall()
{
	putmap();
	putplayer();
	putsprites();
	puthud();
	Bdisp_PutDisp_DD();
}
int timer1()
{
	int i;
	clock=!clock;
	putall();
	for(i=0;i<10;i++)if(sprites[i].life>0&&sprites[i].life<30000)sprites[i].life--;
}
int pause()
{
	KillTimer(ID_USER_TIMER1);
	while( IsKeyDown(KEY_CTRL_UP) || IsKeyDown(KEY_CTRL_DOWN) || IsKeyDown(KEY_CTRL_LEFT) || IsKeyDown(KEY_CTRL_RIGHT));
}
int start()
{
	SetTimer(ID_USER_TIMER1,400,timer1);
}

int dialog(int n,unsigned char* a[])
{
	int i;
	unsigned int key;

	key=0;
	for(i=0;i<n&&key!=KEY_CTRL_EXIT;i++){
		putall();
		printf5(0,59,"[EXIT] to skip");
		msgbox(a[i]);
		GetKey(&key);
	}
	putall();
}
int fight(int id)
{
	unsigned int key;
	int hp,tmp;
	unsigned char mat[32];

	putpicture(hud_fight,0,0,128,64);

	hp=monsters0[id].hp;

	clock=0;getpicture(mat,getpid(50));putpicture(mat,4,4,16,16);
	printf12(4,22,"勇者");
	printf7(11,36,"%-6d",player.hp);
	printf7(11,44,"%d",player.atk);
	printf7(11,52,"%d",player.dec);

	getpicture(mat,getpid(200+id));putpicture(mat,76,4,16,16);
	printf12(76,22,"%s",monsters0[id].name);
	printf7(83,36,"%-6d",hp);
	printf7(83,44,"%d",monsters0[id].atk);
	printf7(83,52,"%d",monsters0[id].dec);

	Bdisp_PutDisp_DD();Sleep(400);

	tmp=0;
	if(id==12)tmp=100;
	if(id==13)tmp=300;
	if(id==23)tmp=player.hp/3;
	if(id==25)tmp=player.hp/4;

	if(tmp!=0){
		clock=1;getpicture(mat,getpid(50));putpicture(mat,4,4,16,16);
		if(log10(tmp)>4) printf5(23,8,"-%d",tmp); else printf7(23,8,"-%-4d",tmp);
		Bdisp_PutDisp_DD();Sleep(200);
		player.hp-=tmp;
		printf7(11,36,"%-6d",player.hp);
		clock=0;getpicture(mat,getpid(200+id));putpicture(mat,76,4,16,16);
		Bdisp_PutDisp_DD();Sleep(200);
		printf7(23,8,"     ",tmp);
	}
	while(player.hp>0&&hp>0){
		if(player.hp>0){
			tmp=player.atk-monsters0[id].dec;if(tmp<0)tmp=0;
			clock=1;getpicture(mat,getpid(200+id));putpicture(mat,76,4,16,16);
			printf7(95,8,"-%-4d",tmp);
			Bdisp_PutDisp_DD();Sleep(200);
			hp-=tmp;if(hp<0)hp=0;
			printf7(83,36,"%-6d",hp);
			clock=0;getpicture(mat,getpid(50));putpicture(mat,4,4,16,16);
			Bdisp_PutDisp_DD();Sleep(200);
			printf7(95,8,"     ");
		}
		if(hp>0){
			tmp=monsters0[id].atk-player.dec;if(tmp<0)tmp=0;
			clock=1;getpicture(mat,getpid(50));putpicture(mat,4,4,16,16);
			printf7(23,8,"-%-4d",tmp);
			Bdisp_PutDisp_DD();Sleep(200);
			player.hp-=tmp;if(player.hp<0)player.hp=0;
			printf7(11,36,"%-6d",player.hp);
			clock=0;getpicture(mat,getpid(200+id));putpicture(mat,76,4,16,16);
			Bdisp_PutDisp_DD();Sleep(200);
			printf7(23,8,"     ",tmp);
		}
	}
	if(player.hp>0){
		msgbox("战胜%s！\n获得%d金钱！\n获得%d\xBE\xAD\xD1\xE9！",monsters0[id].name,monsters0[id].gold,monsters0[id].exp);
		player.gold+=monsters0[id].gold;
		player.exp+=monsters0[id].exp;
		GetKey(&key);
	}
}
int damageestimate(int id)
{
	int t,dam;
	if(player.atk<=monsters0[id].dec)return -1;
	dam=0;
	if(id==12)dam=100;
	if(id==13)dam=300;
	if(id==23)dam=player.hp/3;
	if(id==25)dam=player.hp/4;
	t=(int)((monsters0[id].hp-1)/(player.atk-monsters0[id].dec));
	if(monsters0[id].atk>player.dec)dam+=t*(monsters0[id].atk-player.dec);
	return dam;
}
int move()
{
	for(player.c=2;player.c<=4;player.c++){
		fixviewport(		player.x*16+8+directions[player.t][0]*4*(player.c-1),
					player.y*16+8+directions[player.t][1]*4*(player.c-1),
					player.f);
		putall();
		Sleep(50);
	}
	player.x+=directions[player.t][0];
	player.y+=directions[player.t][1];
	player.c=0;
	fixviewport(player.x*16+8,player.y*16+8,player.f);
	putall();
}
int elevatecheck(int floor)
{
	int i,b,x,y,f,x0,y0;
	if(!player.inv.elevator)return 0;
	b=0;
	for(i=0;i<5;i++){
		f=player.f;
		x=player.x+directions[i][0];
		y=player.y+directions[i][1];
		if(x>=0&&x<mapwidth&&y>=0&&y<mapheight&&f>=0&&f<mapnumber)
			if(map[f][x][y]==2 || map[f][x][y]==3) b=1;
	}
	if(!b)return 0;
	if(floor<0||floor>player.inv.maxfloor)return 0;
	x0=y0=-1;
	for(x=0;x<mapwidth;x++)for(y=0;y<mapwidth;y++)if(map[floor][x][y]==2||(floor==0&&map[floor][x][y]==3)){x0=x;y0=y;}
	if(x0==-1)return 0;
	player.f=floor;
	player.x=x0;
	player.y=y0;
	
	fixviewport(player.x*16+8,player.y*16+8,player.f);
	putall();
	return 1;
}
int beginning()
{
	int p,x;

	fixviewport(player.x*16+8,player.y*16+8,player.f);
	putall();
	Sleep(500);
	pause();
	dialog(25,dialog_beginning2);

	player.inv.key[0]+=1;
	player.inv.key[1]+=1;
	player.inv.key[2]+=1;
	start();
	map[0][5][8]=0;
	p=-1;
	for(x=0;x<4;x++){
		if(p!=-1)delsprite(p);
		p=putsprite(getpid(56),30000,2,5*16-4*x,8*16,16,16);
		putall();
		Sleep(100);
	}
	delsprite(p);
	map[0][4][8]=56;
	player.inv.plot=1;
}
int ending()
{

}
int check1(int f,int x,int y)
{

	int i;unsigned int key;
	int j;
	int p;
	int dam;
	int gold,atk,dec,hp,shop,gold2,lv;

	if(x<0||x>=mapwidth||y<0||y>=mapheight||f<0||f>=mapnumber)return 0;
	i=map[f][x][y];
	//floor&stairs
		if(i==0||i==2||i==3)return 1;

	//doors
		if(4<=i&&i<=7){
			if(i!=4){
				if(player.inv.key[i-5]<=0){
					pause();
					putall();
					msgbox("没有%c钥匙！",(unsigned char)(i-5+65));
					GetKey(&key);
					start();
					return 0;
				}else{
					player.inv.key[i-5]--;
				}
			}
			map[f][x][y]=0;
			//pause();
			p=-1;
			for(j=1;j<4;j++){
				if(p!=-1)delsprite(p);
				p=putsprite(getpid(i),30000,2,16*x,16*y+4*j,16,16-4*j);
				putall();
				Sleep(100);
			}
			delsprite(p);
			//putall();
			//start();
			return 1;
		}
	//monsters
		if(i>=200){
			pause();

			//boss
			if(i==226&&player.f==16) dialog(4,dialog_boss1);
			if(i==229&&player.f==19) dialog(4,dialog_boss2);
			if(i==229&&player.f==21) dialog(1,dialog_boss3);

			dam=damageestimate(i-200);
			if(dam==-1||dam>=player.hp){
				putall();
				msgbox("你打不过！");
				GetKey(&key);
			}else{
				putall();
				if(i==229&&player.f==21) fight(31);
				else fight(i-200);
				//boss
				if(i==226&&player.f==16) dialog(2,dialog_bossko1);
				if(i==229&&player.f==19) dialog(1,dialog_bossko2);
				if(i==229&&player.f==21){
					dialog(2,dialog_bossko3);
					start();
					Sleep(1000);
					map[f][x][y]=0;
					Sleep(1500);
					map[21][5][7]=2;
					player.inv.plot=3;
				}

				map[f][x][y]=0;
				putall();
			}
			start();
			return 0;
		}
	//items
		if(i>=100){
			pause();
			putall();
			switch(i-100){
				case 0:msgbox("获得铁盾\n防御加10");player.dec+=10;break;
				case 1:msgbox("获得银盾\n防御加30");player.dec+=30;break;
				case 2:msgbox("获得骑士盾\n防御加70");player.dec+=70;break;
				case 3:msgbox("获得圣盾\n防御加120");player.dec+=120;break;
				case 4:msgbox("获得星光神盾\n防御加150");player.dec+=150;break;
				case 5:msgbox("获得铁剑\n攻击加10");player.atk+=10;break;
				case 6:msgbox("获得银剑\n攻击加30");player.atk+=30;break;
				case 7:msgbox("获得骑士剑\n攻击加70");player.atk+=70;break;
				case 8:msgbox("获得圣剑\n攻击加120");player.atk+=120;break;
				case 9:msgbox("获得星光神剑\n攻击加150");player.atk+=150;break;
				case 10:msgbox("获得攻击宝石\n攻击加3");player.atk+=3;break;
				case 11:msgbox("获得防御宝石\n防御加3");player.dec+=3;break;
				case 12:case 13:case 14:msgbox("获得%c钥匙！",(unsigned char)(i-112+65));player.inv.key[i-112]++;break;
				case 15:msgbox("获得小血瓶\n生命加200");player.hp+=200;break;
				case 16:msgbox("获得大血瓶\n生命加500");player.hp+=500;break;
				case 17:msgbox("获得圣水\n生命翻倍");player.hp*=2;break;
				case 18:break;
				case 19:
					if(player.f==6){msgbox("获得小\xB7\xC9\xD3\xF0\n提升1级");player.lv+=1;player.hp+=1000;player.atk+=10;player.dec+=10;}
					if(player.f==13){msgbox("获得大\xB7\xC9\xD3\xF0\n提升3级");player.lv+=1;player.hp+=1000;player.atk+=10;player.dec+=10;}
				break;
				case 20:msgbox("获得大金币\n金钱加300 ");player.gold+=300;break;
				case 21:msgbox("获得铁锒头！");player.inv.hoe++;break;
				case 22:msgbox("获得黄金十字架");player.inv.cross++;break;
				case 23:msgbox("获得\xB9\xD6\xCE\xEF\xCD\xBC\xBC\xF8\n按[optn]键查看");player.inv.book=1;break;
				case 24:msgbox("获得万能钥匙\n每种钥匙+1");player.inv.key[0]++;player.inv.key[1]++;player.inv.key[2]++;break;
				case 25:msgbox("获得电梯\n在楼梯口使用\n按[+]或[-]");player.inv.elevator=1;break;
				//END...
			}
			GetKey(&key);
			map[f][x][y]=0;
			putall();
			start();
			return 0;
		}
	//npcs
		if(i>=50 || i==10){
			pause();
			switch(i){
			case 56://fairy
				if(player.inv.plot<3){
					if(player.inv.cross==0){
						dialog(1,dialog_fairy1);
					}else{
						dialog(6,dialog_fairy2);
						player.inv.cross--;
						map[20][5][7]=3;
						player.hp+=player.hp/3;
						player.atk+=player.atk/3;
						player.dec+=player.dec/3;
					}
				}else{
					start();
					p=-1;
					for(j=0;j<4;j++){
						if(p!=-1)delsprite(p);
						p=putsprite(getpid(57),30000,2,5*16,8*16-4*j,16,16);
						putall();
						Sleep(100);
					}
					delsprite(p);
					map[0][5][7]=57;
					putall();
					Sleep(1000);
					pause();
					dialog(5,dialog_fairy3);
					start();
					map[0][5][7]=0;
					player.t=0;
					for(j=0;j<=16;j++){
						if(j%4==0)move();
						if(p!=-1)delsprite(p);
						p=putsprite(getpid(57),30000,2,5*16,7*16+4*j,16,16);
						putall();
						Sleep(100);
					}
					player.inv.plot=4;
				}
					
			break;
			case 10://shop
				switch(player.f){
					case 3: gold=25;atk=dec=4;hp=800; break;
					case 11: gold=100;atk=dec=20;hp=4000; break;
				}
				msgbox(dialog_shop[0],gold);
				GetKey(&key);putall();
				shop=menu(1,4,msgbox(dialog_shop[1],gold,atk,dec,hp));
				if(shop==3)break;
				if(player.gold<gold){
					dialog(1,&dialog_shop[2]);
				}else{
					if(shop<3)player.gold-=gold;
					switch(shop){
						case 0:player.atk+=atk;break;
						case 1:player.dec+=dec;break;
						case 2:player.hp+=hp;break;
					}
					dialog(1,&dialog_shop[3]);
				}
			break;
			case 54://merchant
				switch(player.f){
					case 2:
						dialog(5,dialog_merchant2);
						//putall();
						Sleep(1000);
						map[2][9][10]=101;
					break;
					case 15:
						dialog(4,dialog_merchant15);//实际上不是最好的……
						shop=menu(0,2,msgbox(dialog_merchant15[4]));
						putall();
						if(shop==0){
							if(player.gold<500){
								dialog(1,&dialog_merchant15[5]);
							}else{
								player.gold-=500;
								dialog(2,&dialog_merchant15[6]);
								Sleep(1000);
								map[15][6][3]=103;
							}
						}
					break;
					case 5:
						dialog(1,dialog_merchantsell);
						shop=menu(0,4,msgbox(dialog_merchantsell[1],10,50,100));
						if(shop==3)break;
						switch(shop){
							case 0:gold=10;break;
							case 1:gold=50;break;
							case 2:gold=100;break;
						}
						if(player.gold<gold){
							dialog(1,&dialog_merchantsell[2]);
						}else{
							player.gold-=gold;
							player.inv.key[shop]++;
							dialog(1,&dialog_merchantsell[3]);
						}
					break;
					case 12:
						dialog(1,dialog_merchantbuy);
						shop=menu(0,4,msgbox(dialog_merchantbuy[1],7,35,70));
						if(shop==3)break;
						switch(shop){
							case 0:gold=7;break;
							case 1:gold=35;break;
							case 2:gold=70;break;
						}
						if(player.inv.key[shop]<=0){
							putall();
							msgbox(dialog_merchantbuy[2],shop+'A');
							GetKey(&key);
						}else{
							player.gold+=gold;
							player.inv.key[shop]--;
							dialog(1,&dialog_merchantbuy[3]);
						}
					break;
				}
			break;
			case 55://old
				switch(player.f){//old1
					case 5: gold=30;gold2=100;atk=5;dec=5;lv=1; break;
					case 13: gold=90;gold2=270;atk=18;dec=18;lv=3; break;

					case 2:
						dialog(5,dialog_old2);
						//putall();
						Sleep(1000);
						map[2][7][10]=106;
					break;
					case 15:
						dialog(2,dialog_old15);
						shop=menu(0,2,msgbox(dialog_old15[2]));
						putall();
						if(shop==0){
							if(player.exp<500){
								dialog(1,&dialog_old15[3]);
							}else{
								player.exp-=500;
								dialog(2,&dialog_old15[4]);
								Sleep(1000);
								map[15][4][3]=108;
							}
						}
					break;

				}
				if(player.f==5||player.f==13){//old
					msgbox(dialog_old[0]);
					GetKey(&key);putall();
					shop=menu(0,4,msgbox(dialog_old[1],atk,gold,dec,gold,lv,gold2));
					if(shop==3)break;
					if(shop==2)gold=gold2;
					if(player.exp<gold){
						putall();
						msgbox(dialog_old[2]);
						GetKey(&key);
					}else{
						if(shop<3)player.exp-=gold;
						switch(shop){
							case 0:player.atk+=atk;break;
							case 1:player.dec+=dec;break;
							case 2:player.lv+=lv;player.atk+=10*lv;player.dec+=10*lv;player.hp+=1000*lv;break;
						}
						putall();
						msgbox(dialog_old[3]);
						GetKey(&key);
					}
				}
			break;
			case 58://thief
				if(player.inv.plot==1){
					dialog(12,dialog_thief1);
					start();
					putall();
					Sleep(1200);
					player.t=2;
					move();
					map[4][5][0]=0;
					p=-1;
					for(j=0;j<20;j++){
						if(p!=-1)delsprite(p);
						p=putsprite(getpid(58),30000,2,5*16,4*j,16,16);
						putall();
						Sleep(100);
					}
					Sleep(3000);
					for(j=20;j>=0;j--){
						if(p!=-1)delsprite(p);
						p=putsprite(getpid(58),30000,2,5*16,4*j,16,16);
						putall();
						Sleep(100);
					}
					delsprite(p);
					map[4][5][0]=58;
					pause();
					dialog(1,dialog_thief2);
					map[2][1][6]=0;
					player.inv.plot=2;
				}

				if(player.inv.hoe){
					dialog(2,dialog_thief3);
					player.inv.hoe--;
					start();
					putall();
					Sleep(1200);
					player.t=2;
					move();
					map[4][5][0]=0;
					p=-1;
					for(j=0;j<20;j++){
						if(p!=-1)delsprite(p);
						p=putsprite(getpid(58),30000,2,5*16,4*j,16,16);
						putall();
						Sleep(100);
					}
					delsprite(p);
					map[18][5][8]=map[18][5][9]=0;
				}
			break;
			case 57://princess
				if(player.inv.plot<3){
					dialog(13,dialog_princess1);
				}else{
					dialog(5,dialog_princess2);
					map[18][5][4]=0;
					start();
					p=-1;
					for(j=0;j<4;j++){
						if(p!=-1)delsprite(p);
						p=putsprite(getpid(57),30000,2,5*16,4*j+4*16,16,16);
						putall();
						Sleep(100);
					}
					delsprite(p);
				}
			break;
			}
			start();
			return 0;
		}

	return 0;
}
int check2(int f,int x,int y)
{
	int i;
	if(x<0||x>=mapwidth||y<0||y>=mapheight||f<0||f>mapnumber)return 0;
	i=map[f][x][y];
	switch(i){
	case 2:
		Sleep(200);
		if(player.f==1)player.y=0;
		if(player.f>0)player.f--;
		fixviewport(player.x*16+8,player.y*16+8,player.f);
		putall();
	break;case 3:
		Sleep(200);
		if(player.f==0)player.y=10;
		if(player.f<mapnumber-1)player.f++;
		if(player.inv.maxfloor<player.f)player.inv.maxfloor=player.f;
		fixviewport(player.x*16+8,player.y*16+8,player.f);
		putall();
	break;
	}
}
int check3()
{
	if(player.inv.plot==0)beginning();
}
int monsterpedia()
{
	unsigned int key;
	int page,i,dam;
	unsigned char mat[32];
	clock=0;
	page=0;
	while(1){
		putpicture(hud_book,0,0,128,64);
		for(i=0;i<=1;i++)if(i+page<monsternumber){
			getpicture(mat,getpid(page+i+200));
			putpicture(mat,40+i*65,32,16,16);
			printf12(5+i*65,5,"%s",monsters0[page+i].name);
			printf7(11+i*65,19,"%d",monsters0[page+i].hp);
			printf7(11+i*65,27,"%d",monsters0[page+i].atk);
			printf7(11+i*65,35,"%d",monsters0[page+i].dec);
			printf7(11+i*65,43,"+%d",monsters0[page+i].gold);
			printf7(11+i*65,51,"+%d",monsters0[page+i].exp);
			dam=damageestimate(page+i);
			if(dam==-1)printf5(40+i*65,51,"?????");
			else printf5(44-2*log10(dam)+i*65,51,"-%d",damageestimate(page+i));
		}
		GetKey(&key);
		if(key==KEY_CTRL_EXIT||key==KEY_CTRL_AC||key==KEY_CTRL_OPTN)break;
		if((key==KEY_CTRL_LEFT||key==KEY_CTRL_UP)&&page>=2)page-=2;
		if((key==KEY_CTRL_RIGHT||key==KEY_CTRL_DOWN)&&page<=monsternumber-3)page+=2;
	}
	putall();
}
int save()
{
	int fp=0;
	unsigned short file_name[24];
	chr_to_fnt("\\\\fls0\\MagTower.sav",file_name);
	Bfile_DeleteFile(file_name);
	Bfile_CreateFile(file_name,sizeof(player)+sizeof(map));
	fp = Bfile_OpenFile(file_name,_OPENMODE_WRITE);
	if(fp<0)return 0;
	Bfile_WriteFile(fp,&player,sizeof(player));
	Bfile_WriteFile(fp,&map,sizeof(map));
	Bfile_CloseFile(fp);
	return 1;
}
int load()
{
	int fp=0;
	unsigned short file_name[24];
	chr_to_fnt("\\\\fls0\\MagTower.sav",file_name);
	fp = Bfile_OpenFile(file_name,_OPENMODE_READ);
	if(fp<0)return 0;
	Bfile_ReadFile(fp,&player,sizeof(player),-1);
	Bfile_ReadFile(fp,&map,sizeof(map),-1);
	Bfile_CloseFile(fp);
	return 1;
}
int gamemenu()
{
	unsigned int key;
	pause();
	key=menu(1,3,msgbox("\xD3\xCE\xCF\xB7\xD4\xDD\xCD\xA3\n1.\xBC\xCC\xD0\xF8\n2.\xB1\xA3\xB4\xE6\n3.\xCD\xCB\xB3\xF6"));
	switch(key){
		case 0://continue
			start();
		break;
		case 1://save
			if(save()){
				msgbox("\xB1\xA3\xB4\xE6\xB3\xC9\xB9\xA6\xA3\xA1");
			}else{
				msgbox("\xB1\xA3\xB4\xE6\xCA\xA7\xB0\xDC\xA1\xA3\xA1\xA3\xA1\xA3");
			}
			GetKey(&key);
			start();
		break;
		case 2://exit
			GAME=0;
			//GetKey(&key);
			//while(key!=KEY_CTRL_MENU&&key!=KEY_CTRL_AC)GetKey(&key);
			//start();
		break;
	}
}
int gameloop()
{
	unsigned int key;
	int i;
	while(GAME){
		check3();
		if(IsKeyDown(KEY_CHAR_7) && IsKeyDown(KEY_CTRL_SHIFT) && IsKeyDown(KEY_CTRL_F6)){
			pause();
			putall();
			key=menu(1,4,msgbox("====Debug Mode====\n1.Lv+10\n2.Gold&Exp+1000\n3.Turn %s CheatMode\n4.Exit",CHEATMODE?"Off":"On"));
			switch(key){
				case 0:
					player.lv+=10;
					player.atk+=100;
					player.dec+=100;
					player.hp+=10000;
				break;
				case 1:
					player.gold+=1000;
					player.exp+=1000;
				break;
				case 2:
					CHEATMODE=!CHEATMODE;
				break;
			}
			putall();
			start();
		}
		if(IsKeyDown(KEY_CTRL_MENU)){
				pause();
				GetKey(&key);
				start();
		}
		if(IsKeyDown(KEY_CTRL_DOWN)){
				player.t=0;
				if(check1(player.f,player.x,player.y+1))move();
				check2(player.f,player.x,player.y);
		}
		if(IsKeyDown(KEY_CTRL_UP)){
				player.t=1;
				if(check1(player.f,player.x,player.y-1))move();
				check2(player.f,player.x,player.y);
		}
		if(IsKeyDown(KEY_CTRL_LEFT)){
				player.t=2;
				if(check1(player.f,player.x-1,player.y))move();
				check2(player.f,player.x,player.y);
		}
		if(IsKeyDown(KEY_CTRL_RIGHT)){
				player.t=3;
				if(check1(player.f,player.x+1,player.y))move();
				check2(player.f,player.x,player.y);
		}
		if(IsKeyDown(KEY_CTRL_AC)||IsKeyDown(KEY_CTRL_EXIT)){
				gamemenu();
				putall();
		}
		if(IsKeyDown(KEY_CTRL_OPTN)){
				while(IsKeyDown(KEY_CTRL_OPTN));
				pause();
				if(player.inv.book || CHEATMODE)monsterpedia();
				while(IsKeyDown(KEY_CTRL_OPTN));
				start();
		}
		if(IsKeyDown(KEY_CHAR_PLUS)){
				while(IsKeyDown(KEY_CHAR_PLUS));
				if(CHEATMODE){
					player.f++;fixviewport(player.x*16+8,player.y*16+8,player.f);putall();
				}else{
					elevatecheck(player.f+1);
				}
		}
		if(IsKeyDown(KEY_CHAR_MINUS)){
				while(IsKeyDown(KEY_CHAR_MINUS));
				if(CHEATMODE){
					player.f--;fixviewport(player.x*16+8,player.y*16+8,player.f);putall();
				}else{
					elevatecheck(player.f-1);
				}
		}
		//putall();
		//putall();
		//Sleep(100);
	}
}
int initgame()
{
	int f,x,y;int p;
	for(f=0;f<mapnumber;f++)for(x=0;x<mapwidth;x++)for(y=0;y<mapheight;y++)
		map[f][x][y]=map0[f][y][x];
	player.hp=1000;
	player.atk=10;
	player.dec=10;
	player.lv=1;
	player.gold=0;
	player.exp=0;
	player.f=0;
	player.x=5;
	player.y=9;
	player.t=0;
	player.c=0;
	player.inv.key[0]=0;
	player.inv.key[1]=0;
	player.inv.key[2]=0;
	player.inv.hoe=0;
	player.inv.cross=0;
	player.inv.book=0;
	player.inv.elevator=0;
	player.inv.maxfloor=0;
	player.inv.plot=0;
}
int quithandle()
{
	int key;
	if(GAME){
		key=menu(2,2,msgbox("\xD3\xCE\xCF\xB7\xCD\xCB\xB3\xF6\xA3\xAC\n\xC4\xE3\xD2\xAA\xB1\xA3\xB4\xE6\xC2\xF0\xA3\xBF\n1.\xB1\xA3\xB4\xE6\xBA\xF3\xCD\xCB\xB3\xF6\n2.\xB2\xBB\xB1\xA3\xB4\xE6\xD6\xB1\xBD\xD3\xCD\xCB\xB3\xF6"));
		if(key==0){//save
			save();
			msgbox("\xB1\xA3\xB4\xE6\xB3\xC9\xB9\xA6\xA3\xA1");
			GetKey(&key);
		}
	}
	pause();
}
int AddIn_main(int isAppli, unsigned short OptionNum)
{
    unsigned int key;

    CHEATMODE=0;
    GAME=0;

    SetQuitHandler(quithandle);

    open_chs_font ();
    mapwidth=11;
    mapheight=11;
    mapnumber=22;
    clock=0;

    key=menu(1,3,msgbox("\xC4\xA7\xCB\xFE\n\xD0\xC2\xD3\xCE\xCF\xB7\n\xBC\xCC\xD0\xF8\n\xCD\xCB\xB3\xF6"));
    switch(key){
    case 0:
        GAME=1;
        initgame();
    break;
    case 1:
        if(!load()){msgbox("\xBC\xD3\xD4\xD8\xCA\xA7\xB0\xDC\xA1\xA3\xA1\xA3\xA1\xA3");GetKey(&key);return 0;}
        GAME=1;
        fixviewport(player.x*16+8,player.y*16+8,player.f);
	putall();
	msgbox("\xBC\xD3\xD4\xD8\xB3\xC9\xB9\xA6\xA3\xA1");GetKey(&key);
    break;
    case 2:
        GAME=0;
    break;
    }
    if(GAME){
        start();
        gameloop();
    }
    //while(1){
    //    GetKey(&key);
    //}

    return 1;
}




//****************************************************************************
//**************                                              ****************
//**************                 Notice!                      ****************
//**************                                              ****************
//**************  Please do not change the following source.  ****************
//**************                                              ****************
//****************************************************************************


#pragma section _BR_Size
unsigned long BR_Size;
#pragma section


#pragma section _TOP

//****************************************************************************
//  InitializeSystem
//
//  param   :   isAppli   : 1 = Application / 0 = eActivity
//              OptionNum : Option Number (only eActivity)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}

#pragma section

