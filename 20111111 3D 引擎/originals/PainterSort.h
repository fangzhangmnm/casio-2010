 void Render()
{
float maxZ[MAXRENDERQUEUE],minZ[MAXRENDERQUEUE];
char flags[MAXRENDERQUEUE];
long i,j,k,l,pt;
float tmpmax,tmpmin,tmpswap,x1,z1,x2,z2,x3,z3,x4,z4,x5,z5,x6,z6,yo1,yo2,ys1,ys2,ds1,do1,ds2,do2;
float A1,B1,C1,A2,B2,C2,A3,B3,C3,A,B,C;
int a,b,c,d;
char flag,types,typeo;
float floatlimit;

sin_T=sinf(Theta);cos_T=cosf(Theta);
memset(&flags, 0, MAXRENDERQUEUE);
                                                                         //1st: Sort the Polygons at Zmax from Smallest to Biggest
   for(i=0;i<RQNumber;i++){
       LoadRenderItem(i,&x1,&z1,&x2,&z2,&x3,&z3,&ys1,&ys2,&types,&pt,&ds1,&ds2);
       tmpmax=z1;tmpmin=z1;
       if(types!=3){if(tmpmax<z2)tmpmax=z2; else tmpmin=z2;}
       if(types==2){if(tmpmax<z3)tmpmax=z3;else if(tmpmin>z3)tmpmin=z3;}
       maxZ[i]=tmpmax;minZ[i]=tmpmin;
   }

   flag=1;
   for(i=0;(i<RQNumber-1)&&flag;i++){
      flag=0;
      for(j=RQNumber-1;j>i;j--)
         if(maxZ[j]<maxZ[j-1]){
            tmpswap=maxZ[j];maxZ[j]=maxZ[j-1];maxZ[j-1]=tmpswap;
            tmpswap=minZ[j];minZ[j]=minZ[j-1];minZ[j-1]=tmpswap;
            k=RQPointer[j];RQPointer[j]=RQPointer[j-1];RQPointer[j-1]=k;
            k=RQType[j];RQType[j]=RQType[j-1];RQType[j-1]=k;
            flag=1;
        }
   }

//TODOTODOTODOTODO          TODOTODOTODO

while(RQNumber>0){
                                                                           //2nd
    flag=1;i=RQNumber-1;

    LoadRenderItem(i,&x1,&z1,&x2,&z2,&x3,&z3,&ys1,&ys2,&types,&pt,&ds1,&ds2);
    GetAB(&a,&b,types,x1,z1,x2,z2,x3,z3,ds2);
    if(types!=3)GetABC(x1,z1,x2,z2,&A1,&B1,&C1);
    if(types==2){GetABC(x1,z1,x3,z3,&A2,&B2,&C2);GetABC(x3,z3,x2,z2,&A3,&B3,&C3);}

    for(j=0;(j<RQNumber-1)&&flag;j++)
    if(maxZ[j]<minZ[i]) continue; else{

        LoadRenderItem(j,&x4,&z4,&x5,&z5,&x6,&z6,&yo1,&yo2,&typeo,&k,&do1,&do2);

        if(ys2+FLOATLIMIT>=yo1&&yo1+FLOATLIMIT>=0
        || ys1<=yo2+FLOATLIMIT&&yo2<=FLOATLIMIT
        || ys2>0&&yo1<0 || ys1<0&&yo2>0 )          /*!*/
         continue;													//CHECKOUT (2)

        GetAB(&c,&d,typeo,x4,z4,x5,z5,x6,z6,do2);
        if(b<=c||d<=a) continue;											//CHECKOUT (1)

if(types==3){//Object
  if(typeo==3){//to Object
    //z4 always >z1
      flag=0;break;                 //j--;? no.
  }
  if(typeo==1){//to Wall
    GetABC(x4,z4,x5,z5,&A,&B,&C);
    if((A*x1+B*z1+C>FLOATLIMIT)){flag=0;break;}		 //CHECKOUT (3)
  }
  if(typeo==2){//to Floor
    //maxZ[j] always<minZ[i]
    GetABC(x4,z4,x5,z5,&A,&B,&C);
    if((A*x1+B*z1+C<=FLOATLIMIT))continue;			 //CHECKOUT (3)
    GetABC(x4,z4,x6,z6,&A,&B,&C);
    if((A*x1+B*z1+C<=FLOATLIMIT))continue;			 //CHECKOUT (3)
    GetABC(x6,z6,x5,z5,&A,&B,&C);
    if((A*x1+B*z1+C<=FLOATLIMIT))continue;			 //CHECKOUT (3)
    flag=0;break;
  }
}//Object
if(types==1){//Wall
  if(typeo==3){//to Object
    if(A1*x4+B1*z4+C1<-FLOATLIMIT){flag=0;break;}		 //CHECKOUT (4)
  }
  if(typeo==1){//to Wall
    if( (A1*x4+B1*z4+C1>=-FLOATLIMIT) && (A1*x5+B1*z5+C1>=-FLOATLIMIT) ) continue;	//CHECKOUT (4)
    GetABC(x4,z4,x5,z5,&A,&B,&C);
    if( (A*x1+B*z1+C<=FLOATLIMIT) && (A*x2+B*z2+C<=FLOATLIMIT) )  continue;			//CHECKOUT (3)
    flag=0;break;
  }
  if(typeo==2){//to Floor
    if( (A1*x4+B1*z4+C1>=-FLOATLIMIT) && (A1*x5+B1*z5+C1>=-FLOATLIMIT)
       && (A1*x6+B1*z6+C1>=-FLOATLIMIT)) continue;					//CHECKOUT (4)
    floatlimit=((yo1+FLOATLIMIT>=ys1)&&(yo1+FLOATLIMIT>=ys2)
                      ||(yo1<=FLOATLIMIT+ys1)&&(yo1<=FLOATLIMIT+ys2))?FLOATLIMIT:-FLOATLIMIT;            //if wall is completely beneath|above the floor,0 will still shadow it!
    GetABC(x4,z4,x5,z5,&A,&B,&C);
    if((A*x1+B*z1+C<=-floatlimit)&&(A*x2+B*z2+C<=-floatlimit))continue;		//CHECKOUT (3)
    GetABC(x4,z4,x6,z6,&A,&B,&C);
    if((A*x1+B*z1+C<=-floatlimit)&&(A*x2+B*z2+C<=-floatlimit))continue;		//CHECKOUT (3)
    GetABC(x6,z6,x5,z5,&A,&B,&C);
    if((A*x1+B*z1+C<=-floatlimit)&&(A*x2+B*z2+C<=-floatlimit))continue;		//CHECKOUT (3)
    flag=0;break;
  }
}//Wall
if(types==2){//Floor
  if(typeo==3){//to Object
    //maxZ[j] always<minZ[i]
    if(A1*x4+B1*z4+C1>=-FLOATLIMIT)continue;			 //CHECKOUT (4)
    if(A2*x4+B2*z4+C2>=-FLOATLIMIT)continue;			 //CHECKOUT (4)
    if(A3*x4+B3*z4+C3>=-FLOATLIMIT)continue;			 //CHECKOUT (4)
    flag=0;break;
  }
  if(typeo==1){//to Wall
    floatlimit=((yo1+FLOATLIMIT>=ys1)&&(yo1+FLOATLIMIT>=ys2)
                      ||(yo1<=FLOATLIMIT+ys1)&&(yo1<=FLOATLIMIT+ys2))?FLOATLIMIT:-FLOATLIMIT;            //if wall is completely beneath|above the floor,0 will still shadow it!
    if( (A1*x4+B1*z4+C1>=floatlimit) && (A1*x5+B1*z5+C1>=floatlimit)
       ||(A2*x4+B2*z4+C2>=floatlimit) && (A2*x5+B2*z5+C2>=floatlimit)
       ||(A3*x4+B3*z4+C3>=floatlimit) && (A3*x5+B3*z5+C3>=floatlimit)) continue;		//CHECKOUT (4)
    GetABC(x4,z4,x5,z5,&A,&B,&C);
    if( (A*x1+B*z1+C<=FLOATLIMIT) && (A*x2+B*z2+C<=FLOATLIMIT)
      && (A*x3+B*z3+C<=FLOATLIMIT) )  continue;								//CHECKOUT (3)
    flag=0;break;
  }
  if(typeo==2){//to Floor
    if( (A1*x4+B1*z4+C1>=-FLOATLIMIT) && (A1*x5+B1*z5+C1>=-FLOATLIMIT) && (A1*x6+B1*z6+C1>=-FLOATLIMIT)
       ||(A2*x4+B2*z4+C2>=-FLOATLIMIT) && (A2*x5+B2*z5+C2>=-FLOATLIMIT) && (A2*x6+B2*z6+C2>=-FLOATLIMIT)
       ||(A3*x4+B3*z4+C3>=-FLOATLIMIT) && (A3*x5+B3*z5+C3>=-FLOATLIMIT) && (A3*x6+B3*z6+C3>=-FLOATLIMIT)) continue;	//CHECKOUT (4)
    GetABC(x4,z4,x5,z5,&A,&B,&C);
    if((A*x1+B*z1+C>FLOATLIMIT)||(A*x2+B*z2+C>FLOATLIMIT)||(A*x3+B*z3+C>FLOATLIMIT)){flag=0;break;}	//CHECKOUT (3)
    GetABC(x4,z4,x6,z6,&A,&B,&C);
    if((A*x1+B*z1+C>FLOATLIMIT)||(A*x2+B*z2+C>FLOATLIMIT)||(A*x3+B*z3+C>FLOATLIMIT)){flag=0;break;}	//CHECKOUT (3)
    GetABC(x6,z6,x5,z5,&A,&B,&C);
    if((A*x1+B*z1+C>FLOATLIMIT)||(A*x2+B*z2+C>FLOATLIMIT)||(A*x3+B*z3+C>FLOATLIMIT)){flag=0;break;}	//CHECKOUT (3)
  }
}//Floor
    }//if&for

    //if(flags[i]){
       //Reset_Calc();                                                        //TODO
     //}

     if(flag||flags[i]){                      //Render It
       if(types==1){
            SetTexture(pt);
            RenderWall(x1,z1,x2,z2,ys1,ys2,ds1);
       }
       if(types==2){
            SetFloorSet(pt,ys1);
            if(z1<z2){tmpswap=z1;z1=z2;z2=tmpswap;tmpswap=x1;x1=x2;x2=tmpswap;}
            if(z1<z3){tmpswap=z1;z1=z3;z3=tmpswap;tmpswap=x1;x1=x3;x3=tmpswap;}
            if(z2<z3){tmpswap=z2;z2=z3;z3=tmpswap;tmpswap=x2;x2=x3;x3=tmpswap;}

            if(ys1>0)z5=PIXELZOOM*ys1/(Deg-1);else z5=PIXELZOOM*ys1/(Deg-63);
            z6=z5;
            if(z1<z5) ;
            else if(z2<z5){
               x5=x1+(x2-x1)*(z5-z1)/(z2-z1);x6=x1+(x3-x1)*(z6-z1)/(z3-z1);
               a=TransferPoint21S(x5,z5,x1,z1);b=TransferPoint21S(x6,z6,x1,z1);
               if(ys1>0) Triangle(TransferPoint20S(x1,z1),TransferPoint23S(ys1,z1),a,1,b,1);
               else Triangle(TransferPoint20S(x1,z1),TransferPoint23S(ys1,z1),a,63,b,63);

            }else if(z3<z5){
               x5=x1+(x3-x1)*(z5-z1)/(z3-z1);x6=x2+(x3-x2)*(z6-z2)/(z3-z2);
               a=TransferPoint21S(x5,z5,x1,z1);b=TransferPoint21S(x6,z6,x2,z2);
               if(ys1>0)
                  {Triangle(TransferPoint20S(x1,z1),TransferPoint23S(ys1,z1),a,1,TransferPoint20S(x2,z2),TransferPoint23S(ys1,z2));Triangle(TransferPoint20S(x2,z2),TransferPoint23S(ys1,z2),a,1,b,1);}
               else
                  {Triangle(TransferPoint20S(x1,z1),TransferPoint23S(ys1,z1),a,63,TransferPoint20S(x2,z2),TransferPoint23S(ys1,z2));Triangle(TransferPoint20S(x2,z2),TransferPoint23S(ys1,z2),a,63,b,63);}
            }else
            Triangle(TransferPoint20S(x1,z1),TransferPoint23S(ys1,z1),TransferPoint20S(x2,z2),TransferPoint23S(ys1,z2),TransferPoint20S(x3,z3),TransferPoint23S(ys1,z3));
       }//if(type==2)

     memset(&flags, 0, MAXRENDERQUEUE);
     RQNumber--;
//DrawAll(buffer2);
       if(types==3){
            SetTexture(pt);
            RenderPicture(TransferPoint20S(x1,z1),TransferPoint23S(ys2,z1),PIXELZOOM*ds2/z1/(float)(4<<w),PIXELZOOM*ds1/z1/(float)h);
       }
     }else{
            flags[i]=flags[j];flags[j]=1;                                 //i !     >i< not j!!!   qian yi l jiu bu ke yi wang hou yi! Don't Come back again!!!!!!!! and they will be exchanged !!!!

            tmpswap=maxZ[j];maxZ[j]=maxZ[i];maxZ[i]=tmpswap;                 //Exchange
            tmpswap=minZ[j];minZ[j]=minZ[i];minZ[i]=tmpswap;
            k=RQPointer[j];RQPointer[j]=RQPointer[i];RQPointer[i]=k;
            k=RQType[j];RQType[j]=RQType[i];RQType[i]=k;
     }
}//while
}//void Render
