//
const VERSION=1000;
//

#include "fxlib.h"
#include "stdarg.h"
#include "stdlib.h"
#include "dispbios.h"
#include "filebios.h"
#include "timer.h"
#include "string.h"
#include "revolution.h"

void strprintf(char *cpBuffer,char *cpFormat, ...)
{
va_list pArgList;
va_start(pArgList, cpFormat);
  vsprintf(cpBuffer, cpFormat, pArgList);
va_end(pArgList);
}

#include "Time.h"
#include "Database.h"
#include "Rules.h"
#include "UI.h"




int AddIn_main(int isAppli, unsigned short OptionNum)
{
	unsigned int key;
	time t;int n;
	char buff[512];
	if(db_init_game()) ui_init();
	ui_index();
	while(1){
		GetKey(&key);
	}

	return 1;
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
