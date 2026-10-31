#include "fxlib.h"
#include <stdio.h>
#include "spellinput.h"

const int SysCallWrapper[] = { 0xD201422B, 0x60F20000, 0x80010070 };
const int (*SysCallFuncPtr)( int R4, int R5, int R6, int R7, int FNo ) = (void*)&SysCallWrapper;
#define App_RUN_MAT() (void)(*SysCallFuncPtr)( 0, 0, 0, 0, 0x0AAE)
#define PutKey(key) (void)(*SysCallFuncPtr)( key, key<256, 0, 0, 0x0910)

int getAlpha( char var );
void clearAlpha( char var );

void timer( void )
{
	if( IsKeyDown(KEY_CTRL_F6) )
	{
		short code;
		unsigned char buf[16];
		unsigned char *p;

		SaveDisp( 1 );
		code = SpellInputAChar();
		sprintf( buf, "%d", code );
		RestoreDisp( 1 );
		Bdisp_PutDisp_DD();
		if( buf[0] == '-' )
			buf[0] = KEY_CHAR_PMINUS;
		for( p = buf; *p; p++ )
			PutKey( *p );
	}
	if( getAlpha( 'B' ) == 12 )
	{
		char buf[4] = {0};
		int x = getAlpha( 'X' );
		int y = getAlpha( 'Y' );
		short code = getAlpha( 'A');

		clearAlpha( 'B' );

		memcpy( buf, &code, 2 );
		PrintChStr( buf, x, y );
		GetKey( &x );
	}
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
    unsigned int key;

    Bdisp_AllClr_DDVRAM();

	if( OpenFont() < 0 )
	{
		locate(1,1);Print("Font not find");
		GetKey(&key);return 0;
	}

	PrintChStr("载入中，稍候...",20,24);
	Bdisp_PutDisp_DD ();

	if( LoadSpellTable() < 0 )
	{
		Bdisp_AllClr_DDVRAM();
		PrintChStr( "拼音库无法找到", 0, 0 );
		GetKey( &key );return 0;
	}

	clearAlpha( 'B' );
	SetTimer( 1, 250, timer );
	App_RUN_MAT();

    return 1;
}



#define Alpha_GetData(VarName,Dest) (void)(*SysCallFuncPtr)( VarName,(int)Dest, 0, 0, 0x04DF)
#define NUM_DataToStr(data,str) (void*)(*SysCallFuncPtr)( (int)data, (int)str, 0, 0, 0x4F0 )
int getAlpha( char var )
{
	char data[24] = {0};
	unsigned char str[50];
	int value = 0;

	Alpha_GetData( var, data );
	NUM_DataToStr( data, str );

	if( str[0] == 0x87 ) // 0x87 == '-'
		str[0] = '-';
	
	sscanf( str, "%d", &value );

	return value;
}

#define Alpha_SetData(VarName,Dest) (void)(*SysCallFuncPtr)( VarName,(int)Dest, 0, 0, 0x04E0)
void clearAlpha( char var )
{
	char data[24] = {0};
	char str[50];

	Alpha_SetData( var, data );
}



#pragma section _BR_Size
unsigned long BR_Size;
#pragma section
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}
#pragma section



