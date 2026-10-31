/**
 *  The source is modified from SimSpell v1.0
 *  special thanks to the author of SimSpell - diameter
**/

#include "fxlib.h"
#define IS_CH(C) ((C) & 0x80)

int fh_HZK = 0;

FONTCHARACTER * CharToFont(const char *cFileName,FONTCHARACTER *fFileName)
{
	int i,len = strlen(cFileName);
	for(i=0; i<len ;++i)
		fFileName[i] = cFileName[i];
	fFileName[i]=0;
	return fFileName;
}
char * FontToChar(const FONTCHARACTER *fFileName,char *cFileName)
{
	int i = 0;
	while((cFileName[i]=fFileName[i])!=0)++i;
	return cFileName;
}

int OpenFont()
{
	FONTCHARACTER FileName[20];
	CharToFont("\\\\fls0\\HZK12.df",FileName);
	fh_HZK = Bfile_OpenFile(FileName,_OPENMODE_READ);
	return fh_HZK;
}

int CloseFont()
{
	Bfile_CloseFile(fh_HZK);
}

void DrawClearBox (int left,int top,int right,int bottom,int sel)
{
	DISPBOX box;
	box.left = left;box.top = top;box.right = right;box.bottom = bottom;
	Bdisp_AreaClr_VRAM (&box);
	if(sel)
	{
		Bdisp_DrawLineVRAM (box.left,box.top,box.right,box.top);
		Bdisp_DrawLineVRAM (box.left,box.bottom,box.right,box.bottom);
		Bdisp_DrawLineVRAM (box.left,box.top,box.left,box.bottom);
		Bdisp_DrawLineVRAM (box.right,box.top,box.right,box.bottom);
	}
}

void DrawPicture (int x,int y,int width,int height,unsigned char * pGraph)
{
	DISPGRAPH dg;
	dg.x = x;
	dg.y = y;
	dg.GraphData.width = width;
	dg.GraphData.height = height;
	dg.GraphData.pBitmap = pGraph;
	dg.WriteModify = IMB_WRITEMODIFY_NORMAL;
	dg.WriteKind = IMB_WRITEKIND_OVER;
	Bdisp_WriteGraph_VRAM(&dg);
}

void PrintChChar (unsigned char c1,unsigned char c2,int pX,int pY)
{
	unsigned char mat[12][2];
	int i,j,k;
	int sec,pot; 
	sec = c1-0xa0;
	pot = c2-0xa0;
	Bfile_ReadFile(fh_HZK,mat,24,(94*(sec-1)+(pot-1))*24l);
	DrawPicture (pX,pY,12,12,mat);
}

void PrintChStr (unsigned char *cStr,int pX,int pY)
{
	unsigned int key;
	unsigned char c[]=" ";
	while (*cStr)
	{
		if (IS_CH(*cStr))
		{
			unsigned char c1 = *cStr++,
						  c2 = *cStr;
			PrintChChar(c1,c2,pX,pY);
			++cStr;
			pX +=12;
		}
		else
		{
			c[0]=*(cStr++);
			PrintXY(pX,pY+3,c,0);
			pX +=6;
		}
	}
}
