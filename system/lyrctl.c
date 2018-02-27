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

struct LAYER *layer_alloc(void) {
  for(int i = 0; i < MAX_LAYERS; ++ i) {
    if(!dctl -> layers0[i].flags) {
      dctl -> layers0[i].flags = LAYER_INUSE;
      dctl -> layers0[i].height = -1; // hiden
      return &(dctl -> layers0[i]);
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

void layer_ud(struct LAYER* layer, int height) {

  int prh = layer -> height;

  if(height > dctl -> top + 1) height = dctl -> top + 1;
  if(height < -1) height = -1;
  layer -> height = height;

  if(prh > height) {
    if(height >= 0) {
      for(int i = prh; i > height; -- i) {
        dctl -> layers[i] = dctl -> layers[i - 1];
        dctl -> layers[i] -> height = i;
      }
      dctl -> layers[height] = layer;
      map_refresh_sub(layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height + 1);
      display_refresh_sub(layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height + 1, prh);
    } else {
      -- dctl -> top;
      if(dctl -> top >= prh) {
        for(int i = prh; i <= dctl -> top; ++ i) {
          dctl -> layers[i] = dctl -> layers[i + 1];
          dctl -> layers[i] -> height = i;
        }
      }
      map_refresh_sub(layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, 0);
      display_refresh_sub(layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, 0, prh - 1);
    }
  } else if(prh < height) {
    if(prh >= 0) {
      for(int i = prh; i < height; ++ i) {
        dctl -> layers[i] = dctl -> layers[i + 1];
        dctl -> layers[i] -> height = i;
      }
      dctl -> layers[height] = layer;
    } else {
      ++ dctl -> top;
      for(int i = dctl -> top; i > height; -- i) {
        dctl -> layers[i] = dctl -> layers[i - 1];
        dctl -> layers[i] -> height = i;
      }
      dctl -> layers[height] = layer;
    }
    map_refresh_sub(layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height);
    display_refresh_sub(layer -> x0, layer -> y0, layer -> x0 + layer -> xsize, layer -> y0 + layer -> ysize, height, height);
  }
  return;
}

void layer_move(struct LAYER *layer, int nx0, int ny0) {
  int px0 = layer -> x0, py0 = layer -> y0;
  layer -> x0 = nx0, layer -> y0 = ny0;
  if(~ layer -> height) {
    map_refresh_sub(px0, py0, px0 + layer -> xsize, py0 + layer -> ysize, 0);
    map_refresh_sub(nx0, ny0, nx0 + layer -> xsize, ny0 + layer -> ysize, layer -> height - 1);
    display_refresh_sub(px0, py0, px0 + layer -> xsize, py0 + layer -> ysize, 0, layer -> height);
    display_refresh_sub(nx0, ny0, nx0 + layer -> xsize, ny0 + layer -> ysize, layer -> height, layer -> height);
  }
  return;
}

void layer_del(struct LAYER *layer) {
  if(~ layer -> height) {
    layer_ud(layer, -1);
  }
  layer -> flags = 0;
  return;
}

void display_refresh_all(void) {
  int tc, pos, lid;
  unsigned char col, *buf;
  for(int h = 0; h <= dctl -> top; ++ h) {
    struct LAYER *layer = dctl -> layers[h];
    tc  = layer -> icol;
    buf = layer -> img;
    lid = layer - dctl -> layers0;
    for(int i = 0; i < layer -> ysize; ++ i) for(int j = 0;j < layer -> xsize; ++ j) {
      col = buf[i * layer -> xsize + j];
      pos = (layer -> y0 + i) * dctl -> xsize + layer -> x0 + j;
      if(dctl -> map[pos] == lid)
        dctl -> vram[pos] = col;
    }
  }
  return;
}

void display_refresh_layer_sub(struct LAYER *layer, int x0, int y0, int x1, int y1) {
  if(~ layer -> height)
    display_refresh_sub(layer -> x0 + x0, layer -> y0 + y0, layer -> x0 + x1, layer -> y0 + y1, layer -> height, layer -> height);
  return;
}

/*
void display_refresh_sub(int x0, int y0, int x1, int y1, int h0, int h1) {
  unsigned char col, *buf;
  int tc, bx0, by0, bx1, by1, lid, pos;
  for(int h = h0; h <= h1; ++ h) {
    tc  = dctl -> layers[h] -> icol;
    bx0 = imax(x0, dctl -> layers[h] -> x0);
    by0 = imax(y0, dctl -> layers[h] -> y0);
    bx1 = imin(x1, dctl -> layers[h] -> x0 + dctl -> layers[h] -> xsize);
    by1 = imin(y1, dctl -> layers[h] -> y0 + dctl -> layers[h] -> ysize);
    bx0 = imin(bx0, dctl -> xsize), by0 = imin(by0, dctl -> ysize);
    bx1 = imin(bx1, dctl -> xsize), by1 = imin(by1, dctl -> ysize);
    buf = dctl -> layers[h] -> img;
    lid = dctl -> layers[h] - dctl -> layers0;
    for(int i = by0; i < by1; ++ i) for(int j = bx0; j < bx1; ++ j) {
      col = buf[(i - dctl -> layers[h] -> y0) * dctl -> layers[h] -> xsize + j - dctl -> layers[h] -> x0];
      pos = i * dctl -> xsize + j;
      if(dctl -> map[pos] == lid)
        dctl -> vram[pos] = col;
    }
  }
  return;
}*/

void display_refresh_sub(int x0, int y0, int x1, int y1, int h0, int h1) {
  unsigned char col, *buf, *mmp, *mmv, *mmb;
  int *iip, *iiv, *iib;
  int tc, bx0, by0, bx1, by1, lid, pos, sid, len, len2;
  for(int h = h0; h <= h1; ++ h) {
    struct LAYER *layer = dctl -> layers[h];
    tc  = layer -> icol;
    bx0 = imax(x0, layer -> x0);
    by0 = imax(y0, layer -> y0);
    bx1 = imin(x1, layer -> x0 + layer -> xsize);
    by1 = imin(y1, layer -> y0 + layer -> ysize);
    bx0 = imin(bx0, dctl -> xsize), by0 = imin(by0, dctl -> ysize);
    bx1 = imin(bx1, dctl -> xsize), by1 = imin(by1, dctl -> ysize);
    buf = layer -> img;
    lid = layer - dctl -> layers0;
    sid = lid | lid << 8 | lid << 16 | lid << 24;
    /*
    for(int i = by0; i < by1; ++ i) for(int j = bx0; j < bx1; ++ j) {
      col = buf[(i - layer -> y0) * layer -> xsize + j - layer -> x0];
      pos = i * dctl -> xsize + j;
      if(mmp[pos] == lid)
        mmv[pos] = col;
    }
    */

    if(bx1 <= bx0) {
      continue;
    }
    for(int i = by0; i < by1; ++ i) {
      pos = i * dctl -> xsize + bx0;
      mmp = dctl -> map + pos;
      mmv = dctl -> vram + pos;
      mmb = buf + (i - layer -> y0) * layer -> xsize + bx0 - layer -> x0;
      iip = (int *) mmp;
      iiv = (int *) mmv;
      iib = (int *) mmb;
      len = bx1 - bx0;
      len2 = len >> 2;
      for(int j = 0; j < len2; ++ j) {
        if(iip[j] == sid) {
          iiv[j] = iib[j];
        } else {
          for(int k = 0; k < 4; ++ k) {
            if(mmp[(j << 2) | k] == lid) {
              mmv[(j << 2) | k] = mmb[(j << 2) | k];
            }
          }
        }
      }
      len &= 3;
      for(int j = 1; j <= len; ++ j) {
        if(mmp[bx1 - j - bx0] == lid) {
          mmv[bx1 - j - bx0] = mmb[bx1 - j - bx0];
        }
      }
    }
  }
  return;
}

void map_refresh_sub(int x0, int y0, int x1, int y1, int h0) {
  unsigned char col, *buf, *mmp;
  int tc, lid, bx0, by0, bx1, by1, sid, len, len2;
  for(int h = h0; h <= dctl -> top; ++ h) {
    struct LAYER *layer = dctl -> layers[h];
    tc  = layer -> icol;
    lid = layer - dctl -> layers0;
    buf = layer -> img;
    bx0 = imax(x0, layer -> x0);
    by0 = imax(y0, layer -> y0);
    bx1 = imin(x1, layer -> x0 + layer -> xsize);
    by1 = imin(y1, layer -> y0 + layer -> ysize);
    bx0 = imin(bx0, dctl -> xsize), by0 = imin(by0, dctl -> ysize);
    bx1 = imin(bx1, dctl -> xsize), by1 = imin(by1, dctl -> ysize);
    mmp = dctl -> map;
    if(tc == -1) {
      sid = lid | lid << 8 | lid << 16 | lid << 24;
      for(int i = by0; i < by1; ++ i) {
        int *img = (int *)(dctl -> map + i * dctl -> xsize + bx0);
        len = bx1 - bx0; len2 = len >> 2;
        if(bx1 <= bx0) {
          continue;
        }
        for(int j = 0; j < len2; ++ j) {
          img[j] = sid;
        }
        len &= 3;
        for(int j = 1; j <= len; ++ j) {
          mmp[i * dctl -> xsize + bx1 - j] = lid;
        }
      }
    } else {
      for(int i = by0; i < by1; ++ i) for(int j = bx0; j < bx1; ++ j) {
        col = buf[(i - layer -> y0) * layer -> xsize + j - layer -> x0];
        if(col != tc)
          mmp[i * dctl -> xsize + j] = lid;
      }
    }

  }
  return;
}

void putfont_ascii_in_layer(struct LAYER *layer, int x, int y, int col ,int back, char *str) {
	int len = strlen(str);
	boxfill8(layer -> img, layer -> xsize, back, x, y, x + (len << 3) - 1, y + 15);
	putfont_ascii(layer -> img, layer -> xsize, x, y, col, str);
	display_refresh_layer_sub(layer, x, y, x + (len << 3), y + 16);
	return;
}
