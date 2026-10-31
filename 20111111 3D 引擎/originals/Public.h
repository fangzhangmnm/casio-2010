#define SIGHTLIMIT 0.05
#define FLOATLIMIT 0.0001
int PIXELZOOM;

const float pi=3.14159265358979323846264338327950288;
//===Region
long LocalRegion;
//===End
//===VRAM
long RENDERTIMER;
long WALKTIMER;
float LASTTIME;
int ISWALKING;

unsigned char buffer1[1024]; 
unsigned char buffer2[1024];
unsigned char bufferdd1[1024]; 
unsigned char bufferdd2[1024];
//===End
//===Creature

//===End
//===Position
int Deg;
float pos_X,pos_Y,pos_Z,Theta,cos_T,sin_T;
//===End

//===RenderQueue
#define MAXRENDERQUEUE 100

long RQPointer[MAXRENDERQUEUE];
char RQType[MAXRENDERQUEUE];                                    //RenderQueue Type:   0-None   1-Static Wall   2-Static Floor Triangle
int RQNumber;
//===End

//===Painter
#define MULTI 1048576
#define LNMULTI 20

float f_A,f_B,f_C,f_D,f_E,f_F;

int Brush,w,h;
int w_map,h_map,zoom_map;
long Start_map;
long picStart1,picStart2,picStart3;
//===End

//===Dyamic
#define MAXPOINTDS 30
//30=20+2*5
float PointDX[MAXPOINTDS];
float PointDY[MAXPOINTDS];
int PointDNum;

#define MAXOBJECTDS 20
long ObjectDPoint[MAXOBJECTDS];
long ObjectDTexture[MAXOBJECTDS];
float ObjectDY[MAXOBJECTDS];
float ObjectDH[MAXOBJECTDS];
float ObjectDW[MAXOBJECTDS];
int ObjectDNum;

#define MAXWALLDS 5
long WallDLeftPoint[MAXWALLDS];
long WallDRightPoint[MAXWALLDS];
float WallDY1[MAXWALLDS];
float WallDY2[MAXWALLDS];
float WallDRepeat[MAXWALLDS];
long WallDTexture[MAXWALLDS];
int WallDNum;

//===End

int GetABC(float x1,float y1,float x2,float y2,float *A,float *B,float *C)
{*A=y2-y1;*B=x1-x2;*C=x2*y1-x1*y2;
if(*C<0){*A=-*A;*B=-*B;*C=-*C;}}//if (x1,y1)=(x2,y2) then ABC=(0,0,0)!!

void TransferPoint1S(long i,float *x,float *z)
{
   float x1,y1;
   if(i>=0){x1=PointX[i]-pos_X;y1=PointY[i]-pos_Y;}
         else{i=-i-1;x1=PointDX[i]-pos_X;y1=PointDY[i]-pos_Y;}
   *x=x1*sin_T-y1*cos_T;
   *z=x1*cos_T+y1*sin_T;
}
int TransferPoint2S(float x,float z,float x2,float z2)
{  
    if(z>FLOATLIMIT){
        if(PIXELZOOM*x>(127-64)*z)return 127;
        else if(PIXELZOOM*x<(1-64)*z)return 1;
        else return(int)(64+PIXELZOOM*x/z);
    }else{
        return ((z2*x-z*x2)>0)==(z2-z>0)?127:1;
    }
}
int TransferPoint20S(float x,float z)
{  
    if(z>FLOATLIMIT)return(int)(64+PIXELZOOM*x/z);
    else return 0;
}
int TransferPoint21S(float x,float z,float x2,float z2)
{  
    if(z>FLOATLIMIT)return(int)(64+PIXELZOOM*x/z);
    else if(fabsf(z2-z)>FLOATLIMIT)return (int)(64+PIXELZOOM*((z2-SIGHTLIMIT)*x-(z-SIGHTLIMIT)*x2)/SIGHTLIMIT/(z2-z));
    else return 0;
}
int TransferPoint22S(float y,float z,int ceilorfloor)
{
    if(z>FLOATLIMIT){
        if(PIXELZOOM*y>(Deg-1)*z)return 1;
        else if(PIXELZOOM*y<(Deg-63)*z)return 63;
        else return ceilorfloor?ceilf(Deg-PIXELZOOM*y/z):floorf(Deg-PIXELZOOM*y/z);
    }else{
        if(y>0)return 1; else return 63;
    }
}
int TransferPoint23S(float y,float z)
{
    if(z>FLOATLIMIT)return (int)(Deg-PIXELZOOM*y/z);
    else if(y>0)return 1; else return 63;
}
void GetAB(int *A,int *B,char type,float x1,float z1,float x2,float z2,float x3,float z3,float d2)
{
   int a,b,t;
   float tf;
   if(type==1){a=TransferPoint2S(x1,z1,x2,z2);b=TransferPoint2S(x2,z2,x1,z1);}
   if(type==2){
      if(z1<z2){tf=z1;z1=z2;z2=tf;tf=x1;x1=x2;x2=tf;}
      if(z1<z3){tf=z1;z1=z3;z3=tf;tf=x1;x1=x3;x3=tf;} //z1 is max
      if(z1<0){a=0;b=0;}
      else{  //x1,z1>0!
         a=TransferPoint2S(x1,z1,0,0);b=a;
         t=TransferPoint2S(x2,z2,x1,z1);if(t<a)a=t; else if(t>b)b=t;
         t=TransferPoint2S(x3,z3,x1,z1);if(t<a)a=t; else if(t>b)b=t;
      }
   }
   if(type==3){a=TransferPoint2S(x1-d2,z1,0,0);b=TransferPoint2S(x1+d2,z1,0,0);}
   if(a<b){*A=a;*B=b;}else{*A=b;*B=a;}
}
