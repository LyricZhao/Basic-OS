// # define ENABLE_CPU_HLT 	TRUE

# include "bootpack.h"

# include <stdio.h>
# include <string.h>

/* BootInfo */
struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;

/* Mouse */
extern struct MOUSE_DEC mdec;
extern struct MOUSE_STATUS mouse;

/* Memory Management */
extern struct MEM_MANAGER *memc;

/* Layer Management */
extern struct LYRCTL *dctl;
struct LAYER *mlayer;
struct LAYER *blayer;
unsigned char *blayer_img;
struct WINDOW *wmain;

/* Time Management */
int current_time;
extern struct TIMERCTL timerctl;
struct TIMER *sys_timer;

/* Event FIFO */
int event_queue_buf[256];

/* TextBox */
int cursor_x, cursor_col;

/* Keyboard Table */
extern char keytable0[0x80];
extern char keytable1[0x80];
int key_leds, keycmd_wait;
int key_shift, key_ctrl;
int keycmd_buf[32];
struct FIFO32 keycmd;

/* Tasks */
struct TASK *task_main;
struct TASK *idle;

/* Console */
extern struct CONSOLE console;

/* Current Focus */
int cfocus = 0;

/* Functions */
void task_idle(void);
void kmt_interrupt(void);
void task_b_main(struct LAYER *layer);

void HariMain(void) {

	/* Hardware Initialization */
	init_gdtidt();
	init_pic();
	io_sti();

	/* Memory Management Initialization */
	memory_init();

	/* PIT(Programmable Interrupt Controller) Initialization */
	init_PIT();

	/* GUI Initialization */
	init_palette();

	/* Enable Mouse & Keyboard */
	io_out8(PIC0_IMR, 0xf8); // PIT & Keyboard
	io_out8(PIC1_IMR, 0xef);
	enable_keyboard();
	enable_mouse();
	init_mouse_cursor(&mouse, COL8_D_GRAY);
	key_shift = 0, key_ctrl = 0;
	key_leds = (binfo -> leds >> 4) & 7;
	keycmd_wait = -1;
	fifo32_init(&keycmd, 32, keycmd_buf, task_main);
	fifo32_push(&keycmd, KEYCMD_LED);
	fifo32_push(&keycmd, key_leds);

	/* FileSystem Initialization */
	FAT_init();

	/* Multitask Initialization */

		// Main Task
	task_main = multitask_init();
	task_run(task_main, 1, 2);

		// Idle Task
	idle = task_alloc();
	idle -> tss.eip = (int) &task_idle;
	idle -> tss.esp = memory_alloc_4k(memc, 64 * 1024) + 64 * 1024;
	idle -> tss.es = 1 * 8;
	idle -> tss.ss = 1 * 8;
	idle -> tss.ds = 1 * 8;
	idle -> tss.fs = 1 * 8;
	idle -> tss.gs = 1 * 8;
	idle -> tss.cs = 2 * 8;
	# ifndef ENABLE_CPU_HLT
		task_run(idle, MAX_LEVELS - 1, 1);
	# endif

		// Console Task
	console_task_init();

	/* GUI Layers */
	lyrctl_init();
	window_init();

		// Background Layer
	blayer = layer_alloc(dctl);
	blayer_img = (unsigned char *) memory_alloc_4k(memc, binfo -> screen_x * binfo -> screen_y);
	init_screen(blayer_img, binfo -> screen_x, binfo -> screen_y);
	layer_bset(blayer, blayer_img, binfo -> screen_x, binfo -> screen_y, -1);
	layer_move(dctl, blayer, 0, 0);
	layer_ud(dctl, blayer, 0);

		// Window Layer
	wmain = window_alloc();
	window_set(   wmain,    "main", 160, 52, -1,   8 + 150,  56, 1, 1, 0);

		// Mouse Layer
	mlayer = layer_alloc(dctl);
	layer_bset(mlayer, mouse.mcur, 16, 16, COL8_D_GRAY);
	layer_move(dctl, mlayer, mouse.mx, mouse.my);
	layer_ud(dctl, mlayer, 5);

		// TextBox
	make_textbox8(wmain -> layer, 8, 28, 144, 16, COL8_WHITE);
	cursor_x = 8, cursor_col = COL8_BLACK;

		// Memory Status
	memory_dprint();

		// Console
	console_window_init();

		// Refresh all
	display_refresh_all(dctl);

	/* Interrupt Event Queue Initialization */
	fifo32_init(&task_main -> fifo, 256, event_queue_buf, task_main);

	/* Timer */
	sys_timer = timer_alloc();
	timer_init(sys_timer, &task_main -> fifo, 0);
	timer_countdown(sys_timer, 100);

	/* Interrupts */
	for(;;) {
		kmt_interrupt();
	 }

	return;
}

void task_idle(void) {
	for(;;)
		io_hlt();
}

