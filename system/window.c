# include "bootpack.h"

# include <string.h>

struct WINDOWCTL *wctl;

extern struct LYRCTL *dctl;
extern struct MEM_MANAGER *memc;

void window_init(void) {
  wctl = (struct WINDOWCTL *) memory_alloc_4k(memc, sizeof(struct WINDOWCTL));
  wctl -> tot = 0;
  return;
}

void window_set(struct WINDOW *window, char *title, int xsize, int ysize, int icol, int mx, int my, int height, int ws, struct TASK *task) {
  window -> img = (unsigned char *) memory_alloc_4k(memc, xsize * ysize);
  window -> layer = layer_alloc(dctl);
  memcpy(window -> title, title, sizeof(char) * (strlen(title) + 1));
  layer_bset(window -> layer, window -> img, xsize, ysize, icol);
  draw_center_window(window -> img, xsize, ysize, window -> title, ws);
  layer_move(dctl, window -> layer, mx, my);
  layer_ud(dctl, window -> layer, height);
  window -> task = task;
  return;
}

struct WINDOW *window_alloc(void) {
  struct WINDOW *window = &wctl -> windows[wctl -> tot ++];
  return window;
}
