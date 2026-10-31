#define MAX_CONTROL_NUMBER 50
#define MINI_WIDTH 3
#define MINI_HEIGHT 5
#define BIG_WIDTH 5
#define BIG_HEIGHT 7
#define MAX_WIDTH 128
#define MAX_PRINT_BUFFER 33
//MAX_WIDTH/(MINI_WIDTH+1)+1

typedef enum{NULL_TYPE,CAPTION,PASSAGE} vf_CONTROLTYPE;
typedef struct{
	int width;
	int height;
	void (*printptr)(int x,int y,char* str); 
} vf_FONT;
typedef struct{
	vf_CONTROLTYPE type;
	int x;int y;int width;int height;
	int font;
	char *text;
} vf_CONTROL;

vf_CONTROL vf_control[MAX_CONTROL_NUMBER];

void vf_refresh()
{
	int i,j,k,l,x,y,yr,w,h;
	char printbuff[MAX_PRINT_BUFFER];

	Bdisp_AllClr_VRAM();
	for(i=0;i<MAX_CONTROL_NUMBER;i+=1){
		x=vf_control[i].x;y=vf_control[i].y;
		w=vf_control[i].width;h=vf_control[i].height;
		switch(vf_control[i].type){
			case CAPTION:
				l=(vf_control[i].width+1)/(BIG_WIDTH+1);
				j=0;
				while(vf_control[i].text[j] && j<l){printbuff[j]=vf_control[i].text[j];j++;}
				printbuff[j]=0;
				PrintXY(x,y,printbuff,0);
			break;
			case PASSAGE:
				l=(vf_control[i].width+1)/(MINI_WIDTH+1);
				j=0;yr=0;
				while(vf_control[i].text[j] && yr+MINI_HEIGHT<=vf_control[i].height){
					k=0;
					while(vf_control[i].text[j] && k<l){printbuff[k]=vf_control[i].text[j];j++;k++;}
					printbuff[k]=0;
					PrintMini(x,y+yr,printbuff,0);
					yr+=MINI_HEIGHT+1;
				}
			break;
		}//switch
	}//for
	Bdisp_PutDisp_DD();
}
int vf_create()
{
	int i;
	for(i=0;i<MAX_CONTROL_NUMBER;i+=1)
		if(vf_control[i].type==NULL_TYPE) return i;
	return -1;
}
int vf_addcaption(int x,int y,int width,char *text)
{
	int i;
	if((i=vf_create())==-1) return -1;
	vf_control[i].type=CAPTION;
	vf_control[i].x=x;vf_control[i].y=y;vf_control[i].width=width;vf_control[i].text=text;
	vf_control[i].height=BIG_HEIGHT;
	return i;
}
int vf_addpassage(int x,int y,int width,int height,char *text)
{
	int i;
	if((i=vf_create())==-1) return -1;
	vf_control[i].type=PASSAGE;
	vf_control[i].x=x;vf_control[i].y=y;vf_control[i].width=width;vf_control[i].height=height;vf_control[i].text=text;
	return i;
}

