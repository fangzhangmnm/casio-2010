//function

#define FONT_BIG_HEIGHT 7
#define FONT_BIG_WIDTH 5
#define FONT_SMALL_HEIGHT 5

typedef enum {BIG,SMALL} FONT;

#define MAX_CLOCK_NUMBER 5

typedef struct{
	int x;
	int y;
	FONT font;
	TIMEFORMAT format;
	long int countdown;
} clock;
clock clockarray[MAX_CLOCK_NUMBER];
int nclock;
int alarm;
void clockevent()
{
	char buff[30];
	time t;
	int i,j,x;

	for(i=0;i<nclock;i++){
		switch(clockarray[i].format){
		case DATE_AND_TIME:
		case TIME:
			readtime(&t);
			writetime(buff,t,clockarray[i].format);
			x=clockarray[i].x;
		break;
		case COUNTDOWN:
			j=clockarray[i].countdown;
			if(j==0)alarm=1;
			getcountdown(&t,j);
			writetime(buff,t,clockarray[i].format);
			clockarray[i].countdown--;
		break;
		}
		if(clockarray[i].font==BIG)PrintXY(x,clockarray[i].y,buff,0);
		else PrintMini(x,clockarray[i].y,buff,0);
	}
	Bdisp_PutDisp_DD();
}
int addclock(int x,int y,FONT f,TIMEFORMAT tf,long int countdown)
{
	if(nclock>=MAX_CLOCK_NUMBER) return 1;
	clockarray[nclock].x=x;
	clockarray[nclock].y=y;
	clockarray[nclock].font=f;
	clockarray[nclock].format=tf;
	clockarray[nclock].countdown=countdown;
	nclock++;
	return 0;
}
void clearclock()
{
	nclock=0;
}
void enableclock()
{
	alarm=0;
	clockevent();
	SetTimer(ID_USER_TIMER1,1000,&clockevent);
}
void disableclock()
{
	KillTimer(ID_USER_TIMER1);
}
int ui_box(int x1,int y1,int x2,int y2)
{
	Bdisp_DrawLineVRAM(x1,y1,x1,y2);
	Bdisp_DrawLineVRAM(x1,y2,x2,y2);
	Bdisp_DrawLineVRAM(x2,y2,x2,y1);
	Bdisp_DrawLineVRAM(x2,y1,x1,y1);
}
int ui_button(int x1,int y1,int x2,int y2)
{
	unsigned int key;

	while(1){
		Bdisp_AreaReverseVRAM(x1,y1,x2,y2);
		GetKey(&key);
		Bdisp_AreaReverseVRAM(x1,y1,x2,y2);
		switch(key){
			case KEY_CTRL_EXE: return 0;
			case KEY_CTRL_UP: case KEY_CTRL_LEFT: return -1;
			case KEY_CTRL_DOWN: case KEY_CTRL_RIGHT: return 1;
		}
	}
}
int ui_textbox(char *buffer,int *init_length,int max_length,int x,int y,FONT font,int width,int password)
{
	unsigned int key;
	int i,ischar,height,length;
	char buff[30],puff[30];
	char *outbuff;
	DISPBOX area;

	if(width==0 && font==BIG) width=max_length*(FONT_BIG_WIDTH+1);
	if(font==BIG) height=FONT_BIG_HEIGHT; else height=FONT_SMALL_HEIGHT;
	area.left=x;area.top=y;area.right=x+width-1;area.bottom=y+height-1;
	if(password) outbuff=puff; else outbuff=buff;
	length=*init_length;
	for(i=0;i<length;i++){
		buff[i]=buffer[i];puff[i]='*';
	}
	buff[length]=0;puff[length]=0;
	key=ischar=0;
	while(key!=KEY_CTRL_EXE && key!=KEY_CTRL_EXIT && key!=KEY_CTRL_UP && key!= KEY_CTRL_DOWN){
		if(length<max_length){
			buff[length]='_';puff[length]='_';
			buff[length+1]=0;puff[length+1]=0;
		}else{
			buff[length]=0;puff[length]=0;
		}
		if(font==BIG)PrintXY(x,y,outbuff,0);
		else PrintMini(x,y,outbuff,MINI_OVER);
		ischar=GetKey(&key);
		Bdisp_AreaClr_DDVRAM(&area);

		if(key==KEY_CTRL_AC) length=0;
		if(key==KEY_CTRL_DEL && length>0) length--;
		if(ischar && isalnum(key) && length<max_length){
			buff[length]=key;puff[length]='*';length++;
		}
	}

	if(key!=KEY_CTRL_EXIT){
		for(i=0;i<length;i++) buffer[i]=buff[i];
		buffer[length]=0;buff[length]=0;puff[length]=0;
		*init_length=length;
	}else{
		length=*init_length;
		for(i=0;i<length;i++){
			buff[i]=buffer[i];puff[i]='*';
		}
		buff[length]=0;puff[length]=0;
	}
	if(font==BIG)PrintXY(x,y,outbuff,0);
	else PrintMini(x,y,outbuff,MINI_OVER);
	switch(key){
		case KEY_CTRL_EXIT:
			return 4;
		case KEY_CTRL_UP:
			return -1;
		case KEY_CTRL_EXE:
		case KEY_CTRL_DOWN:
			return 1;
	}
}

//ui 
typedef enum {INDEX,REGISTER1,REGISTER2} UI_NOW;
UI_NOW ui_now;
void ui_init()
{
	db_reset_game();
}
void ui_index()
{
	unsigned int key;
	char username[MAX_STRING_LENGTH+1],password[MAX_STRING_LENGTH+1];
	int usernamel=0,passwordl=0;
	int index=0,move;

	Bdisp_AllClr_DDVRAM();
	PrintXY(1,1,"OGAME",0);PrintMini(31,2," - Conquer the universe",0);
	Bdisp_DrawLineVRAM(0,9,52,9);
	PrintXY(3,11,"Username",0);
	ui_box(1,19,126,29);//PrintXY(3,21,"Username",0);
	PrintXY(3,31,"Password",0);
	ui_box(1,39,126,49);//PrintXY(3,41,"********",0);
	ui_box(6,51,57,61);PrintXY(8,53,"Login",0);
	ui_box(70,51,121,61);PrintXY(72,53,"Register",0);

	clearclock();
	addclock(54,9,SMALL,DATE_AND_TIME,0);
	enableclock();

	move=1;
	while(move){
		switch(index){
		case 0:
			move=ui_textbox(username,&usernamel,MAX_STRING_LENGTH,3,21,BIG,122,0);
		break;case 1:
			move=ui_textbox(password,&passwordl,MAX_STRING_LENGTH,3,41,BIG,122,0);
		break;case 2:
			move=ui_button(6,51,57,61);
		break;case 3:
			move=ui_button(70,51,121,61);
		break;
		}
		index=(index+move+4)%4;
	}
	disableclock();
	clearclock();

	switch(index){
	case 2:
		if(ru_login(username,password)) PrintXY(0,0,"Logging Success",0);
		else PrintXY(0,0,"Logging Fail",0);
	break;
	case 3:
		ui_now=REGISTER1;
	break;
	}
}
void ui_preview()
{
	
}