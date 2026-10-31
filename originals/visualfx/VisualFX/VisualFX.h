/////////////////////////////font
typedef void (*vf_PUTSTRING)(int,int,int,char*);
typedef struct {
	int width;int height;
	int multi;
	vf_PUTSTRING putstring;
}vf_FONT;

vf_FONT*	vf_addfont(int width,int height,int multi,vf_PUTSTRING putstring);
void		vf_freefont(vf_FONT* font);
vf_FONT*	vf_adddefaultfont();
vf_FONT*	vf_addminifont();
/////////////////////////////control
typedef struct{
	int x;int y;int width;int height;
}vf_DRAWBOX;
/*
typedef struct{
	int x;int y;int width;int height;
	void (*fresh)();
	void (*focus)();
	*/

int t();
