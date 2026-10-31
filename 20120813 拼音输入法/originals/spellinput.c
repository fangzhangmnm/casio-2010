/**
 *  The source is modified from SimSpell v1.0
 *  special thanks to the author of SimSpell - diameter
**/

#include "fxlib.h"
#include <string.h>
#include "pinyin.h"

void PrintChStr (unsigned char *cStr,int pX,int pY);

int GetPressCharSp (int key)
{
	switch(key)
	{
		case KEY_CTRL_XTT:
		case KEY_CHAR_A:	return 'a';
		case KEY_CHAR_LOG:
		case KEY_CHAR_B:	return 'b';
		case KEY_CHAR_LN:
		case KEY_CHAR_C:	return 'c';
		case KEY_CHAR_SIN:
		case KEY_CHAR_D:	return 'd';
		case KEY_CHAR_COS:
		case KEY_CHAR_E:	return 'e';
		case KEY_CHAR_TAN:
		case KEY_CHAR_F:	return 'f';
		case KEY_CHAR_FRAC:
		case KEY_CHAR_G:	return 'g';
		case KEY_CTRL_FD:
		case KEY_CHAR_H:	return 'h'; 
		case KEY_CHAR_LPAR:
		case KEY_CHAR_I:	return 'i';
		case KEY_CHAR_RPAR:
		case KEY_CHAR_J:	return 'j';
		case KEY_CHAR_COMMA:
		case KEY_CHAR_K:	return 'k';
		case KEY_CHAR_STORE:
		case KEY_CHAR_L:	return 'l';
		case KEY_CHAR_7:
		case KEY_CHAR_M:	return 'm';
		case KEY_CHAR_8:
		case KEY_CHAR_N:	return 'n';
		case KEY_CHAR_9:
		case KEY_CHAR_O:	return 'o';
		case KEY_CHAR_4:
		case KEY_CHAR_P:	return 'p';
		case KEY_CHAR_5:
		case KEY_CHAR_Q:	return 'q';
		case KEY_CHAR_6:
		case KEY_CHAR_R:	return 'r';
		case KEY_CHAR_MULT:
		case KEY_CHAR_S:	return 's';
		case KEY_CHAR_DIV:
		case KEY_CHAR_T:	return 't';
		case KEY_CHAR_1:
		case KEY_CHAR_U:	return 'u';
		case KEY_CHAR_2:
		case KEY_CHAR_V:	return 'v';
		case KEY_CHAR_3:
		case KEY_CHAR_W:	return 'w';
		case KEY_CHAR_PLUS:
		case KEY_CHAR_X:	return 'x';
		case KEY_CHAR_MINUS:
		case KEY_CHAR_Y:	return 'y';
		case KEY_CHAR_0:
		case KEY_CHAR_Z:	return 'z';
		default:			return 0;
	}
}

int InputString(int x,int y,char *c)
{
	// fix len = 8
	unsigned int key;
	char ch;
	int len = 8;
	int i = strlen(c);
	c[i] = ch = 0;
	PrintXY(x,y,"        ",1);
	PrintXY(x,y,c,1);
	while (1)
	{
		GetKey (&key);
		if (ch=GetPressCharSp(key))
		{
			if (i<len)
			{
				c[i++] = ch;
			}
		}
		else
		{
			if (key==KEY_CTRL_AC)
				i = 0;
			else if (key==KEY_CTRL_DEL) {if(i>0)c[i--] = 0;}
			else if (key==KEY_CTRL_EXE || key==KEY_CTRL_EXIT)
			{
				PrintXY(x,y,"        ",0);
				PrintXY(x,y,c,0);				
				return key==KEY_CTRL_EXE ? 1:0;
			}
		}
		c[i] = ch = 0;
		PrintXY(x,y,"        ",1);
		PrintXY(x,y,c,1);
	}
}

typedef struct
{
	char *key;
	char *table;
}FIND_INDEX;

typedef struct
{
	short py_size;
	short table_size;
}TABLE_INFO;

