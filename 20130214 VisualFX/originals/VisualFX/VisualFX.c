#include "fxlib.h"
#include "stddef.h"
#include "VisualFX.h"
/////////////////////////////font
vf_FONT* vf_addfont(int width,int height,int multi,vf_PUTSTRING putstring)
{
	vf_FONT* ptr;
	ptr=(vf_FONT*)malloc(sizeof(vf_FONT));
	if(ptr==NULL) return NULL;
	ptr->width=width;
	ptr->height=height;
	ptr->multi=multi;
	ptr->putstring=putstring;
	return ptr;
}
void vf_freefont(vf_FONT* font)
{
	free(font);
	font=NULL;
}
static is_multichar(char c)
{
	return c&0x80;
}
static void puttext(int x,int y,vf_FONT* font,int width,int height,int sel,char* text)
{
	int pt,pb,c,lim,yr;
	char printbuff[255];

	lim=(width+1)/(font->width+1);
	pt=0;yr=0;
	while(text[pt] && yr+font->height<=height){
		pb=0;c=0;
		while(text[pt] && c<lim){
			if(text[pt]=='\n'){pt++;break;}
			if(is_multichar(printbuff[pb++]=text[pt++])) printbuff[pb++]=text[pt++];
			c++;
		}
		printbuff[pb]=0;
		font->putstring(x,y+yr,sel,printbuff);
		yr+=font->height+1;
	}
}
static void printxy(int x,int y,int sel,char* text){PrintXY(x,y,text,sel);}
static void printmini(int x,int y,int sel,char* text){PrintMini(x,y,text,sel);}
vf_FONT* vf_adddefaultfont()
{
	return vf_addfont(5,7,0,printxy);
}
vf_FONT* vf_addminifont()
{
	return vf_addfont(4,5,0,printmini);
}
/////////////////////////////default control
/*
typedef struct{
	c
static void textbox_fresh(vf_DRAWBOX drawbox,TEXTBOX textbox)
{
	
}
*/

















int t()
{
	vf_FONT *b,*s;
	b=vf_adddefaultfont();
	s=vf_addminifont();
	puttext(0,0,b,128,64,0,"static void printmini(int x,int y,int sel,char* text)\n{\nPrintMini(x,y,text,sel);\n}");
}
