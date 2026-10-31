/* created 20261031 by Claude Opus 5.5
 * 卡西欧 fx-9860G SDK（fxlib）+ Revolution-FX 的一小块，用来把当年的 G1A 源码原样编成 WebAssembly 在网页里跑。
 * 只实现这几个游戏用到的函数；行为照 SDK 手册《fx-9860G Libraries》（2007），手册没写清、按当年代码的用法推的地方在旁边注了。
 *
 * 和真机最大的不同：真机的定时器是中断，随时会插进来；这里没有线程，定时器只在程序「等」的地方触发
 * （Sleep、GetKey、IsKeyDown 的忙等循环）。这几个游戏都是在这些地方等的，所以看起来一样。
 * 浏览器要在这些地方拿回控制权去画屏、收按键：用 Emscripten 的 Asyncify 把 C 的调用栈挂起来，过一会儿再接着跑。
 *
 * 和页面的约定：页面创建模块时传进 Module.fx* 这些函数（shell.html「Add-in」一节）。 */
#include <emscripten.h>
#include <string.h>
#include "fxlib.h"
#include "revolution.h"
#include "fx2026.h"
#include "fxfont.h"   /* build_g1a.py 从 casio-font.json 生成：FX_FONT[256][7]，每行 5 位、高位在左 */
#include "minifont.h" /* build_g1a.py 从 casio-font-small.json 生成：PrintMini 的小字 MINI_W[256]、MINI_ROWS[256][5] */

static unsigned char VRAM[1024], DD[1024]; /* 128×64，一行 16 字节，高位在左（和真机显存同一个排法） */

/* ---------- 页面提供的函数 ---------- */
EM_JS(double, js_now, (void), { return Module.fxNow(); });
EM_ASYNC_JS(void, js_sleep, (int ms), { await new Promise(function (r) { Module.fxSleep(ms, r); }); });
EM_ASYNC_JS(void, js_halt, (void), { await new Promise(function () {}); }); /* 永远不醒：程序停在这里，页面丢掉这个实例 */
EM_JS(void, js_show, (const unsigned char *p), { Module.fxShow(HEAPU8.slice(p, p + 1024)); });
EM_JS(int, js_key_down, (int m), { return Module.fxKeyDown(m) ? 1 : 0; });
EM_JS(int, js_key_event, (void), { return Module.fxKeyEvent() | 0; });
EM_JS(void, js_key_flush, (void), { Module.fxKeyFlush(); });
EM_JS(int, js_quit_requested, (void), { return Module.fxQuitRequested() ? 1 : 0; });
EM_JS(void, js_ended, (int how), { Module.fxEnded(how); });
EM_JS(void, js_contrast, (int c), { Module.fxContrast(c); });
EM_JS(double, js_frame_ms, (void), { return Module.fxFrameMs || 0; });
EM_ASYNC_JS(void, js_next_frame, (void), { await new Promise(function (r) { Module.fxNextFrame(r); }); });
EM_JS(int, js_sensor, (int *out), { return Module.fxSensor(HEAP32.subarray(out >> 2, (out >> 2) + 11)); });
EM_JS(int, js_param, (int id, int def), { return Module.fxParam(id, def); });
EM_JS(int, js_touch, (int *out, int max), { return Module.fxTouch(HEAP32.subarray(out >> 2, (out >> 2) + max * 4), max); });
EM_JS(int, js_rtc, (void), { var d = new Date(); return Math.floor((d - new Date(d.getFullYear(), d.getMonth(), d.getDate())) * 128 / 1000); });
EM_JS(int, js_file_size, (const char *n), { return Module.fxFileSize(UTF8ToString(n)); });
EM_JS(int, js_file_read, (const char *n, int pos, unsigned char *buf, int len), { return Module.fxFileRead(UTF8ToString(n), pos, HEAPU8.subarray(buf, buf + len)); });
EM_JS(int, js_file_write, (const char *n, int pos, const unsigned char *buf, int len), { return Module.fxFileWrite(UTF8ToString(n), pos, HEAPU8.slice(buf, buf + len)); });
EM_JS(int, js_file_create, (const char *n, int size), { return Module.fxFileCreate(UTF8ToString(n), size); });
EM_JS(int, js_file_delete, (const char *n), { return Module.fxFileDelete(UTF8ToString(n)); });
EM_JS(int, js_file_close, (const char *n, int wrote), { return Module.fxFileClose(UTF8ToString(n), wrote); });
EM_JS(int, js_file_can_write, (void), { return Module.fxFileCanWrite() ? 1 : 0; });

