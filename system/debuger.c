# include "bootpack.h"

# include <stdio.h>

extern struct BOOTINFO *binfo;
extern unsigned char *blayer_img;
extern struct LYRCTL *dctl;
extern struct LAYER  *blayer;

void dprint_int(int dx) {
  char s[24];
  sprintf(s, "%d", dx);
  dprint_str(s);
  return;
}

void dprint_str(char *dc) {
  boxfill8(blayer_img, binfo -> screen_x, COL8_WHITE, 3, binfo -> screen_y - 45, binfo -> screen_x - 1, binfo -> screen_y - 30);
  putfont_ascii(blayer_img, binfo -> screen_x, 3, binfo -> screen_y - 45, COL8_BLACK, dc);
  display_refresh_layer_sub(blayer, 3, binfo -> screen_y - 45, binfo -> screen_x - 1, binfo -> screen_y - 30);
  return;
}
