# include "bootpack.h"
# include <string.h>

extern struct LYRCTL *dctl;

void putfont8(char *vram, int xsize, int x, int y, unsigned char color, char *font) {
	for(int i = 0; i < 16; ++ i) for(int j = 0; j < 8; ++ j) {
		if(font[i] >> j & 1) vram[x + 7 - j + (y + i) * xsize] = color;
	}
	return;
}

void putfont_ascii(char *vram, int xsize, int x, int y, unsigned char color, unsigned char *str) {
	extern char hankaku[4096];
	for(;*str; ++ str, x += 8)
		putfont8(vram, xsize, x, y, color, hankaku + (*str) * 16);
	return;
}

void init_screen(char *vram, int xsize, int ysize) {
/*
	boxfill8(vram, xsize, COL8_WHITE ,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_M_GRAY,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_WHITE ,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_M_GRAY,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_WHITE ,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_WHITE ,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_D_GRAY,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_D_GRAY, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_BLACK ,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_BLACK , 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_D_GRAY, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_D_GRAY, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_WHITE , xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_WHITE , xsize -  3, ysize - 24, xsize -  3, ysize -  3);
	// taskbar is not good
*/
	boxfill8(vram, xsize, COL8_WHITE ,  0,         0,          xsize -  1, ysize -  1);
	return;
}

void boxfill8(char *vram, int width, unsigned char color, int x0, int y0, int x1, int y1) {
	for(int i = x0; i <= x1; ++ i)
		for(int j = y0; j <= y1; ++ j)
			vram[i + j * width] = color;
	return;
}

void boxfill8_in_layer(struct LAYER *layer, unsigned char col, int x0, int y0, int x1, int y1) {
	unsigned char *img = layer -> img;
	int width = layer -> xsize;
	for(int i = x0; i <= x1; ++ i)
		for(int j = y0; j <= y1; ++ j)
			img[i + j * width] = col;
	display_refresh_layer_sub(layer, x0, y0, x1 + 1, y1 + 1);
	return;
}

void make_textbox8(struct LAYER *layer, int x0, int y0, int xsize, int ysize, int col) {
	int x1 = x0 + xsize, y1 = y0 + ysize;
	boxfill8(layer -> img, layer -> xsize, COL8_D_GRAY, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(layer -> img, layer -> xsize, COL8_D_GRAY, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(layer -> img, layer -> xsize,  COL8_WHITE, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(layer -> img, layer -> xsize,  COL8_WHITE, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(layer -> img, layer -> xsize,  COL8_BLACK, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(layer -> img, layer -> xsize,  COL8_BLACK, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(layer -> img, layer -> xsize, COL8_M_GRAY, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(layer -> img, layer -> xsize, COL8_M_GRAY, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(layer -> img, layer -> xsize, 				 col, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	display_refresh_layer_sub(layer, x0 - 3, y0 - 3, x1 + 3, y1 + 3);
	return;
}

void init_palette(void) {
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,
		0xff, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0xff, 0xff, 0x00,
		0x00, 0x00, 0xff,
		0xff, 0x00, 0xff,
		0x00, 0xff, 0xff,
		0xff, 0xff, 0xff,
		0x2d, 0x2d, 0x2d,
		0x84, 0x00, 0x00,
		0x00, 0x84, 0x00,
		0x84, 0x84, 0x00,
		0x00, 0x00, 0x84,
		0x84, 0x00, 0x84,
		0x00, 0x84, 0x84,
		0x84, 0x84, 0x84
	};
	set_palette(0, 15, table_rgb);
	static unsigned char table_rgb2[216 * 3];
	for(int b = 0, t; b < 6; ++ b) {
		for(int g = 0; g < 6; ++ g) {
			for(int r = 0; r < 6; ++ r) {
				t = (r + g * 6 + b * 36) * 3;
				table_rgb2[t + 0] = r * 51;
				table_rgb2[t + 1] = g * 51;
				table_rgb2[t + 2] = b * 51;
			}
		}
	}
	set_palette(16, 231, table_rgb2);
	return;
}

void draw_block(char *vram, int width, int x, int y, int xsize, int ysize, char *arr) {
	for(int i = 0; i < ysize; ++ i) for(int j = 0; j < xsize; ++ j)
		vram[(y + i) * width + x + j] = arr[i * xsize + j];
	return;
}

void set_palette(int start, int end, unsigned char *rgb) {
	int i, eflags;
	eflags = io_load_eflags();
	// CLI - clear interrupt flag
	// STI - set interrupt flag
	io_cli();
	io_out8(0x03c8, start);
	for(i = start; i <= end; ++ i) {
		io_out8(0x03c9, rgb[0] >> 2);
		io_out8(0x03c9, rgb[1] >> 2);
		io_out8(0x03c9, rgb[2] >> 2);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return;
}

void draw_center_window(unsigned char *vram, int xsize, int ysize, char *title, char status) {
	boxfill8(vram, xsize, COL8_M_GRAY, 0,         0,         xsize - 1, 0        );
	boxfill8(vram, xsize, COL8_WHITE , 1,         1,         xsize - 2, 1        );
	boxfill8(vram, xsize, COL8_M_GRAY, 0,         0,         0,         ysize - 1);
	boxfill8(vram, xsize, COL8_WHITE , 1,         1,         1,         ysize - 2);
	boxfill8(vram, xsize, COL8_D_GRAY, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(vram, xsize, COL8_BLACK , xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(vram, xsize, COL8_D_GRAY, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(vram, xsize, COL8_D_GRAY, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(vram, xsize, COL8_BLACK , 0,         ysize - 1, xsize - 1, ysize - 1);
	make_wtitle(vram, xsize, title, status);
	return;
}

void make_wtitle(unsigned char *img, int xsize, char *title, char status) {
	static char xbtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	char tc, tbc;
	if(status){
		tc = COL8_WHITE;
		tbc = COL8_D_BLUE;
	} else {
		tc = COL8_M_GRAY;
		tbc = COL8_D2_BLUE;
	}
	boxfill8(img, xsize, tbc, 3, 3, xsize - 4, 20);
	putfont_ascii(img, xsize, (xsize >> 1) - (strlen(title) << 2), 4, tc, title);
	char col;
	for(int i = 0; i < 14; ++ i) for(int j = 0; j < 16; ++ j) {
		col = xbtn[i][j];
		if(col == '@') col = COL8_BLACK;
		else if(col == '$') col = COL8_D_GRAY;
		else if(col == 'Q') col = COL8_M_GRAY;
		else col = COL8_WHITE;
		img[(5 + i) * xsize + 5 + j] = col;
	}
	return;
}
