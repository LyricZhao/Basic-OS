# include "api.h"

# include <stdio.h>
# include <string.h>
# include <stdlib.h>

void HariMain(void) {
	api_memory_init();
	int win = api_win_open(160, 100, -1, "Stars & Lines");
	int i, x, y, c;
	api_win_boxfill(win, 6, 26, 153, 93, 0);
	for(i = 0; i < 200; ++ i) {
		x = rand() % 147 + 6;
		y = rand() % 67 + 26;
		c = rand() % 15;
		api_win_dw_point(win, x, y, c);
	}
	for(i = 0; i < 8; ++ i) {
		api_win_dw_line(win, 8, 26, 77, i * 9 + 26, i);
		api_win_dw_line(win, 88, 26, i * 9 + 88, 89, i);
	}
	api_win_refresh_sub(win, 6, 26, 154, 94);
	for(;;) {
		int x = api_getkey(1);
		char ss[32]; sprintf(ss, "%d\n", x);
		api_print(ss);
		if(x == (0x0a << 16)) {
			break;
		}
	}
	api_win_close(win);
	api_end();
}
