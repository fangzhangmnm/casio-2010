 void SetTexture(long pt)
{
  if(pt>=0)
  {
    picStart1=TextureLight[pt];picStart2=TextureDark[pt];picStart3=TextureAlpha[pt];w=TextureWidth[pt];h=TextureHeight[pt];
  }else{//(pt<0)
    SetTexture(TextureCartoon[-pt+1+(RENDERTIMER/TextureCartoon[-pt])%TextureCartoon[-pt-1]]);
  }
}
void SetFloorSet(long i,float D)
{
  float x0,z0,x1,z1,x2,z2;
  SetTexture(FloorSetTexture[i]);
  TransferPoint1S(FloorSetP0[i],&x0,&z0);TransferPoint1S(FloorSetDPx[i],&x1,&z1);TransferPoint1S(FloorSetDPy[i],&x2,&z2);
  SetFloor(D,x0,z0,x1-x0,z1-z0,x2-x0,z2-z0);
}
void AddStaticObjects(long Start,long Number)
{
     long i,tmppp;
     float x1,z1;
     for(i=0;i<Number;i++){
          tmppp=Start+i;
          TransferPoint1S(ObjectPoint[tmppp],&x1,&z1);
          if(z1>SIGHTLIMIT){
              RQType[RQNumber]=3;
              RQPointer[RQNumber]=Start+i;
              RQNumber++;
          }
     }
}
void AddStaticWalls(long Start,long Number)
{
     long i,tmppp;
     float x1,z1,x2,z2;
     for(i=0;i<Number;i++){
          tmppp=Start+i;
          TransferPoint1S(WallLeftPoint[tmppp],&x1,&z1);
          TransferPoint1S(WallRightPoint[tmppp],&x2,&z2);
          if(!((TransferPoint2S(x1,z1,x2,z2)==TransferPoint2S(x2,z2,x1,z1))
               ||(z1<SIGHTLIMIT&&z2<SIGHTLIMIT))){
              RQType[RQNumber]=1;
              RQPointer[RQNumber]=Start+i;
              RQNumber++;
          }
     }
}
void AddStaticFloors(long Start,long Number)
{
     long i,tmppp;
     float x1,z1,x2,z2,x3,z3;
     for(i=0;i<Number;i++){
          tmppp=Start+i;
          TransferPoint1S(FloorPoint1[tmppp],&x1,&z1);
          TransferPoint1S(FloorPoint2[tmppp],&x2,&z2);
          TransferPoint1S(FloorPoint3[tmppp],&x3,&z3);

          if((TransferPoint2S(x1,z1,x2,z2)!=TransferPoint2S(x2,z2,x1,z1)   //which ought kick out the case that z1,z2&z3 <0 ?
           ||TransferPoint2S(x1,z1,x3,z3)!=TransferPoint2S(x3,z3,x1,z1)
           ||TransferPoint2S(x3,z3,x2,z2)!=TransferPoint2S(x2,z2,x3,z3))
           &&!(z1<SIGHTLIMIT&&z2<SIGHTLIMIT&&z3<SIGHTLIMIT)){
              RQType[RQNumber]=2;
              RQPointer[RQNumber]=Start+i;
              RQNumber++;
          }
     }
}
int AddDyamicPoint(float x1,float y1,long *pt)
{
    if(PointDNum>=MAXPOINTDS)return 1;
    PointDX[PointDNum]=x1;PointDY[PointDNum]=y1;
    *pt=PointDNum;
    PointDNum++;
    return 0;
}
int AddDyamicObject(long pt,float x1,float y1,float z1,float width1,float height1,long pTexture)//New Position=-1
{
  if(ObjectDNum>=MAXOBJECTDS)return 1;
  if(pt==-1)if(AddDyamicPoint(x1,y1,&pt))return 1;
  ObjectDPoint[ObjectDNum]=pt;
  ObjectDTexture[ObjectDNum]=pTexture;
  ObjectDY[ObjectDNum]=z1;ObjectDW[ObjectDNum]=width1;ObjectDH[ObjectDNum]=height1;
  ObjectDNum++;
  return 0;
}
int AddDyamicWall(long pt1,float x1,float y1,long pt2,float x2,float y2,float z1,float z2,float repeat1,long pTexture)//New Position=-1
{
  if(WallDNum>=MAXWALLDS)return 1;
  if(pt1==-1&&pt2==-1&&PointDNum+2>MAXPOINTDS)return 1;
  if(pt1==-1)if(AddDyamicPoint(x1,y1,&pt1))return 1;
  if(pt2==-1)if(AddDyamicPoint(x2,y2,&pt2))return 1;
  WallDLeftPoint[WallDNum]=pt1;WallDRightPoint[WallDNum]=pt2;
  WallDY1[WallDNum]=z1;WallDY2[WallDNum]=z2;
  WallDRepeat[WallDNum]=repeat1;WallDTexture[WallDNum]=pTexture;
  WallDNum++;
  return 0;
}
void AddDyamicToRQ()
{
     long i;
     float x1,z1,x2,z2,x3,z3;
     for(i=0;i<ObjectDNum;i++){
          TransferPoint1S(ObjectDPoint[i],&x1,&z1);
          if(z1>SIGHTLIMIT){
              RQType[RQNumber]=-3;
              RQPointer[RQNumber]=i;
              RQNumber++;
          }
     }
     for(i=0;i<WallDNum;i++){
          TransferPoint1S(WallDLeftPoint[i],&x1,&z1);
          TransferPoint1S(WallDRightPoint[i],&x2,&z2);
          if(!((TransferPoint2S(x1,z1,x2,z2)==TransferPoint2S(x2,z2,x1,z1))
               ||(z1<SIGHTLIMIT&&z2<SIGHTLIMIT))){
              RQType[RQNumber]=-1;
              RQPointer[RQNumber]=i;
              RQNumber++;
          }
     }
}
void CleanRQ(){	RQNumber=0;}
void CleanDyamic(){	ObjectDNum=0;	PointDNum=0;	WallDNum=0;}
void LoadRenderItem(long i,float *x1,float *z1,float *x2,float *z2,float *x3,float *z3,float *y1,float *y2,char *type,long *pt,float *d1,float *d2)
{
    long p;
    p=RQPointer[i];
    switch(RQType[i]){
      case 1:                                              //Static Wall
        *type=1;*y1=WallY1[p]-pos_Z;*y2=WallY2[p]-pos_Z;
        TransferPoint1S(WallLeftPoint[p],x1,z1);TransferPoint1S(WallRightPoint[p],x2,z2);  //x3=x1;z3=z1;
        *pt=WallTexture[p];*d1=WallRepeat[p];
      break;
      case 2:                                              //Static Floor
        *type=2;*y1=FloorD[p]-pos_Z;*y2=*y1;
        TransferPoint1S(FloorPoint1[p],x1,z1);TransferPoint1S(FloorPoint2[p],x2,z2);TransferPoint1S(FloorPoint3[p],x3,z3);
        *pt=FloorSet[p];*d1=FloorD[p];
      break;
      case 3:                                              //Static Object
        *type=3;*y1=ObjectY[p]+ObjectH[p]-pos_Z;*y2=ObjectY[p]-pos_Z;
        TransferPoint1S(ObjectPoint[p],x1,z1);  //x2=x1;z2=z1;x3=x1;z3=z1;
        *pt=ObjectTexture[p];*d1=ObjectH[p];*d2=ObjectW[p]/2;
      break;
      case -1:                                              //Dyamic Wall
        *type=1;*y1=WallDY1[p]-pos_Z;*y2=WallDY2[p]-pos_Z;
        TransferPoint1S(WallDLeftPoint[p],x1,z1);TransferPoint1S(WallDRightPoint[p],x2,z2);  //x3=x1;z3=z1;
        *pt=WallDTexture[p];*d1=WallDRepeat[p];
      break;
      case -3:                                              //Dyamic Object
        *type=3;*y1=ObjectDY[p]+ObjectDH[p]-pos_Z;*y2=ObjectDY[p]-pos_Z;
        TransferPoint1S(ObjectDPoint[p],x1,z1);  //x2=x1;z2=z1;x3=x1;z3=z1;
        *pt=ObjectDTexture[p];*d1=ObjectDH[p];*d2=ObjectDW[p]/2;
      break;
    }
}
