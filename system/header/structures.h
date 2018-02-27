/* asmhead.nas */
struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short screen_x, screen_y;
	char *vram;
};

/* dstcbl.c */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

/* fifo.c */
struct FIFO8 {
	unsigned char *buf;
	int size, free;
	int p, q, flags;
};

struct FIFO32 {
	int *buf;
	int size, free;
	int p, q, flags;
	struct TASK *task;
};

/* mouse.c */
struct MOUSE_DEC {
	unsigned char buf[3]; int phase;
	int x, y, btn, mmx, mmy;
};

struct MOUSE_STATUS {
	unsigned char mcur[256];
	int mx, my;
};

/* memory.c */
struct FREE_BLOCK {
  unsigned int addr, size;
};

struct MEM_MANAGER {
  int free_count, lost_size, lost_time;
  struct FREE_BLOCK fb[MAX_FREE_BLOCK_CNT];
};

/* multitask.c */
struct TSS32 {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

struct TASK {
	int gdt_sel, flags;
	int priority, level;
	struct TSS32 tss;
	struct FIFO32 fifo;
	struct CONSOLE *console;
	int cs_base;
};

struct TASKLV {
	int tot, current;
	struct TASK *tasks[MAX_TASKS_LV];
};

struct TASKCTL {
	int clv; char lv_change;
	struct TASKLV levels[MAX_LEVELS];
	struct TASK tasks0[MAX_TASKS];
};

/* lyrctl.c */
struct LAYER {
	unsigned char *img;
	int xsize, ysize, x0, y0;
	int icol, height, flags;
	struct TASK *task;
	struct WINDOW *window;
};

struct LYRCTL {
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct LAYER layers0[MAX_LAYERS];
	struct LAYER *layers[MAX_LAYERS];
};

/* timer.c */
struct TIMER {
	unsigned int count, timeout, pos;
	struct FIFO32 *tq;
	unsigned char data, flags;
};

struct TIMERCTL {
	unsigned int count, next, tot;
	struct TIMER *timers[MAX_TIMERS];
	struct TIMER timers0[MAX_TIMERS];
};

/* window.c */
struct WINDOW {
	struct LAYER *layer;
	unsigned char *img;
	char title[32];
	struct TASK *task;
	struct CONSOLE *console;
};

struct WINDOWCTL{
	struct WINDOW windows[MAX_WINDOWS];
	int tot;
};

/* console.c */
struct CONSOLE {
	struct WINDOW *window;
	struct TASK *task;
	struct TIMER *timer;
	int cursor_x, cursor_y;
};

/* filesystem.c */

struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};
