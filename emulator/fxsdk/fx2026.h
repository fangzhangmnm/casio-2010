/* created 20261031 by Claude Opus 5.5
 * 2026 扩展：真机上没有，只给 2026 版用（考古家规（见 CLAUDE.md）：原样版是默认，2026 版另算）。照卡西欧库的风格：整数、出参、返回值 0 = 成功 / 负数 = 失败，
 * 模块前缀 + 下划线命名（像 Bdisp_ / Bfile_ / RTC_）。
 * user 10-31：「锁帧算dt」「传感器数据给全量的。但是int还是float你定，按照casio库的风格来」
 *            「casio不用懂，或者用casio自己的角度，假设casio有一个陀螺仪」 */
#ifndef FXSDK_FX2026_H
#define FXSDK_FX2026_H

/* 等到下一次屏幕刷新（浏览器的一帧），返回距上一次 Frame_Wait 过了多少微秒（计算器上的时间，速度倍率已算进去；最多 100000）。
 * 游戏一帧画一次、按这个时间算位移 = 锁帧算 dt。 */
int Frame_Wait(void);

/* 计算器自带的加速度计 + 陀螺仪（假想的）。坐标是计算器自己的：x = 液晶往右，y = 液晶往上，z = 屏幕朝外。
 * 计算器不知道自己被装在什么网页、什么设备里——那边怎么转、怎么摆，网页自己换算好再交过来。 */
typedef struct {
  int gx, gy, gz;   /* 加速度计读数（含重力），单位 0.001 m/s²：静止时指向「上」，平放屏幕朝上 ≈ (0, 0, +9807) */
  int ax, ay, az;   /* 不含重力的加速度，单位 0.001 m/s²（没有就是 0） */
  int wx, wy, wz;   /* 绕 x / y / z 轴的角速度，单位 0.01°/s（右手定则） */
  int heading;      /* 朝向（指南针那种，绕竖直方向），单位 0.01°；没有就是 −1 */
  int age;          /* 这份数据离现在多少毫秒 */
} SENSOR_DATA;
/* 返回 0 = 有数据；-1 = 没开 / 没给权限 / 这台设备没有。 */
int Sensor_Read(SENSOR_DATA *data);

/* 触屏（user 10-31「或者说其实就是画控件的时候consume一下event？」）：每次 Frame_Wait 时拍一张这一帧的触点快照。
 * 坐标是计算器液晶上的点（x 0–127 往右，y 0–63 往下，和 Bdisp_SetPoint_VRAM 同一套）——网页怎么摆计算器、怎么旋转，网页自己换算好。
 * 用法（立即模式）：画控件的时候问 Touch_Hit，有手指就当场「吃掉」它，后面的控件就拿不到这根手指，不会一下点两个东西。 */
#define TOUCH_DOWN 1   /* 这一帧刚按下 */
#define TOUCH_HELD 2   /* 按着 */
#define TOUCH_UP   4   /* 这一帧刚抬起 */
#define TOUCH_MAX  8
typedef struct {
  int id;      /* 同一根手指从按下到抬起不变 */
  int x, y;    /* 液晶坐标 */
  int state;   /* TOUCH_DOWN / TOUCH_HELD / TOUCH_UP 之一 */
  int used;    /* 这一帧已被 Touch_Hit 吃掉 = 1 */
} TOUCH_POINT;
/* 把这一帧的触点抄到 pts（最多 max 个），返回个数。 */
int Touch_Read(TOUCH_POINT *pts, int max);
/* 矩形 [x1, x2] × [y1, y2] 里有没有还没被吃掉、状态在 states（TOUCH_* 按位或）里的触点：有就吃掉它，返回 1；没有返回 0。 */
int Touch_Hit(int x1, int y1, int x2, int y2, int states);

/* 页面上「调手感」那几个参数的当前值（整数，单位见各游戏自己的说明）；没有这个参数就返回 def。 */
int Param_Get(int id, int def);

/* 随机数种子用：当天零点起的 1/128 秒数（真机上社区用的 RTC_GetTicks 系统调用就是这个单位）。 */
int RTC_GetTicks(void);

#endif
