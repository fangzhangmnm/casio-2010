/* created 20261031 by Claude Opus 5.5
 * Revolution-FX（当年 9860 圈子里常用的第三方库）的替身：只声明 Doodle Jump 和 3D 引擎用到的几个。
 * 参数类型是从调用处推的（没有原库的头文件）：PlotPixel 当年传的是 &buffer（整个数组的地址），所以收 void *。 */
#ifndef FXSDK_REVOLUTION_H
#define FXSDK_REVOLUTION_H
#include "fxlib.h"
#define BLACK 1
#define WHITE 0
void Change_Contrast(int contrast);
void Reset_Calc(void);
void PlotPixel(int x, int y, void *buffer, int color);
void DrawAll(void *buffer);
void GrayInit(int a, int b);
void GrayEnd(void);
void GrayLinkBuffers(void *a, void *b);
void CPUSpeedNormal(void);
void CPUSpeedDouble(void);
void CPUSpeedTriple(void);
void CPUSpeedQuadruple(void);
#endif
