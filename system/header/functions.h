/* utility.c */
int imax(int a, int b);
int imin(int a, int b);
void iswap(int *a, int *b);
void cUpper(char *str);

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void load_tr(int tr);
int load_cr0(void);
void store_cr0(int cr0);
void farjump(int eip, int cs);
void farcall(int eip, int cs);
char asm_check_mem(unsigned int p);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void asm_inthandler0d(void);
void asm_inthandler0c(void);
void asm_con_putchar(char c);
void asm_hrb_api(void);
void asm_end_app(void);
void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);

/* graphic.c */
void init_palette(void);
void init_screen(char *vram, int xsize, int ysize);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(char *vram, int width, unsigned char color, int x0, int y0, int x1, int y1);
void putfont8(char *vram, int xsize, int x, int y, unsigned char color, char *font);
void putfont_ascii(char *vram, int xsize, int x, int y, unsigned char color, unsigned char *str);
void draw_block(char *vram, int width, int x, int y, int xsize, int ysize, char *arr);
void draw_center_window(unsigned char *vram, int xsize, int ysize, char *title, char status);
void make_textbox8(struct LAYER *layer, int x0, int y0, int xsize, int ysize, int col);
void boxfill8_in_layer(struct LAYER *layer, unsigned char col, int x0, int y0, int x1, int y1);
void make_wtitle(unsigned char *img, int xsize, char *title, char status);

/* dstcbl.c */
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

/* int.c */
void init_pic(void);
void inthandler27(int *esp);

/* fifo.c */
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_push(struct FIFO8 *fifo, unsigned char data);
unsigned char fifo8_pop(struct FIFO8 *fifo);
int fifo8_size(struct FIFO8 *fifo);

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_push(struct FIFO32 *fifo, int data);
int fifo32_pop(struct FIFO32 *fifo);
int fifo32_size(struct FIFO32 *fifo);

/* debuger.c */
void dprint_int(int dx);
void dprint_str(char *dc);

/* keyboard.c */
void wait_KBC_sendready(void);
void enable_keyboard(void);
void inthandler21(int *esp);

/* mouse.c */
void enable_mouse(void);
void init_mouse_cursor(struct MOUSE_STATUS *mouse, char background_color);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void inthandler2c(int *esp);

/* memory.c */
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void memory_init(void);
void memory_dprint(void);
void memory_management_init(struct MEM_MANAGER *memc);
unsigned int memory_alloc(struct MEM_MANAGER *memc, unsigned int size);
char memory_free(struct MEM_MANAGER *memc, unsigned int addr, unsigned int size);
unsigned int memory_total_free(struct MEM_MANAGER *memc);
unsigned int memory_alloc_4k(struct MEM_MANAGER *memc, unsigned int size);
char memory_free_4k(struct MEM_MANAGER *memc, unsigned int addr, unsigned int size);
void command_mem(struct CONSOLE *con);

/* lyrctl.c */
void lyrctl_init(void);
struct LAYER *layer_alloc(struct LYRCTL *lyrctl);
void layer_bset(struct LAYER *layer, unsigned char *img, int xsize, int ysize, int icol);
void layer_ud(struct LYRCTL *lyrctl, struct LAYER *layer, int height);
void layer_move(struct LYRCTL *lyrctl, struct LAYER *layer, int nx0, int ny0);
void layer_del(struct LYRCTL *lyrctl, struct LAYER *layer);
void display_refresh_all(struct LYRCTL *lyrctl);
void display_refresh_layer_sub(struct LYRCTL *lyrctl, struct LAYER *layer, int x0, int y0, int x1, int y1);
void display_refresh_sub(struct LYRCTL *lyrctl, int x0, int y0, int x1, int y1, int h0, int h1);
void map_refresh_sub(struct LYRCTL *lyrctl, int x0, int y0, int x1,int y1, int h0);
void putfont_ascii_in_layer(struct LAYER *layer, int x, int y, int col, int back, char *str);

/* timer.c */
void init_PIT(void);
void inthandler20(int *esp);
struct TIMER *timer_alloc(void);
void timer_init(struct TIMER *timer, struct FIFO32 *tq, unsigned char data);
void timer_free(struct TIMER *timer);
void timer_countdown(struct TIMER *timer, unsigned int timeout);

/* multitask.c */
struct TASK *multitask_init(void);
void task_switch(void);
void task_init(struct TASK *task);
void task_run(struct TASK *task, int level, int priority);
void task_sleep(struct TASK *task);
void task_add(struct TASK *stask);
void task_del(struct TASK *task);
void task_switchsub(void);
struct TASK *task_alloc(void);
struct TASK *task_now(void);

/* window.c */
void window_init(void);
struct WINDOW *window_alloc(void);
void window_set(struct WINDOW *window, char *title, int xsize, int ysize, int icol, int mx, int my, int height, int ws, struct TASK *task);

/* console.c */
void console_main(void);
void console_window_init(void);
void console_task_init(void);
void move_oneline(unsigned char *img, int xsize);
void command_handler(struct CONSOLE *con, char *command);
void print_screen(struct CONSOLE *con, char *str, int len);
void pf_nline(struct CONSOLE *con);
void command_clear(struct CONSOLE *con);
void command_run(struct CONSOLE *con, char *para);

/* filesystem.c */
void FAT_init(void);
void command_ls(struct CONSOLE *con);
void command_cat(struct CONSOLE *con, char *para);
char check_fname(struct FILEINFO *file, char *name);
void load_file(struct FILEINFO *file, char *targ);
struct FILEINFO *find_file(char *para);
void cat_output(struct CONSOLE *con, struct FILEINFO *file);

/* api.c */
void api_win_dw_line(struct WINDOW *window, int x0, int y0, int x1, int y1, int col);
void con_putchar(struct CONSOLE *con, char c);
void con_print(struct CONSOLE *con, char *str);
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
int *inthandler0d(int *esp);
int *inthandler0c(int *esp);
