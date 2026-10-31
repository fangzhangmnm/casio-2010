
void SetFloor(float D,float X0,float Z0,float dxx,float dzx,float dxy,float dzy)
{
     float t;
     t=((float)MULTI)/(dxx*dzy-dzx*dxy);
     f_A=D*dzy*t;   f_B=-t*(64.5*D*dzy+PIXELZOOM*D*dxy);   f_C=t*(-X0*dzy+Z0*dxy);
     f_D=-D*dzx*t;   f_E=t*(64.5*D*dzx+PIXELZOOM*D*dxx);   f_F=-t*(-X0*dzx+Z0*dxx);
}

void SetTexture(long pt);//    \/ needs.

void LineH_map(int l,int r,int y)
{
  long a1,b1,a2,b2,W,H,pB,X,Y;
  long a10,b10,a20,b20,W0,H0,X0,Y0;
  long pB2;
  long tmppt;
  float t,t0;
  int i;
  char mark,mark2,terrian,terrian0;

  if((Deg==y)||(y<1)||(y>63))return;
  if(l<1)l=1;if(r>127)r=127;
  t=1/(float)(Deg-y);
  t0=t/(float)zoom_map;

  W=(long)8<<(LNMULTI+w);H=(long)h<<LNMULTI;
  W0=(long)1<<(LNMULTI+w_map);H0=(long)h_map<<LNMULTI;

  a1=(long)(f_A*t)%W;b1=(long)(l*a1+f_B*t+f_C)%W;
  a2=(long)(f_D*t)%H;b2=(long)(l*a2+f_E*t+f_F)%H;
  a10=(long)(f_A*t0);b10=(long)(l*a10+f_B*t0+f_C/(float)zoom_map);
  a20=(long)(f_D*t0);b20=(long)(l*a20+f_E*t0+f_F/(float)zoom_map);

  if(a1<0)a1+=W;if(a2<0)a2+=H;if(b1<0)b1+=W;if(b2<0)b2+=H;

  terrian0=-1;

  for(i=l;i<=r;i++)
  {
    if((b10>=0)&&(b10<=W0)&&(b20>=0)&&(b20<=H0))
    {
        X0=b10>>LNMULTI;Y0=(b20>>LNMULTI)<<w_map;
        terrian=FloorMap[X0+Y0+Start_map];
        if(terrian!=terrian0){terrian0=terrian;SetTexture(FloorMapPointer[terrian]);}
        if(terrian){
            X=b1>>LNMULTI;Y=(b2>>LNMULTI)<<w;
            pB=(X>>3)+Y;mark=128>>(X&7);          //"(X>>3)+Y" != "X>>3+Y" !!!
            if((picturebuffer[picStart3+pB]&mark)){
                //PlotPixel(i, y, &buffer1,(picturebuffer[picStart1+pB]&mark)!=0);PlotPixel(i, y, &buffer2,(picturebuffer[picStart2+pB]&mark)!=0);
              pB2=(i>>3)+(y<<4);mark2=128>>(i&7);
              buffer1[pB2]=(picturebuffer[picStart1+pB]&mark)==0?buffer1[pB2]&~mark2:buffer1[pB2]|mark2;
              buffer2[pB2]=(picturebuffer[picStart2+pB]&mark)==0?buffer2[pB2]&~mark2:buffer2[pB2]|mark2;
            }
        }
    }
    b1+=a1;if(b1>=W) b1-=W;
    b2+=a2;if(b2>=H) b2-=H;
    b10+=a10;
    b20+=a20;
  }
}

void LineH(int l,int r,int y)
{
  long a1,b1,a2,b2,W,H,pB,X,Y,pB2;
  float t;
  int i;
  char mark,mark2;

  if((Deg==y)||(y<1)||(y>63))return;
  if(l<1)l=1;if(r>127)r=127;
  t=1/(float)(Deg-y);

  W=(long)8<<(LNMULTI+w);H=(long)h<<LNMULTI;
  a1=(long)(f_A*t)%W;b1=(long)(l*a1+f_B*t+f_C)%W;
  a2=(long)(f_D*t)%H;b2=(long)(l*a2+f_E*t+f_F)%H;
  if(a1<0)a1+=W;if(a2<0)a2+=H;if(b1<0)b1+=W;if(b2<0)b2+=H;

  for(i=l;i<=r;i++){
    X=b1>>LNMULTI;Y=(b2>>LNMULTI)<<w;
    pB=(X>>3)+Y;mark=128>>(X&7);          //"(X>>3)+Y" != "X>>3+Y" !!!
    if((picturebuffer[picStart3+pB]&mark)){
        //PlotPixel(i, y, &buffer1,(picturebuffer[picStart1+pB]&mark)!=0);PlotPixel(i, y, &buffer2,(picturebuffer[picStart2+pB]&mark)!=0);
      pB2=(i>>3)+(y<<4);mark2=128>>(i&7);
      buffer1[pB2]=(picturebuffer[picStart1+pB]&mark)==0?buffer1[pB2]&~mark2:buffer1[pB2]|mark2;
      buffer2[pB2]=(picturebuffer[picStart2+pB]&mark)==0?buffer2[pB2]&~mark2:buffer2[pB2]|mark2;
    }
    b1+=a1;if(b1>=W) b1-=W;
    b2+=a2;if(b2>=H) b2-=H;
  }
}

