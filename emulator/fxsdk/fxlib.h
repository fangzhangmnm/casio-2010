/* created 20261031 by Claude Opus 5.5
 * 卡西欧 fx-9860G SDK 的 fxlib.h 的替身：当年的 G1A 源码一个字不改，编成 WebAssembly 时 include 的是这个。
 * 函数名和参数照 SDK 手册《fx-9860G Libraries》（2007）；只声明这几个游戏用到的。实现在 fxsdk.c。
 * 键码数值是凭记忆写的 keybios.h 的值——程序只拿它们和同名常量比较（魔塔的菜单用 key-KEY_CHAR_1，字符键照 ASCII 连号），
 * 所以数值对不对不影响运行。 */
#ifndef FXSDK_FXLIB_H
#define FXSDK_FXLIB_H

#include <stdio.h>   /* vsprintf：当年的代码没 include 就直接用了，SDK 的头文件链里带着 */
#include <string.h>  /* strlen / memset：同上 */

typedef unsigned short FONTCHARACTER;
typedef struct { int left, top, right, bottom; } DISPBOX;

/* ---- 键码 ---- */
#define KEY_CHAR_0      0x30
#define KEY_CHAR_1      0x31
#define KEY_CHAR_2      0x32
#define KEY_CHAR_3      0x33
#define KEY_CHAR_4      0x34
#define KEY_CHAR_5      0x35
#define KEY_CHAR_6      0x36
#define KEY_CHAR_7      0x37
#define KEY_CHAR_8      0x38
#define KEY_CHAR_9      0x39
#define KEY_CHAR_DP     0x2e
#define KEY_CHAR_EXP    0x0f
#define KEY_CHAR_PMINUS 0x87
#define KEY_CHAR_PLUS   0x89
#define KEY_CHAR_MINUS  0x99
#define KEY_CHAR_MULT   0xa9
#define KEY_CHAR_DIV    0xb9
#define KEY_CHAR_LPAR   0x28
#define KEY_CHAR_RPAR   0x29
#define KEY_CHAR_COMMA  0x2c
#define KEY_CHAR_STORE  0x0e
#define KEY_CTRL_NOP    0
#define KEY_CTRL_EXE    30004
#define KEY_CTRL_DEL    30025
#define KEY_CTRL_AC     30015
#define KEY_CTRL_EXIT   30002
#define KEY_CTRL_MENU   30003
#define KEY_CTRL_SHIFT  30006
#define KEY_CTRL_ALPHA  30007
#define KEY_CTRL_OPTN   30008
#define KEY_CTRL_VARS   30016
#define KEY_CTRL_UP     30018
#define KEY_CTRL_DOWN   30023
#define KEY_CTRL_LEFT   30020
#define KEY_CTRL_RIGHT  30021
#define KEY_CTRL_F1     30009
#define KEY_CTRL_F2     30010
#define KEY_CTRL_F3     30011
#define KEY_CTRL_F4     30012
#define KEY_CTRL_F5     30013
#define KEY_CTRL_F6     30014

/* ---- 显示 ---- */
#define MINI_OVER  0x10
#define MINI_OR    0x11
#define MINI_REV   0x12
#define MINI_REVOR 0x13
void Bdisp_AllClr_DD(void);
void Bdisp_AllClr_VRAM(void);
void Bdisp_AllClr_DDVRAM(void);
void Bdisp_AreaClr_VRAM(const DISPBOX *box);
void Bdisp_AreaReverseVRAM(int x1, int y1, int x2, int y2);
void Bdisp_PutDisp_DD(void);
void Bdisp_SetPoint_VRAM(int x, int y, unsigned char point);
void Bdisp_SetPoint_DD(int x, int y, unsigned char point);
void Bdisp_SetPoint_DDVRAM(int x, int y, unsigned char point);
int Bdisp_GetPoint_VRAM(int x, int y);
void Bdisp_DrawLineVRAM(int x1, int y1, int x2, int y2);
void Bdisp_ClearLineVRAM(int x1, int y1, int x2, int y2);
void PrintXY(int x, int y, const unsigned char *str, int type);
void PrintMini(int x, int y, const unsigned char *str, int type);
void locate(int x, int y);
void Print(const unsigned char *str);

/* ---- 按键 / 时间 ---- */
int GetKey(unsigned int *keycode);
int IsKeyDown(int keycode);
int IsKeyUp(int keycode);
void Sleep(int millisecond);

/* 定时器和退出回调：当年的代码传进来的函数有的声明成 int f()、有的是 void f()；
 * WebAssembly 按函数指针调用时签名必须对得上，所以在这里记下它返回 void 还是 int，调用时按原样调。 */
#define ID_USER_TIMER1 1
#define ID_USER_TIMER2 2
#define ID_USER_TIMER3 3
#define ID_USER_TIMER4 4
#define ID_USER_TIMER5 5
#define FX_CALLBACK(fn) ((void *)(fn)), __builtin_types_compatible_p(__typeof__(*(fn)), void (void))
int fx_SetTimer(int ID, int elapse, void *handler, int returns_void);
void fx_SetQuitHandler(void *callback, int returns_void);
#define SetTimer(ID, elapse, handler) fx_SetTimer((ID), (elapse), FX_CALLBACK(handler))
#define SetQuitHandler(callback) fx_SetQuitHandler(FX_CALLBACK(callback))
int KillTimer(int ID);

/* ---- 文件（存储内存 \\fls0\） ---- */
#define _OPENMODE_READ             0x01
#define _OPENMODE_READ_SHARE       0x80
#define _OPENMODE_WRITE            0x02
#define _OPENMODE_READWRITE        0x03
#define _OPENMODE_READWRITE_SHARE  0x83
#define IML_FILEERR_NOERROR          0
#define IML_FILEERR_ENTRYNOTFOUND   -1
#define IML_FILEERR_ILLEGALPARAM    -2
#define IML_FILEERR_DEVICEFULL      -4
#define IML_FILEERR_ACCESSDENYED    -8
#define IML_FILEERR_ALREADYEXISTENTRY -13
int Bfile_OpenFile(const FONTCHARACTER *filename, int mode);
int Bfile_ReadFile(int HANDLE, void *buf, int size, int readpos);
int Bfile_WriteFile(int HANDLE, const void *buf, int size);
int Bfile_SeekFile(int HANDLE, int pos);
int Bfile_CloseFile(int HANDLE);
int Bfile_GetFileSize(int HANDLE);
int Bfile_CreateFile(const FONTCHARACTER *filename, int size);
int Bfile_DeleteFile(const FONTCHARACTER *filename);

/* SDK 模板里 InitializeSystem 用的；浏览器里不走这条路，直接调 AddIn_main */
#define INIT_ADDIN_APPLICATION(isAppli, OptionNum) 1

#endif
