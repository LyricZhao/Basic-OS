# include "bootpack.h"

# include <string.h>

struct WINDOWCTL *wctl;
struct WINDOW *key_window;

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
  window -> layer -> task = task;
  return;
}

void win_del(struct WINDOW *window) {
  struct LAYER *layer = window -> layer;
  memory_free_4k(memc, window -> img, layer -> xsize * layer -> ysize);
  window -> layer = 0;
  return;
}

struct WINDOW *window_alloc(void) {
  struct WINDOW *window = &wctl -> windows[wctl -> tot ++];
  return window;
}

void win_key_on(struct WINDOW *window) {
  key_window = window;
  struct LAYER *layer = window -> layer;
  make_wtitle(window -> img, layer -> xsize, window -> title, 1);
  display_refresh_layer_sub(dctl, layer, 3, 3, layer -> xsize - 3, 21);
  return;
}

void win_key_off(struct WINDOW *window) {
  struct LAYER *layer = window -> layer;
  make_wtitle(window -> img, layer -> xsize, window -> title, 0);
  display_refresh_layer_sub(dctl, layer, 3, 3, layer -> xsize - 3, 21);
  return;
}
