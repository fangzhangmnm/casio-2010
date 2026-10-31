/*****************************************************************/
/*                                                               */
/*   CASIO fx-9860G SDK Library                                  */
/*                                                               */
/*   File name : [ProjectName].c                                 */
/*                                                               */
/*   Copyright (c) 2006 CASIO COMPUTER CO., LTD.                 */
/*                                                               */
/*****************************************************************/
#include "fxlib.h"
#include "mathf.h"
#include "revolution.h"

#include "Textures.h"
#include "Maps.h"
#include "Public.h"
#include "Painter.h"
#include "DataStruct.h"
#include "PainterSort.h"
int TouchCheck(x0,y0,dx,dy) //must rely on RenderQueue!
{

}

#include "Game_Script.h"
#define MAXCREATURES 10

float CreatureX[MAXCREATURES],CreatureY[MAXCREATURES];
char CreatureType[MAXCREATURES];
const void (*CreatureAI[])(void)={&gs_RegionError};
char CreatureStatic[MAXCREATURES];
long CreatureHP[MAXCREATURES];
/*
const char RegionMap[]=
{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	1,	1,	2,	3,	3,	3,	3,	3,	3,
	0,	1,	1,	2,	3,	3,	3,	3,	3,	3,
	0,	1,	1,	2,	3,	3,	3,	3,	3,	3,
	0,	0,	0,	0,	3,	3,	3,	3,	3,	3,
	4,	0,	0,	0,	0,	0,	3,	3,	3,	3,
	4,	0,	0,	0,	0,	0,	3,	3,	3,	3,
	4,	4,	4,	4,	6,	6,	6,	6,	6,	6,
	4,	4,	0,	4,	5,	6,	6,	6,	6,	6,
	4,	4,	0,	4,	6,	6,	6,	6,	6,	6,
};
const RegionMapWidth=10;
const RegionMapHeight=10;

const RegionWidth=10;
const RegionHeight=10;
*/
const void (*RegionCallonFlip[])(void)=           {	&gs_RegionError,	&gs_DoNothing,};
const void (*RegionCallonEnter[])(void)=        {	&gs_RegionError,	&gs_DoNothing,};
const void (*RegionCallonExit[])(void)=           {	&gs_DoNothing,		&gs_DoNothing,};
const void (*RegionCallonFirstEnter[])(void)={	&gs_DoNothing,		&gs_DoNothing,};
#define STEP 0.05
#define STEPT 0.025
long CheckRegion()
{
  return 0;
}


void Input()
{
  ISWALKING=0;
  if(IsKeyDown(KEY_CTRL_UP))if(Deg<60)Deg+=3;
  if(IsKeyDown(KEY_CTRL_DOWN))if(Deg>3)Deg-=3;
  if(IsKeyDown(KEY_CTRL_LEFT)){Theta+=STEPT;if(Theta>2*pi)Theta-=(2*pi);cos_T=cosf(Theta);sin_T=sinf(Theta);}
  if(IsKeyDown(KEY_CTRL_RIGHT)){Theta-=STEPT;if(Theta<0)Theta+=(2*pi);cos_T=cosf(Theta);sin_T=sinf(Theta);}
  if(IsKeyDown(KEY_CHAR_PLUS))if(PIXELZOOM<500)PIXELZOOM+=5;
  if(IsKeyDown(KEY_CHAR_MINUS))if(PIXELZOOM>10)PIXELZOOM-=5;
  if(IsKeyDown(KEY_CHAR_8)){pos_X+=STEP*cos_T;pos_Y+=STEP*sin_T;ISWALKING=1;}
  if(IsKeyDown(KEY_CHAR_5)){pos_X-=STEP*cos_T;pos_Y-=STEP*sin_T;ISWALKING=1;}
  if(IsKeyDown(KEY_CHAR_4)){pos_X-=STEP*sin_T;pos_Y+=STEP*cos_T;ISWALKING=1;}
  if(IsKeyDown(KEY_CHAR_6)){pos_X+=STEP*sin_T;pos_Y-=STEP*cos_T;ISWALKING=1;}
  if(IsKeyDown(KEY_CTRL_AC))Reset_Calc();
}
void Init()
{
       //GrayLinkBuffers(&bufferdd1, &bufferdd2);  // Link them together
       //GrayInit(6800,3250);    // Load the timers with 3661
       //CPUSpeedQuadruple();//CPUSpeedTriple();////CPUSpeedDouble();
       PIXELZOOM=63;
       Deg=32;
       RENDERTIMER=0;
       WALKTIMER=0;

       pos_X=1.714502;pos_Y=-5.863173;pos_Z=.5;
       Theta=0;cos_T=cosf(Theta);sin_T=sinf(Theta);
}
//****************************************************************************
//  AddIn_main (Sample program main function)
//
//  param   :   isAppli   : 1 = This application is launched by MAIN MENU.
//                        : 0 = This application is launched by a strip in eACT application.
//
//              OptionNum : Strip number (0~3)
//                         (This parameter is only used when isAppli parameter is 0.)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************

int AddIn_main(int isAppli, unsigned short OptionNum)
{
    unsigned int key;
    int i;
    Init();

    while(1){    //Main Loop
        Input();
        LocalRegion=CheckRegion();


//Rendering
        memset(&buffer1, 0, 1024);
        memset(&buffer2, 0, 1024);
        CleanRQ();
        SetFloorSet(0,-pos_Z);

        w_map=3;h_map=5;Start_map=0;zoom_map=32;
        for(i=Deg;i<=63;i++)LineH_map(1,127,i);
        AddStaticWalls(0,21);//21
        AddStaticFloors(0,11);
        AddStaticObjects(0,7);
        Render();
        //RegionCallonFlip[0]();
        SetTexture(8);
        if(ISWALKING)RenderPicture(64,68-(int)(4*sinf(RENDERTIMER*2*pi/15)),1,1);
        else RenderPicture(64,68,1,1);

        PlotPixel(RENDERTIMER%128, 1, &buffer2, BLACK);
        PlotPixel((int)(Theta/2/pi*128), 1, &buffer2, BLACK);
        PlotPixel(RQNumber, 1, &buffer2, BLACK);

        for(i=0;i<1024;i++){
           bufferdd1[i]=buffer1[i];bufferdd2[i]=buffer2[i];  
        }
        RENDERTIMER++;
        if(ISWALKING)WALKTIMER++;

        DrawAll(&bufferdd2);
//End
    }
    while(1){
        GetKey(&key);
    }
      GrayEnd();
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