void Triangle(int X1,int Y1,int X2,int Y2,int X3,int Y3)
{
  int xmin,xmax,y,t;
  float tmp,xl,xr,dxl,dxr;

  if(Y1>Y2){t=Y1;Y1=Y2;Y2=t;t=X1;X1=X2;X2=t;}
  if(Y1>Y3){t=Y1;Y1=Y3;Y3=t;t=X1;X1=X3;X3=t;}
  if(Y2>Y3){t=Y2;Y2=Y3;Y3=t;t=X2;X2=X3;X3=t;}
  xmax=X1;xmin=X1;if(X2>xmax) xmax=X2; else if(X2<xmin) xmin=X2;if(X3>xmax) xmax=X3; else if(X3<xmin) xmin=X3;
  if(xmax>127)xmax=127;if(xmin<1)xmin=1;
  xl=X1;xr=X1;y=Y1;
  if(Y1!=Y2){
    dxl=(float)(X2-X1)/(float)(Y2-Y1);
    dxr=(float)(X3-X1)/(float)(Y3-Y1);
    if(dxl>dxr){tmp=dxl; dxl=dxr; dxr=tmp;}
    y=Y1;if(y<1)y=1;
    xl+=(y-Y1)*dxl;xr+=(y-Y1)*dxr;
    for(y=Y1;y<Y2;y++){
      LineH((xl>xmin?xl:xmin),(xr<xmax?xr:xmax),y);
      xl+=dxl;xr+=dxr;
    }
  } else {if(X2>X1)xr=X2; else xl=X2;}
  LineH((xl>xmin?xl:xmin),(xr<xmax?xr:xmax),Y2);
  if(Y2!=Y3){
    dxl=(float)(X3-X2)/(float)(Y3-Y2);
    dxr=(float)(X3-X1)/(float)(Y3-Y1);
    if(dxl<dxr){tmp=dxl;dxl=dxr;dxr=tmp;}
    if(Y3>63)Y3=63;
    for(y=Y2+1;y<=Y3;y++){
      LineH((xl>xmin?xl:xmin),(xr<xmax?xr:xmax),y);
      xl+=dxl;xr+=dxr;
    }
  }
}

void RenderPicture(int X0,int Y0,float ZoomX,float ZoomY)
{
     int x,y,xs,xe,ys,ye,X,Y;
     long a,a0,b,c,d,pB,pB2,p1,p2,p3;
     float xsf,ysf;
     char mark,mark2;

     c=(long)(MULTI/ZoomX);                                                  //not "*" but "/"   !!!
     d=(long)(MULTI/ZoomY); 
     xsf=X0-fabsf(ZoomX)*(4<<w);xs=ceilf(xsf);xe=floorf(X0+fabsf(ZoomX)*(4<<w));
     if(xs<1)xs=1;
     a0=(xs-xsf)*c;
     if(xe>127) xe=127;
     ysf=Y0-ZoomY*h;ys=ceilf(ysf);
     if(ys<1)ys=1;
     b=(ys-ysf)*d;
     ye=(Y0<=63?Y0:63);
     for(y=ys;y<=ye;y++){
        a=a0;Y=(b>>LNMULTI)<<w;
        p1=picStart1+Y;p2=picStart2+Y;p3=picStart3+Y;
        for(x=xs;x<=xe;x++){
           X=a>>LNMULTI;
           pB=(X>>3);mark=128>>(X&7);
           if((picturebuffer[p3+pB]&mark)){
                 //PlotPixel(x, y, &buffer1,(picturebuffer[p1+pB]&mark)!=0);PlotPixel(x, y, &buffer2,(picturebuffer[p2+pB]&mark)!=0);
               pB2=(x>>3)+(y<<4);mark2=128>>(x&7);
               buffer1[pB2]=(picturebuffer[p1+pB]&mark)==0?buffer1[pB2]&~mark2:buffer1[pB2]|mark2;
               buffer2[pB2]=(picturebuffer[p2+pB]&mark)==0?buffer2[pB2]&~mark2:buffer2[pB2]|mark2;
           }
           a+=c;
        }
        b+=d;
     }
}

