# include "bootpack.h"

# include <string.h>

struct LYRCTL *dctl;
extern struct BOOTINFO *binfo;
extern struct MEM_MANAGER *memc;

void lyrctl_init(void) {
  dctl = (struct LYRCTL *) memory_alloc_4k(memc, sizeof(struct LYRCTL));
  dctl -> vram  = binfo -> vram;
  dctl -> map   = (unsigned char *)memory_alloc_4k(memc, binfo -> screen_x * binfo -> screen_y);
  dctl -> xsize = binfo -> screen_x;
  dctl -> ysize = binfo -> screen_y;
  dctl -> top = -1; // no layers
  for(int i = 0; i < MAX_LAYERS; ++ i)
    dctl -> layers0[i].flags = 0;
  return;
}

struct LAYER *layer_alloc(struct LYRCTL *lyrctl) {
  for(int i = 0; i < MAX_LAYERS; ++ i) {
    if(!lyrctl -> layers0[i].flags) {
      lyrctl -> layers0[i].flags = LAYER_INUSE;
      lyrctl -> layers0[i].height = -1; // hiden
      return &(lyrctl -> layers0[i]);
    }
  }
  return 0;
}

void layer_bset(struct LAYER *layer, unsigned char *img, int xsize, int ysize, int icol) {
  layer -> img   = img;
  layer -> xsize = xsize;
  layer -> ysize = ysize;
  layer -> icol  = icol;
  return;
}

void layer_ud(struct LYRCTL *lyrctl, struct LAYER* layer, int height) {

  int prh = layer -> height;

  if(height > lyrctl -> top + 1) height = lyrctl -> top + 1;
  if(height < -1) height = -1;
  layer -> height = height;

  if(prh > height) {
    if(height >= 0) {
      for(int i = prh; i > height; -- i) {
        lyrctl -> layers[i] = lyrctl -> layers[i - 1];
        lyrctl -> layers[i] -> height = i;
      }
      lyrctl -> layers[height] = layer;
      map_refresh_sub(lyrctl, layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height + 1);
      display_refresh_sub(lyrctl, layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height + 1, prh);
    } else {
      -- lyrctl -> top;
      if(lyrctl -> top > prh) {
        for(int i = prh; i <= lyrctl -> top; ++ i) {
          lyrctl -> layers[i] = lyrctl -> layers[i + 1];
          lyrctl -> layers[i] -> height = i;
        }
      }
      map_refresh_sub(lyrctl, layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, 0);
      display_refresh_sub(lyrctl, layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, 0, prh - 1);
    }
  } else if(prh < height) {
    if(prh >= 0) {
      for(int i = prh; i < height; ++ i) {
        lyrctl -> layers[i] = lyrctl -> layers[i + 1];
        lyrctl -> layers[i] -> height = i;
      }
      lyrctl -> layers[height] = layer;
    } else {
      ++ lyrctl -> top;
      for(int i = lyrctl -> top; i > height; -- i) {
        lyrctl -> layers[i] = lyrctl -> layers[i - 1];
        lyrctl -> layers[i] -> height = i;
      }
      lyrctl -> layers[height] = layer;
    }
    map_refresh_sub(lyrctl, layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height);
    display_refresh_sub(lyrctl, layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height, height);
  }
  return;
}

void layer_move(struct LYRCTL *lyrctl, struct LAYER *layer, int nx0, int ny0) {
  int px0 = layer -> x0, py0 = layer -> y0;
  layer -> x0 = nx0, layer -> y0 = ny0;
  if(~ layer -> height) {
    map_refresh_sub(lyrctl, px0, py0, px0 + layer -> xsize, py0 + layer -> ysize, 0);
    map_refresh_sub(lyrctl, nx0, ny0, nx0 + layer -> xsize, ny0 + layer -> ysize, layer -> height - 1);
    display_refresh_sub(lyrctl, px0, py0, px0 + layer -> xsize, py0 + layer -> ysize, 0, layer -> height);
    display_refresh_sub(lyrctl, nx0, ny0, nx0 + layer -> xsize, ny0 + layer -> ysize, layer -> height, layer -> height);
  }
  return;
}

void layer_del(struct LYRCTL *lyrctl, struct LAYER *layer) {
  if(~ layer -> height)
    layer_ud(lyrctl, layer, -1);
  layer -> flags = 0;
  return;
}