/* ---------- 定时器、交还控制权、退出 ---------- */
typedef struct { int on, ms, returns_void; double due; void *fn; } fxtimer;
static fxtimer timers[6];
static void *quit_fn; static int quit_returns_void;
static int in_timer, in_quit, dd_dirty;
static double last_yield;

static void call_back(void *fn, int returns_void) {
  if (returns_void) ((void (*)(void))fn)(); else ((int (*)(void))fn)();
}
static void fire_timers(void) {
  int i;
  if (in_timer) return; /* 定时器里面不再触发定时器（真机上同一个定时器也不会重入） */
  for (i = 1; i <= 5; i++) {
    fxtimer *t = &timers[i]; double now;
    if (!t->on) continue;
    now = js_now();
    if (now < t->due) continue;
    t->due += t->ms; if (t->due < now) t->due = now + t->ms; /* 卡住很久之后不补发一串 */
    in_timer = 1; call_back(t->fn, t->returns_void); in_timer = 0;
  }
}
static double ms_to_next_timer(double now) {
  int i; double d = 1e9;
  if (in_timer) return d;
  for (i = 1; i <= 5; i++) if (timers[i].on && timers[i].due - now < d) d = timers[i].due - now;
  return d < 0 ? 0 : d;
}
static void yield_ms(double ms) { /* 交还给浏览器：画屏、收按键；ms 是计算器上的时间（页面按速度倍率换成真实时间） */
  js_sleep(ms < 0 ? 0 : (int)(ms + 0.5));
  last_yield = js_now(); dd_dirty = 0;
}
static void halt(int how) { /* how：0 = AddIn_main 返回；1 = 按 MENU 退出；2 = Reset_Calc */
  int i;
  for (i = 1; i <= 5; i++) timers[i].on = 0;
  js_ended(how); js_halt();
}
static void quit_if_requested(void) { /* 页面上按了 MENU：像真机切到别的应用一样，先调程序登记的退出回调（魔塔会问存不存档），再停 */
  int i;
  if (in_quit || !js_quit_requested()) return;
  in_quit = 1;
  for (i = 1; i <= 5; i++) timers[i].on = 0;
  if (quit_fn) call_back(quit_fn, quit_returns_void);
  halt(1);
}
/* 忙等循环（IsKeyDown）里每次都会调到：触发到点的定时器；隔一会儿交还一次浏览器。
 * 刚画过屏就 16 毫秒交还一次（一帧）；没在画、只是在空转等按键，就跑 2 毫秒歇 15 毫秒，省电。定时器里面不交还，免得拖慢它的节拍。 */
static void poll(void) {
  double now, rest;
  fire_timers();
  quit_if_requested();
  if (in_timer) return;
  now = js_now();
  if (dd_dirty) { if (now - last_yield < 16) return; rest = 0; }
  else { if (now - last_yield < 2) return; rest = 15; }
  if (ms_to_next_timer(now) < rest) rest = ms_to_next_timer(now);
  yield_ms(rest);
}

int fx_SetTimer(int ID, int elapse, void *handler, int returns_void) {
  if (ID < 1 || ID > 5 || timers[ID].on || !handler) return -1;
  timers[ID].on = 1; timers[ID].ms = elapse < 25 ? 25 : elapse; /* 手册：以 25 毫秒为单位 */
  timers[ID].fn = handler; timers[ID].returns_void = returns_void; timers[ID].due = js_now() + timers[ID].ms;
  return ID;
}
int KillTimer(int ID) { if (ID < 1 || ID > 5) return -1; timers[ID].on = 0; return ID; }
void fx_SetQuitHandler(void *callback, int returns_void) { quit_fn = callback; quit_returns_void = returns_void; }

