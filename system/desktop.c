# include "bootpack.h"

unsigned char rgb2pal(int r, int g, int b, int x, int y) {
	static int table[4] = { 3, 1, 0, 2 };
	int i;
	x &= 1, y &= 1;
	i = table[x + y * 2];
	r = (r * 21) >> 8;
	g = (g * 21) >> 8;
	b = (b * 21) >> 8;
	r = (r + i) >> 2;
	g = (g + i) >> 2;
	b = (b + i) >> 2;
	return 16 + r + g * 6 + b * 36;
}

extern struct LAYER *blayer;
extern unsigned char *blayer_img;
extern struct MEM_MANAGER *memc;


void load_desktop(void) {
  struct DLL_STRPICENV *env = (struct DLL_STRPICENV *)memory_alloc_4k(memc, sizeof(struct DLL_STRPICENV));
  char *filebuf = (char *)memory_alloc_4k(memc, 512 * 1024);
  struct RGB *picbuf = (struct RGB *)memory_alloc_4k(memc, 1024 * 768 * sizeof(struct RGB));
  char *img = blayer_img;
  struct FILEINFO *file = find_file("desktop.jpg");
  load_file(file, filebuf);
  int rb = decode0_JPEG(env, file -> size, (unsigned char *)filebuf, 4, (unsigned char *) picbuf, 0);
  if(rb) {
    dprint_str("ERROR");
    for(;;);
  }
  for(int i = 0; i < 480; ++ i) {
    char *p = img + i * 640;
    struct RGB *q = picbuf + i * 640;
    for(int j = 0; j < 640; ++ j) {
      p[j] = rgb2pal(q[j].r, q[j].g, q[j].b, j, i);
    }
  }
  display_refresh_all();
  memory_free_4k(memc, (int)filebuf, 512 * 1024);
  memory_free_4k(memc, (int)picbuf, 1024 * 768 * sizeof(struct RGB));
  memory_free_4k(memc, (int)env, sizeof(struct DLL_STRPICENV));
  return;
}