void display_refresh_all(struct LYRCTL *lyrctl) {
  int tc, pos, lid;
  unsigned char col, *buf;
  for(int h = 0; h <= lyrctl -> top; ++ h) {
    tc  = lyrctl -> layers[h] -> icol;
    buf = lyrctl -> layers[h] -> img;
    lid = lyrctl -> layers[h] - lyrctl -> layers0;
    for(int i = 0; i < lyrctl -> layers[h] -> ysize; ++ i) for(int j = 0;j < lyrctl -> layers[h]-> xsize; ++ j) {
      col = buf[i * lyrctl -> layers[h] -> xsize + j];
      pos = (lyrctl -> layers[h]-> y0 + i) * lyrctl -> xsize + lyrctl -> layers[h]-> x0 + j;
      if(lyrctl -> map[pos] == lid)
        lyrctl -> vram[pos] = col;
    }
  }
  return;
}

void display_refresh_layer_sub(struct LYRCTL *lyrctl, struct LAYER *layer, int x0, int y0, int x1, int y1) {
  if(~ layer -> height)
    display_refresh_sub(lyrctl, layer -> x0 + x0, layer -> y0 + y0, layer -> x0 + x1, layer -> y0 + y1, layer -> height, layer -> height);
  return;
}

void display_refresh_sub(struct LYRCTL *lyrctl, int x0, int y0, int x1, int y1, int h0, int h1) {
  unsigned char col, *buf;
  int tc, bx0, by0, bx1, by1, lid, pos;
  for(int h = h0; h <= h1; ++ h) {
    tc  = lyrctl -> layers[h] -> icol;
    bx0 = imax(x0, lyrctl -> layers[h] -> x0);
    by0 = imax(y0, lyrctl -> layers[h] -> y0);
    bx1 = imin(x1, lyrctl -> layers[h] -> x0 + lyrctl -> layers[h] -> xsize);
    by1 = imin(y1, lyrctl -> layers[h] -> y0 + lyrctl -> layers[h] -> ysize);
    bx0 = imin(bx0, lyrctl -> xsize), by0 = imin(by0, lyrctl -> ysize);
    bx1 = imin(bx1, lyrctl -> xsize), by1 = imin(by1, lyrctl -> ysize);
    buf = lyrctl -> layers[h] -> img;
    lid = lyrctl -> layers[h] - lyrctl -> layers0;
    for(int i = by0; i < by1; ++ i) for(int j = bx0; j < bx1; ++ j) {
      col = buf[(i - lyrctl -> layers[h] -> y0) * lyrctl -> layers[h] -> xsize + j - lyrctl -> layers[h] -> x0];
      pos = i * lyrctl -> xsize + j;
      if(lyrctl -> map[pos] == lid)
        lyrctl -> vram[pos] = col;
    }
  }
  return;
}

void map_refresh_sub(struct LYRCTL *lyrctl, int x0, int y0, int x1, int y1, int h0) {
  unsigned char col, *buf;
  int tc, lid, bx0, by0, bx1, by1;
  for(int h = h0; h <= lyrctl -> top; ++ h) {
    tc  = lyrctl -> layers[h] -> icol;
    lid = lyrctl -> layers[h] - lyrctl -> layers0;
    buf = lyrctl -> layers[h] -> img;
    bx0 = imax(x0, lyrctl -> layers[h] -> x0);
    by0 = imax(y0, lyrctl -> layers[h] -> y0);
    bx1 = imin(x1, lyrctl -> layers[h] -> x0 + lyrctl -> layers[h] -> xsize);
    by1 = imin(y1, lyrctl -> layers[h] -> y0 + lyrctl -> layers[h] -> ysize);
    bx0 = imin(bx0, lyrctl -> xsize), by0 = imin(by0, lyrctl -> ysize);
    bx1 = imin(bx1, lyrctl -> xsize), by1 = imin(by1, lyrctl -> ysize);
    for(int i = by0; i < by1; ++ i) for(int j = bx0; j < bx1; ++ j) {
      col = buf[(i - lyrctl -> layers[h] -> y0) * lyrctl -> layers[h] -> xsize + j - lyrctl -> layers[h] -> x0];
      if(tc == -1 || col != tc)
        lyrctl -> map[i * lyrctl -> xsize + j] = lid;
    }
  }
  return;
}

void putfont_ascii_in_layer(struct LAYER *layer, int x, int y, int col ,int back, char *str) {
	int len = strlen(str);
	boxfill8(layer -> img, layer -> xsize, back, x, y, x + (len << 3) - 1, y + 15);
	putfont_ascii(layer -> img, layer -> xsize, x, y, col, str);
	display_refresh_layer_sub(dctl, layer, x, y, x + (len << 3), y + 16);
	return;
}