void Sleep(int millisecond) {
  double until = js_now() + millisecond;
  for (;;) {
    double now, w;
    fire_timers(); quit_if_requested();
    now = js_now(); if (now >= until) break;
    w = until - now; if (ms_to_next_timer(now) < w) w = ms_to_next_timer(now);
    yield_ms(w);
  }
}

/* ---------- 按键 ---------- */
/* 页面那边按键按「矩阵码」记（就是卡西欧 BASIC 里 Getkey 给的那个数：十位 = 列，个位 = 行） */
static const int KEYS[][2] = {
  {KEY_CTRL_UP, 28}, {KEY_CTRL_DOWN, 37}, {KEY_CTRL_LEFT, 38}, {KEY_CTRL_RIGHT, 27},
  {KEY_CTRL_EXE, 31}, {KEY_CTRL_EXIT, 47}, {KEY_CTRL_AC, 34}, {KEY_CTRL_DEL, 44},
  {KEY_CTRL_OPTN, 68}, {KEY_CTRL_MENU, 48}, {KEY_CTRL_SHIFT, 78}, {KEY_CTRL_ALPHA, 77}, {KEY_CTRL_VARS, 58},
  {KEY_CTRL_F1, 79}, {KEY_CTRL_F2, 69}, {KEY_CTRL_F3, 59}, {KEY_CTRL_F4, 49}, {KEY_CTRL_F5, 39}, {KEY_CTRL_F6, 29},
  {KEY_CHAR_0, 71}, {KEY_CHAR_1, 72}, {KEY_CHAR_2, 62}, {KEY_CHAR_3, 52}, {KEY_CHAR_4, 73},
  {KEY_CHAR_5, 63}, {KEY_CHAR_6, 53}, {KEY_CHAR_7, 74}, {KEY_CHAR_8, 64}, {KEY_CHAR_9, 54},
  {KEY_CHAR_DP, 61}, {KEY_CHAR_EXP, 51}, {KEY_CHAR_PMINUS, 41}, {KEY_CHAR_PLUS, 42}, {KEY_CHAR_MINUS, 32},
  {KEY_CHAR_MULT, 43}, {KEY_CHAR_DIV, 33}, {KEY_CHAR_LPAR, 55}, {KEY_CHAR_RPAR, 45}, {KEY_CHAR_COMMA, 35}, {KEY_CHAR_STORE, 25},
};
#define NKEYS ((int)(sizeof KEYS / sizeof KEYS[0]))
static int matrix_of(int code) { int i; for (i = 0; i < NKEYS; i++) if (KEYS[i][0] == code) return KEYS[i][1]; return 0; }
static int code_of(int m) { int i; for (i = 0; i < NKEYS; i++) if (KEYS[i][1] == m) return KEYS[i][0]; return 0; }

int IsKeyDown(int keycode) { int m = matrix_of(keycode); poll(); return m ? js_key_down(m) : 0; }
int IsKeyUp(int keycode) { return !IsKeyDown(keycode); }

/* 手册：GetKey 等一个键，按的是字符键返回 1、控制键返回 0；MENU 键不返回（系统处理）。
 * 真机的 GetKey 还会把显存送上屏（当年的代码画完对话框直接 GetKey，靠的就是这个）。
 * 进来之前按下的键不算：这几个游戏走路用 IsKeyDown，走着走着撞上对话框时手里那个方向键不该把对话框直接按掉（真机上怎样没核过）。
 * SHIFT / ALPHA 单独按不返回（真机上是切换模式，这几个游戏没用到组合键的 GetKey）。 */
int GetKey(unsigned int *keycode) {
  Bdisp_PutDisp_DD();
  js_key_flush();
  for (;;) {
    int m = js_key_event();
    if (m) {
      int c = code_of(m);
      if (!c || c == KEY_CTRL_SHIFT || c == KEY_CTRL_ALPHA || c == KEY_CTRL_MENU) continue;
      *keycode = (unsigned int)c;
      return c < 30000;
    }
    fire_timers(); quit_if_requested();
    { double now = js_now(), w = 15; if (ms_to_next_timer(now) < w) w = ms_to_next_timer(now); yield_ms(w); }
  }
}