void task_b_main(struct LAYER *layer) {
	struct FIFO32 fifo;
	struct TIMER *timerb;
	char str[32];
	int fifobuf[128], count = 0;
	fifo32_init(&fifo, 128, fifobuf, 0);
	timerb = timer_alloc();
	timer_init(timerb, &fifo, 1);
	timer_countdown(timerb, 100);
	for(;;) {
		++ count;
		io_cli();
		if(!fifo32_size(&fifo)) {
			# ifdef ENABLE_CPU_HLT
				io_stihlt();
			# else
				io_sti();
			# endif
		}
		else {
			fifo32_pop(&fifo);
			io_sti();
			sprintf(str, "%11d", count);
			putfont_ascii_in_layer(layer, 24, 28, COL8_BLACK, COL8_D_GRAY, str);
			timer_countdown(timerb, 100);
		}
	}
}

struct LAYER *moving_layer;

void kmt_interrupt() {
	if(fifo32_size(&keycmd) && keycmd_wait < 0) {
		keycmd_wait = fifo32_pop(&keycmd);
		wait_KBC_sendready();
		io_out8(PORT_KEYDAT, keycmd_wait);
	}
	io_cli();
	if(!fifo32_size(&task_main -> fifo)) {
		task_sleep(task_main);
		# ifdef ENABLE_CPU_HLT
			io_stihlt(); // with HLT
		# else
			io_sti(); // without HLT
		# endif
	} else {
		char str[32];
		unsigned int icode0, itype0, itype1, itype2, itype3;
		icode0 = fifo32_pop(&task_main -> fifo);
		io_sti();
		itype0 = (icode0 & 0xff000000) >> 24;
		itype1 = (icode0 & 0x00ff0000) >> 16;
		itype2 = (icode0 & 0x0000ff00) >>  8;
		itype3 = (icode0 & 0x000000ff)      ;
		sprintf(str, "|%08x", icode0);
		putfont_ascii_in_layer(blayer, 16, 16, COL8_BLACK, COL8_RED, str);
		switch (itype0) {

			/* Keyboard Interrupt */
			case 0:
				sprintf(str, "%02X", itype1);
				putfont_ascii_in_layer(blayer, 0, 16, COL8_BLACK, COL8_RED, str);
				if(itype1 < 0x80) {
					if(key_shift == 0)
						str[0] = keytable0[itype1];
					else
						str[0] = keytable1[itype1];
				} else {
					str[0] = 0;
				}

				if('A' <= str[0] && str[0] <= 'Z') {
					if((((key_leds & 4) == 0) ^ (key_shift == 0)) == 0)
						str[0] ^= 32;
				}

				// Normal (Displayable Character)
				if(str[0]) {
					if(key_ctrl) {
						if('A' <= str[0] && str[0] <= 'Z') {
							str[0] ^= 32;
						}
						switch (str[0]) {
							case 'z':
								if(cfocus == 1) {
									if(console.task -> tss.ss0) {
										struct CONSOLE *con = (struct CONSOLE *) *((int *) 0x0fec);
										con_print(con, "Process Is Terminated.\n");
										io_cli();
										console.task -> tss.eax = (int) &(console.task -> tss.esp0);
										console.task -> tss.eip = (int) &asm_end_app;
										io_sti();
									}
								}
								break;
							default:
								break;
						}
					} else {
						if(cfocus == 0) {
							if(cursor_x < 144) {
								str[1] = 0;
								putfont_ascii_in_layer(wmain -> layer, cursor_x, 28, COL8_BLACK, COL8_WHITE, str);
								cursor_x += 8;
								boxfill8(wmain -> layer -> img, wmain -> layer -> xsize, cursor_col, cursor_x, 28, cursor_x + 7, 43);
								display_refresh_layer_sub(dctl, wmain -> layer, cursor_x, 28, cursor_x + 8, 44);
							}
						} else if(cfocus == 1) {
							fifo32_push(&console.task -> fifo, str[0] << 16);
						}
					}
					break;
				}

				switch (itype1) {
					// BackSpace
					case 0x0e:
						if(!cfocus) {
							if(cursor_x > 8) {
								putfont_ascii_in_layer(wmain -> layer, cursor_x, 28, COL8_BLACK, COL8_WHITE, " ");
								cursor_x -= 8;
								boxfill8(wmain -> layer -> img, wmain -> layer -> xsize, cursor_col, cursor_x, 28, cursor_x + 7, 43);
								display_refresh_layer_sub(dctl, wmain -> layer, cursor_x, 28, cursor_x + 8, 44);
							}
						} else {
							fifo32_push(&(console.task -> fifo), 8 << 16);
						}
						break;

					// Tab
					case 0x0f: { // Note Here: {} must be here to define the domain of the variable allocated after 'case'
						struct LAYER *l1 = 					wmain -> layer;
						struct LAYER *l2 = console.window -> layer;
						make_wtitle(l1 -> img, l1 -> xsize,    "main", cfocus    );
						make_wtitle(l2 -> img, l2 -> xsize, "console", cfocus ^ 1);
						cfocus ^= 1;
						display_refresh_layer_sub(dctl, l1, 0, 0, l1 -> xsize, 21);
						display_refresh_layer_sub(dctl, l2, 0, 0, l2 -> xsize, 21);
						break;
					}

					//CapsLock
					case 0x3a:
						key_leds ^= 4;
						fifo32_push(&keycmd, KEYCMD_LED);
						fifo32_push(&keycmd, key_leds);
						break;

					// NumLock
					case 0x45:
						key_leds ^= 2;
						fifo32_push(&keycmd, KEYCMD_LED);
						fifo32_push(&keycmd, key_leds);
						break;

					// ScrollLock
					case 0x46:
						key_leds ^= 1;
						fifo32_push(&keycmd, KEYCMD_LED);
						fifo32_push(&keycmd, key_leds);
						break;

					// Enter
					case 0x1c:
						if(cfocus)
							fifo32_push(&console.task -> fifo, 10 << 16);

					// Send OK
					case 0xfa:
						keycmd_wait = -1;
						break;

					// Send Not OK
					case 0xfe:
						wait_KBC_sendready();
						io_out8(PORT_KEYDAT, keycmd_wait);

					// L-Shift ON
					case 0x2a:
						key_shift |= 1;
						break;

					// R-Shift ON
					case 0x36:
						key_shift |= 2;
						break;

					// L-Ctrl ON
					case 0x1d:
						key_ctrl |= 1;
						break;

					// L-Shift OFF
					case 0xaa:
						key_shift &= ~ 1;
						break;

					// R-Shift OFF
					case 0xb6:
						key_shift &= ~ 2;
						break;

					// L-Ctrl OFF
					case 0x9d:
						key_ctrl &= ~ 1;
						break;

					default:
						break;
				}

				break;


			/* Mouse Interrupt */
			case 1:
				if(mouse_decode(&mdec, itype1)) {
					mouse.mx += mdec.x, mouse.my += mdec.y;
					if(mouse.mx < 0) mouse.mx = 0; if(mouse.my < 0) mouse.my = 0;
					if(mouse.mx + 1 > binfo -> screen_x) mouse.mx = binfo -> screen_x - 1;
					if(mouse.my + 1 > binfo -> screen_y) mouse.my = binfo -> screen_y - 1;
					layer_move(dctl, mlayer, mouse.mx, mouse.my);
					if(mdec.btn & 0x01) {
						int x, y;
						if(mdec.mmx < 0) {
							for(int i = dctl -> top - 1; i > 0; -- i) {
								moving_layer = dctl -> layers[i];
								x = mouse.mx - moving_layer -> x0;
								y = mouse.my - moving_layer -> y0;
								if((0 <= x && x < moving_layer -> xsize) && (0 <= y && y < moving_layer -> ysize)) {
									if(moving_layer -> img[y * moving_layer -> xsize + y] != moving_layer -> icol) {
										layer_ud(dctl, moving_layer, dctl -> top - 1);
										if((3 <= x && x < moving_layer -> xsize - 3) && (3 <= y && y < 21)) {
											mdec.mmx = mouse.mx;
											mdec.mmy = mouse.my;
										}
										if((5 <= x && x < 21) && (5 <= y && y < 19)) {
											if(moving_layer -> task != 0) {
												mdec.mmx = -1;
												struct CONSOLE *con = (struct CONSOLE *) *((int *) 0x0fec);
												con_print(con, "Process Is Terminated.(By Mouse)\n");
												io_cli();
												console.task -> tss.eax = (int) &(console.task -> tss.esp0);
												console.task -> tss.eip = (int) &asm_end_app;
												io_sti();
											}
										}
										break;
									}
								}
							}
						} else {
							x = mouse.mx - mdec.mmx;
							y = mouse.my - mdec.mmy;
							layer_move(dctl, moving_layer, moving_layer -> x0 + x, moving_layer -> y0 + y);
							mdec.mmx = mouse.mx;
							mdec.mmy = mouse.my;
						}
					} else {
						mdec.mmx = -1;
					}
				}
				break;

			/* Timer Interrupt */
			case 2:
					sprintf(str, "%03d[SEC]", ++ current_time);
					putfont_ascii_in_layer(blayer, 0, 64, COL8_BLACK, COL8_WHITE, str);
					timer_countdown(sys_timer, 100);
				break;

			/* Exception */
			default:
				putfont_ascii_in_layer(blayer, 0, 64, COL8_BLACK, COL8_WHITE, "ERROR");
				for(;;);
		}
	}
	return;
}
