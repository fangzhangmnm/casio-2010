#include "fxlib.h"
typedef struct{
	float x,y;
	float o,v;
	int c;
	int type;
} bullet;
struct{
	float x,y;
} player;

bullet bullets[100];int bulletnum;
void checkbullet()
{
	int i;

	for(i=0;i<bulletnum;i+=1)if(bullets[i].type){
		switch(bullets[i].type){
			case 324:	//test
			break;
		}
	}
}
void init()
{
	int i;

	player.x=8;player.y=32;
	bulletnum=0;
	for(i=0;i<1000;i+=1) bullets[i].type=0;
}
int AddIn_main(int isAppli, unsigned short OptionNum)
{
    unsigned int key;

    init();

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