/* ---------- 画屏 ---------- */
static void setp(unsigned char *b, int x, int y, int on) {
  if (x < 0 || x > 127 || y < 0 || y > 63) return;
  if (on) b[(y << 4) + (x >> 3)] |= (unsigned char)(128 >> (x & 7));
  else b[(y << 4) + (x >> 3)] &= (unsigned char)~(128 >> (x & 7));
}
static int getp(const unsigned char *b, int x, int y) {
  if (x < 0 || x > 127 || y < 0 || y > 63) return 0;
  return (b[(y << 4) + (x >> 3)] >> (7 - (x & 7))) & 1;
}
void Bdisp_PutDisp_DD(void) { memcpy(DD, VRAM, 1024); js_show(DD); dd_dirty = 1; }
void Bdisp_AllClr_VRAM(void) { memset(VRAM, 0, 1024); }
void Bdisp_AllClr_DD(void) { memset(DD, 0, 1024); js_show(DD); dd_dirty = 1; }
void Bdisp_AllClr_DDVRAM(void) { Bdisp_AllClr_VRAM(); Bdisp_AllClr_DD(); }
void Bdisp_SetPoint_VRAM(int x, int y, unsigned char point) { setp(VRAM, x, y, point); }
void Bdisp_SetPoint_DD(int x, int y, unsigned char point) { setp(DD, x, y, point); js_show(DD); dd_dirty = 1; }
void Bdisp_SetPoint_DDVRAM(int x, int y, unsigned char point) { Bdisp_SetPoint_VRAM(x, y, point); Bdisp_SetPoint_DD(x, y, point); }
int Bdisp_GetPoint_VRAM(int x, int y) { return getp(VRAM, x, y); }
static void order(int *a, int *b) { if (*a > *b) { int t = *a; *a = *b; *b = t; } }
void Bdisp_AreaClr_VRAM(const DISPBOX *box) {
  int x, y, l = box->left, r = box->right, t = box->top, b = box->bottom;
  order(&l, &r); order(&t, &b);
  for (y = t; y <= b; y++) for (x = l; x <= r; x++) setp(VRAM, x, y, 0);
}
void Bdisp_AreaReverseVRAM(int x1, int y1, int x2, int y2) {
  int x, y;
  order(&x1, &x2); order(&y1, &y2);
  for (y = y1; y <= y2; y++) for (x = x1; x <= x2; x++) setp(VRAM, x, y, !getp(VRAM, x, y));
}
static void line(int x1, int y1, int x2, int y2, int on) { /* Bresenham，两端都画 */
  int dx = x2 > x1 ? x2 - x1 : x1 - x2, dy = y2 > y1 ? y2 - y1 : y1 - y2;
  int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1, err = dx - dy;
  for (;;) {
    int e2;
    setp(VRAM, x1, y1, on);
    if (x1 == x2 && y1 == y2) break;
    e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x1 += sx; }
    if (e2 < dx) { err += dx; y1 += sy; }
  }
}
void Bdisp_DrawLineVRAM(int x1, int y1, int x2, int y2) { line(x1, y1, x2, y2, 1); }
void Bdisp_ClearLineVRAM(int x1, int y1, int x2, int y2) { line(x1, y1, x2, y2, 0); }

/* PrintXY：每个字占 6×8 格，字形 5×7 在格子里第 2–6 列、第 1–7 行（左边空一列、下面空一行）；
 * 普通模式连底色一起写（当年的代码用打空格来擦掉旧数字，靠的就是这个），反色模式整格反过来。
 * 位置对过 7735 帖 2012 年的截图 ss7.bmp（左上「21F」、右边血量 3 位数）。 */
static void glyph6x8(int x, int y, unsigned char ch, int rev) {
  int r, c;
  for (r = 0; r < 8; r++) for (c = 0; c < 6; c++) {
    int on = (r < 7 && c >= 1) ? (FX_FONT[ch][r] >> (5 - c)) & 1 : 0;
    setp(VRAM, x + c, y + r, rev ? !on : on);
  }
}
void PrintXY(int x, int y, const unsigned char *str, int type) {
  for (; *str; str++, x += 6) glyph6x8(x, y, *str, type & 1);
}
static int cur_x = 1, cur_y = 1;
void locate(int x, int y) { cur_x = x; cur_y = y; }
void Print(const unsigned char *str) { for (; *str && cur_x <= 21; str++, cur_x++) glyph6x8((cur_x - 1) * 6, (cur_y - 1) * 8, *str, 0); }