void RenderWall(float X1,float Z1,float X2,float Z2,float Y1,float Y2,float Repeat)//,long picStart1,long picStart2,long picStart3, int w,int H)
{
    int Xs,Xe,Ye,t,Xrr,Yrr,X,Y;
    long W,H,pB,p1,p2,p3;
    long M,N;
    float dX,dY,dZ;
    float max;
    float A,B,C,D,D2,E,E2,F,G,P,Q;
    int ysm,yem;
    char val,mark;
    long pB2;
    char mark2;
  W=8<<w;H=h;

  //RANGE CHECKING
    if(Y1-Y2<SIGHTLIMIT)return;
    max=Z1>Z2?Z1:Z2;
    if(max<SIGHTLIMIT)return;
    if((Deg-PIXELZOOM*Y2/max<1)||(Deg-PIXELZOOM*Y1/max>63))return;

    Xs=TransferPoint2S(X1,Z1,X2,Z2);Xe=TransferPoint2S(X2,Z2,X1,Z1);
    if(Xs==Xe)return;
    if(Xs>Xe){t=Xs;Xs=Xe;Xe=t;}
    ysm=TransferPoint22S(Y1,Z1,1);t=TransferPoint22S(Y1,Z2,1);if(ysm>t)ysm=t;
    yem=TransferPoint22S(Y2,Z1,0);t=TransferPoint22S(Y2,Z2,0);if(yem<t)yem=t;
  //END

  //CONST CALCULATING
    dX=(X2-X1)/W/Repeat;dY=(Y2-Y1)/H;dZ=(Z2-Z1)/W/Repeat;

    A=-Z1*Xs+64*Z1+PIXELZOOM*X1;
    B=dZ*Xs-64*dZ-PIXELZOOM*dX;
    C=PIXELZOOM*(Z2*X1-Z1*X2)/W/Repeat;
    D=Deg-PIXELZOOM*Y1*(dZ*Xs-64*dZ-PIXELZOOM*dX)/C;
    D2=Deg-PIXELZOOM*Y2*(dZ*Xs-64*dZ-PIXELZOOM*dX)/C;
    E=-PIXELZOOM*Y1*dZ/C;
    E2=-PIXELZOOM*Y2*dZ/C;
    F=-MULTI*C/(PIXELZOOM*dY);                                                     //?
    G=-MULTI*(Y1/dY);

  //END

  //MAIN LOOP
   for(Xrr=Xs;Xrr<=Xe;Xrr++){
        X=(int)(A/B)%W;
        pB=X>>3;mark=128>>(X&7);
        p1=picStart1+pB;p2=picStart2+pB;p3=picStart3+pB;
        Yrr=ceilf(D);Ye=floorf(D2);
        if(Yrr<ysm)Yrr=ysm;
        if(Ye>yem)Ye=yem;
        pB2=(Xrr>>3)+(Yrr<<4);mark2=128>>(Xrr&7);

        N=(long)(P=F/B);
        M=(long)(P*(Yrr-Deg)+G);
        while(Yrr<=Ye){
            Y=M>>LNMULTI<<w;
            if((picturebuffer[p3+Y]&mark)){
               buffer1[pB2]=(picturebuffer[p1+Y]&mark)==0?buffer1[pB2]&~mark2:buffer1[pB2]|mark2;
               buffer2[pB2]=(picturebuffer[p2+Y]&mark)==0?buffer2[pB2]&~mark2:buffer2[pB2]|mark2;
                 //PlotPixel(Xrr, Yrr, &buffer1,(picturebuffer[p1+Y]&mark)!=0);PlotPixel(Xrr, Yrr, &buffer2,(picturebuffer[p2+Y]&mark)!=0);
            }
            Yrr+=1;M+=N;
            pB2+=16;
            //Bdisp_SetPoint_VRAM(Xrr,Yrr,wall[Y][X]);
        }
        A-=Z1;B+=dZ;D+=E;D2+=E2;
    }
  //END
}
