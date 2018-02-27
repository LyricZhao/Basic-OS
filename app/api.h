/* System API */
char* api_malloc(int size);
 void api_memory_init(void);
 void api_free(char *addr, int size);
 void api_end(void);
  int api_getkey(int mode);

/* GUI API */
 int api_win_open(int xsize, int ysize, int icol, char *title);
void api_win_close(int win);
void api_win_print(int win, int x, int y, int col, int len, char *str);
void api_win_boxfill(int win, int x0, int y0, int x1, int y1, int col);
void api_win_dw_point(int win, int x, int y, int col);
void api_win_dw_line(int win, int x0, int y0, int x1, int y1, int col);
void api_win_refresh_sub(int win, int x0, int y0, int x1, int y1);

/* Console API */
void api_putchar(char c);
void api_print(char *str);

/* Timer API */
int api_timer_alloc(void);
int api_timer_init(int timer, int data);
int api_timer_countdown(int timer, int time);
void api_timer_free(int timer);