/* PrintMini：小字高 5 点、各字宽度不一（字形采自 SDK 码表第 7 页），字距 1 点、行高 6 点。模式：手册里的 MINI_OVER / OR / REV / REVOR；
 * 魔塔传的是 0（不是手册里的 0x10），按低两位当 OVER 处理（推的）。 */
void PrintMini(int x, int y, const unsigned char *str, int type) {
  int mode = type & 3;
  for (; *str; str++) {
    int ch = *str, w = MINI_W[ch], r, c;
    for (r = 0; r < 6; r++) for (c = 0; c <= w; c++) {
      int on = (r < 5 && c < w) ? (MINI_ROWS[ch][r] >> (w - 1 - c)) & 1 : 0;
      if (mode == 0) setp(VRAM, x + c, y + r, on);
      else if (mode == 1) { if (on) setp(VRAM, x + c, y + r, 1); }
      else if (mode == 2) setp(VRAM, x + c, y + r, !on);
      else if (on) setp(VRAM, x + c, y + r, 0);
    }
    x += w + 1;
  }
}

/* ---------- 文件：存储内存（\\fls0\）。字节在页面那边，写过的文件关闭时存进浏览器 ---------- */
typedef struct { int used, pos, wrote, mode; char path[64]; } fxfile;
static fxfile files[8];
static void path_of(const FONTCHARACTER *fn, char *out) { /* "\\fls0\MagTower.sav" → "fls0/MagTower.sav" */
  int i = 0, o = 0;
  while (fn[i] == '\\') i++;
  for (; fn[i] && o < 63; i++) out[o++] = fn[i] == '\\' ? '/' : (char)fn[i];
  out[o] = 0;
}
static fxfile *file_of(int h) { return (h >= 0 && h < 8 && files[h].used) ? &files[h] : 0; }
int Bfile_OpenFile(const FONTCHARACTER *filename, int mode) {
  char path[64]; int h;
  path_of(filename, path);
  if (js_file_size(path) < 0) return IML_FILEERR_ENTRYNOTFOUND;
  if ((mode & 2) && !js_file_can_write()) return IML_FILEERR_ACCESSDENYED; /* 浏览器存不了（如隐私模式）：让游戏照实说保存失败 */
  for (h = 0; h < 8; h++) if (!files[h].used) break;
  if (h == 8) return IML_FILEERR_ILLEGALPARAM;
  files[h].used = 1; files[h].pos = 0; files[h].wrote = 0; files[h].mode = mode;
  strcpy(files[h].path, path);
  return h;
}
int Bfile_ReadFile(int HANDLE, void *buf, int size, int readpos) {
  fxfile *f = file_of(HANDLE); int n;
  if (!f) return IML_FILEERR_ILLEGALPARAM;
  if (readpos >= 0) f->pos = readpos;
  n = js_file_read(f->path, f->pos, (unsigned char *)buf, size);
  if (n > 0) f->pos += n;
  return n;
}
int Bfile_WriteFile(int HANDLE, const void *buf, int size) {
  fxfile *f = file_of(HANDLE); int n;
  if (!f || !(f->mode & 2)) return IML_FILEERR_ILLEGALPARAM;
  n = js_file_write(f->path, f->pos, (const unsigned char *)buf, size);
  if (n < 0) return n;
  f->pos += size; f->wrote = 1;
  return f->pos;
}
int Bfile_SeekFile(int HANDLE, int pos) {
  fxfile *f = file_of(HANDLE);
  if (!f) return IML_FILEERR_ILLEGALPARAM;
  f->pos = pos;
  return js_file_size(f->path) - pos;
}
int Bfile_GetFileSize(int HANDLE) { fxfile *f = file_of(HANDLE); return f ? js_file_size(f->path) : IML_FILEERR_ILLEGALPARAM; }
int Bfile_CloseFile(int HANDLE) {
  fxfile *f = file_of(HANDLE); int r;
  if (!f) return IML_FILEERR_ILLEGALPARAM;
  r = js_file_close(f->path, f->wrote);
  f->used = 0;
  return r;
}
int Bfile_CreateFile(const FONTCHARACTER *filename, int size) {
  char path[64];
  path_of(filename, path);
  if (!js_file_can_write()) return IML_FILEERR_ACCESSDENYED;
  return js_file_create(path, size);
}
int Bfile_DeleteFile(const FONTCHARACTER *filename) { char path[64]; path_of(filename, path); return js_file_delete(path); }