TABLE_INFO table_info[]={
	{  1,	  4},	{  2,	 26},	{  2,	 18},	{  3,	  6},	{  2,	 18},	{  2,	 34},
	{  3,	 16},	{  3,	 30},	{  4,	 24},	{  3,	 36},	{  3,	 30},	{  3,	 10},
	{  4,	 12},	{  2,	 46},	{  4,	 24},	{  4,	  8},	{  3,	  8},	{  3,	 12},
	{  4,	 18},	{  2,	 40},	{  2,	 20},	{  2,	  2},	{  3,	 22},	{  3,	 14},
	{  4,	 10},	{  3,	 10},	{  2,	 10},	{  4,	  6},	{  3,	 24},	{  4,	  6},
	{  4,	 20},	{  5,	 24},	{  4,	 20},	{  3,	 12},	{  4,	 20},	{  5,	 30},
	{  3,	 30},	{  5,	 10},	{  4,	 24},	{  3,	 34},	{  5,	  2},	{  5,	 14},
	{  6,	 10},	{  4,	 10},	{  4,	 14},	{  4,	  2},	{  2,	 24},	{  4,	 12},
	{  3,	  2},	{  2,	  8},	{  4,	  6},	{  3,	 16},	{  3,	  6},	{  3,	 12},
	{  2,	 12},	{  3,	 24},	{  3,	 30},	{  4,	 10},	{  3,	 24},	{  2,	  6},
	{  4,	 14},	{  2,	 36},	{  4,	 32},	{  4,	 16},	{  3,	 14},	{  4,	 18},
	{  3,	  2},	{  4,	 20},	{  3,	 16},	{  2,	 28},	{  4,	 12},	{  3,	  8},
	{  3,	 16},	{  3,	 24},	{  1,	 26},	{  2,	  2},	{  2,	 16},	{  2,	 16},
	{  3,	 34},	{  4,	 22},	{  3,	 24},	{  3,	 30},	{  4,	 30},	{  2,	  2},
	{  3,	  2},	{  2,	 88},	{  2,	  4},	{  3,	 12},	{  3,	 22},	{  4,	 18},
	{  3,	 20},	{  2,	 34},	{  3,	  2},	{  3,	  4},	{  4,	 14},	{  4,	 30},
	{  3,	 18},	{  2,	 36},	{  3,	 12},	{  4,	  6},	{  4,	 22},	{  5,	  6},
	{  3,	 32},	{  3,	  6},	{  3,	 12},	{  2,	  4},	{  3,	 14},	{  3,	 38},
	{  4,	  6},	{  3,	 18},	{  2,	 34},	{  3,	  4},	{  3,	  8},	{  4,	 10},
	{  4,	 18},	{  3,	 14},	{  2,	 36},	{  3,	 18},	{  4,	 10},	{  4,	 28},
	{  5,	 28},	{  3,	 42},	{  3,	 12},	{  3,	 20},	{  2,	108},	{  3,	 36},
	{  4,	 78},	{  5,	 26},	{  4,	 56},	{  3,	 50},	{  3,	 40},	{  4,	 50},
	{  5,	  4},	{  3,	 34},	{  2,	 52},	{  4,	 14},	{  3,	 18},	{  3,	 22},
	{  2,	  6},	{  3,	 10},	{  3,	 14},	{  4,	 14},	{  3,	  8},	{  2,	 30},
	{  3,	  8},	{  4,	  4},	{  4,	  8},	{  3,	  8},	{  2,	 14},	{  3,	 10},
	{  4,	  8},	{  4,	  4},	{  5,	 16},	{  3,	 22},	{  3,	  8},	{  3,	  8},
	{  2,	 14},	{  3,	  6},	{  3,	 30},	{  4,	 14},	{  3,	 18},	{  2,	  6},
	{  3,	 22},	{  4,	  6},	{  2,	 68},	{  4,	 28},	{  5,	 24},	{  4,	 24},
	{  3,	 10},	{  3,	 24},	{  4,	 28},	{  3,	 22},	{  4,	 18},	{  3,	 12},
	{  2,	 40},	{  4,	 12},	{  3,	  4},	{  3,	 14},	{  3,	 24},	{  2,	 28},
	{  2,	 18},	{  3,	 12},	{  3,	 18},	{  4,	 12},	{  3,	 24},	{  2,	  2},
	{  3,	 32},	{  3,	  6},	{  4,	 16},	{  2,	 28},	{  4,	 18},	{  4,	 16},
	{  3,	  4},	{  3,	 12},	{  4,	 12},	{  3,	  2},	{  2,	 36},	{  3,	  6},
	{  2,	 30},	{  2,	 14},	{  3,	 10},	{  3,	  6},	{  4,	  2},	{  3,	 10},
	{  2,	  2},	{  3,	  4},	{  3,	  2},	{  4,	  2},	{  2,	 22},	{  4,	 14},
	{  5,	  4},	{  4,	  4},	{  3,	 14},	{  3,	  2},	{  4,	 12},	{  3,	  8},
	{  4,	  8},	{  2,	  6},	{  4,	  2},	{  3,	  4},	{  3,	  8},	{  2,	  2},
	{  1,	  2},	{  2,	 14},	{  2,	 14},	{  3,	 12},	{  3,	 16},	{  4,	 10},
	{  3,	 14},	{  3,	 18},	{  3,	  4},	{  4,	 28},	{  2,	 36},	{  4,	  8},
	{  4,	  8},	{  3,	  4},	{  3,	 10},	{  4,	 18},	{  2,	 16},	{  3,	  2},
	{  2,	 30},	{  2,	 72},	{  3,	  6},	{  4,	 44},	{  5,	 16},	{  4,	 30},
	{  3,	 10},	{  3,	 22},	{  4,	 26},	{  5,	  4},	{  3,	 16},	{  2,	 26},
	{  4,	 22},	{  3,	 16},	{  3,	  4},	{  3,	  8},	{  4,	 10},	{  3,	  6},
	{  2,	  4},	{  3,	 20},	{  4,	  4},	{  2,	  2},	{  4,	 20},	{  3,	  6},
	{  2,	 20},	{  4,	  4},	{  3,	  6},	{  3,	  4},	{  3,	  4},	{  2,	  6},
	{  3,	  8},	{  3,	  8},	{  4,	  6},	{  3,	  8},	{  2,	  6},	{  3,	  2},
	{  4,	  2},	{  3,	 18},	{  4,	  4},	{  4,	 34},	{  5,	 16},	{  4,	 22},
	{  3,	 24},	{  4,	 34},	{  5,	 22},	{  3,	 96},	{  4,	 20},	{  3,	 66},
	{  4,	  4},	{  5,	  8},	{  5,	  4},	{  6,	  5},	{  4,	  8},	{  4,	  8},
	{  4,	  8},	{  2,	 30},	{  4,	 16},	{  3,	  8},	{  2,	 22},	{  4,	  6},
	{  3,	 22},	{  3,	  6},	{  3,	 16},	{  2,	 18},	{  3,	 18},	{  3,	 36},
	{  4,	 26},	{  3,	 22},	{  2,	  2},	{  4,	  8},	{  2,	 30},	{  4,	 16},
	{  4,	 12},	{  3,	  6},	{  4,	 20},	{  4,	 26},	{  3,	  8},	{  2,	 22},
	{  4,	  4},	{  3,	 12},	{  3,	  8},	{  3,	 22},	{  2,	 14},	{  3,	  4},
	{  3,	 34},	{  4,	 20},	{  3,	 66},	{  3,	 20},	{  4,	  6},	{  2,	 18},
	{  2,	 58},	{  2,	 68},	{  3,	 24},	{  4,	 54},	{  5,	 40},	{  4,	 34},
	{  3,	 40},	{  3,	 20},	{  4,	 28},	{  5,	 14},	{  3,	 20},	{  2,	 38},
	{  4,	 20},	{  3,	 14},	{  3,	 28},	{  2,	 32},	{  3,	 66},	{  4,	 34},
	{  3,	 34},	{  2,	 30},	{  2,	106},	{  3,	 32},	{  4,	 36},	{  2,	  2},
	{  4,	 30},	{  3,	 40},	{  2,	 88},	{  4,	 40},	{  3,	 18},	{  3,	 24},
	{  2,	  8},	{  3,	 16},	{  3,	  8},	{  4,	  6},	{  3,	 28},	{  2,	  8},
	{  3,	  2},	{  3,	  2},	{  4,	  6},	{  3,	 26},	{  4,	 14},	{  4,	 34},
	{  5,	 32},	{  4,	 22},	{  3,	 22},	{  4,	 34},	{  5,	 28},	{  3,	 86},
	{  5,	 22},	{  4,	 28},	{  3,	 52},	{  4,	  2},	{  5,	  2},	{  5,	 10},
	{  6,	 16},	{  4,	 12},	{  4,	  4},	{  4,	 20},	{  2,	 28},	{  4,	 14},
	{  3,	  8},	{  2,	 16},	{  4,	  6},	{  3,	  8},	{  3,	  4}			   };

