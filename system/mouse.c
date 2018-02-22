# include "bootpack.h"

struct MOUSE_DEC mdec;
struct MOUSE_STATUS mouse;
extern struct BOOTINFO *binfo;
extern struct TASK *task_main;

void inthandler2c(int *esp) { // PS2 Mouse
  unsigned char data;
  io_out8(PIC1_OCW2, 0x64);
  io_out8(PIC0_OCW2, 0x62);
  data = io_in8(PORT_KEYDAT);
  fifo32_push(&task_main -> fifo, 0x01000000 | (((int)data) << 16) );
  return;
}

void init_mouse_cursor(struct MOUSE_STATUS *mouse, char background_color) {
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	for(int i = 0; i < 16; ++ i) for(int j = 0; j < 16; ++ j) {
		if(cursor[i][j] == '*') mouse -> mcur[(i << 4) + j] = COL8_M_GRAY;
		if(cursor[i][j] == 'O') mouse -> mcur[(i << 4) + j] = COL8_WHITE;
		if(cursor[i][j] == '.') mouse -> mcur[(i << 4) + j] = background_color;
	}
	mouse -> mx = binfo -> screen_x >> 1;
	mouse -> my = binfo -> screen_y >> 1;
	return;
}

void enable_mouse(void) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat) {
	if(mdec -> phase == 0) {
		if(dat == 0xfa) mdec -> phase = 1;
		return 0;
	} else {
		if((mdec -> phase == 1) && (dat & 0xc8) != 0x08)
			return 0;
		mdec -> buf[mdec -> phase - 1] = dat;
		mdec -> phase ++;
		if(mdec -> phase == 4) {
			mdec -> phase = 1;
			mdec -> btn = mdec -> buf[0] & 0x07;
			mdec -> x = mdec -> buf[1];
			mdec -> y = mdec -> buf[2];
			if(mdec -> buf[0] & 0x10) mdec -> x |= 0xffffff00; // X Sign Bit
			if(mdec -> buf[0] & 0x20) mdec -> y |= 0xffffff00; // Y Sign Bit
			mdec -> y = - mdec -> y;
			return 1;
		}
		return 0;
	}
	return -1;
}