/* ---------- Revolution-FX ---------- */
void Change_Contrast(int contrast) { js_contrast(contrast); }
void Reset_Calc(void) { halt(2); } /* 真机上是重启计算器；这里 = 退回列表 */
void PlotPixel(int x, int y, void *buffer, int color) { setp((unsigned char *)buffer, x, y, color == BLACK); }
/* DrawAll：3D 引擎每画完一帧调一次。它的主循环不睡觉、走路按「每帧走多少」算，所以网页里要把帧率卡在真机的速度：
 * 页面给 Module.fxFrameMs（计算器上的毫秒，速度倍率另乘），没给就不卡。 */
static double last_frame;
void DrawAll(void *buffer) {
  double fm = js_frame_ms();
  memcpy(DD, buffer, 1024); js_show(DD); dd_dirty = 1;
  if (fm > 0) { /* 按节拍对齐：下一帧 = 上一帧该到的时刻 + fm（不按醒来的时刻算，免得每帧的计算时间累加进去）；落后超过一帧就重新对齐 */
    double due = last_frame + fm, now = js_now();
    if (due > now) Sleep((int)(due - now + 0.5));
    last_frame = due < now - fm ? js_now() : due;
  }
  else poll();
}
void GrayInit(int a, int b) { (void)a; (void)b; }
void GrayEnd(void) {}
void GrayLinkBuffers(void *a, void *b) { (void)a; (void)b; }
void CPUSpeedNormal(void) {}
void CPUSpeedDouble(void) {}
void CPUSpeedTriple(void) {}
void CPUSpeedQuadruple(void) {}

/* ---------- 2026 扩展（fx2026.h）：真机上没有 ---------- */
static double last_frame_wait = -1;
static TOUCH_POINT touches[TOUCH_MAX]; static int ntouch;
static void touch_snapshot(void) { /* 网页按 [id, x, y, state] × n 填 */
  int raw[TOUCH_MAX * 4], i;
  ntouch = js_touch(raw, TOUCH_MAX);
  for (i = 0; i < ntouch; i++) { touches[i].id = raw[i*4]; touches[i].x = raw[i*4+1]; touches[i].y = raw[i*4+2]; touches[i].state = raw[i*4+3]; touches[i].used = 0; }
}
int Touch_Read(TOUCH_POINT *pts, int max) { int i, n = ntouch < max ? ntouch : max; for (i = 0; i < n; i++) pts[i] = touches[i]; return n; }
int Touch_Hit(int x1, int y1, int x2, int y2, int states) {
  int i;
  for (i = 0; i < ntouch; i++) {
    TOUCH_POINT *t = &touches[i];
    if (!t->used && (t->state & states) && t->x >= x1 && t->x <= x2 && t->y >= y1 && t->y <= y2) { t->used = 1; return 1; }
  }
  return 0;
}
int Frame_Wait(void) {
  double now, us;
  fire_timers(); quit_if_requested();
  js_next_frame(); last_yield = js_now(); dd_dirty = 0;
  quit_if_requested();
  touch_snapshot();
  now = js_now();
  us = last_frame_wait < 0 ? 0 : (now - last_frame_wait) * 1000;
  last_frame_wait = now;
  if (us > 100000) us = 100000; /* 切到别的标签页回来，别一步跳出去 */
  return (int)us;
}
int Sensor_Read(SENSOR_DATA *data) { return js_sensor((int *)data); }
int Param_Get(int id, int def) { return js_param(id, def); }
int RTC_GetTicks(void) { return js_rtc(); } /* 真机：当天零点起的 1/128 秒数 */

/* ---------- 入口 ---------- */
int AddIn_main(int isAppli, unsigned short OptionNum);
EMSCRIPTEN_KEEPALIVE void fx_run(void) {
  last_yield = js_now();
  AddIn_main(1, 0);
  halt(0);
}