const int table_size = sizeof(table_info)/sizeof(TABLE_INFO);

FIND_INDEX *find_index[sizeof(table_info)/sizeof(TABLE_INFO)];

extern FONTCHARACTER * CharToFont(const char *cFileName,FONTCHARACTER *fFileName);
extern char * FontToChar(const FONTCHARACTER *fFileName,char *cFileName);

int LoadSpellTable ()
{
	int fh_lib;
	int i,pos;
	FONTCHARACTER FileName[20];
	CharToFont("\\\\fls0\\PINYIN.py",FileName);
	fh_lib = Bfile_OpenFile(FileName,_OPENMODE_READ);
	
	if (fh_lib<0) return -1;

	pos = 0;
	for (i=0;i<table_size;++i)
	{
		find_index[i]  =  (FIND_INDEX*)calloc(sizeof(FIND_INDEX),1);
		find_index[i]->key   =  (char*)calloc(sizeof(char),table_info[i].py_size+1);
		Bfile_ReadFile(	fh_lib,
						find_index[i]->key,
						table_info[i].py_size,
						pos);
		pos+=table_info[i].py_size;
		find_index[i]->table =  (char*)calloc(sizeof(char),table_info[i].table_size+1);
		Bfile_ReadFile(	fh_lib,
						find_index[i]->table,
						table_info[i].table_size,
						pos);
		pos+=table_info[i].table_size;
	}
	Bfile_CloseFile(fh_lib);
}

int MatchSpellTable (char *mc)
{
	int i,j=-1,t=0,k;
	for (i=0;i<table_size;++i)
	{
		if (*(find_index[i]->key)==*mc)
		{
			if(j<0)j = i;
			++t;
		}
		else if(t) break;
	}
	if (j<0) return -1;
	for (t=j;t<i;++t)
	{
		if (strcmp(find_index[t]->key,mc)==0)
			return t;
	}
	return -1;
}

short SpellInputAChar()
{
	char buf[16]="";
	int size,p,d,r,k;
	unsigned int key;
	while(1)
	{
		DrawClearBox (0,48,127,63,1);
		DrawClearBox (0,39,6*8+1,48,1);
		r = InputString(1,40,buf);
		if (r)
		{
			d = MatchSpellTable(buf);
			if (d>=0)
			{
				size = strlen(find_index[d]->table)/2;
				r = 1;
				p = 0;
				while(1)
				{
					if (r)
					{
						int i;
						char id[]="0",chs[]="  ";
						DrawClearBox (0,48,127,63,1);
						for (i=0;i<5 && p+i<size;++i)
						{
							(*id)++;
							PrintMini(i*25+1,54,id,MINI_OVER);
							memcpy(chs,&(find_index[d]->table[(i+p)*2]),2);
							PrintChStr(chs,i*25+6,50);
						}
						r = 0;
					}					
					GetKey(&key);
					if (key==KEY_CTRL_EXIT) break;
					else if (key==KEY_CTRL_RIGHT || key== KEY_CTRL_F6)
					{
						p+=5;if (p>=size) p -= 5;r=1;
					}
					else if (key==KEY_CTRL_LEFT)
					{
						p-=5;if (p<0) p =0;r =1;
					}
					for (k=0;k<5;++k)
					{
						if (key=='1'+k)
						{
							if (p+k<size)
							{
								short ch;
								memcpy(&ch,&(find_index[d]->table[(k+p)*2]),2);
								return ch;
							}
						}
					}
				}
			}
		}
		else
			return 0;
	}
}
