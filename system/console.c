# include "bootpack.h"

# include <stdio.h>
# include <string.h>

struct CONSOLE econs[MAX_CONSOLES];
int cons_tot;
struct CONSOLE *key_console;
int now_running_con;

extern struct TASK *task_main;
extern struct MEM_MANAGER *memc;
extern struct LYRCTL *dctl;
extern struct LAYER *blayer;
extern struct WINDOWCTL *wctl;
extern struct WINDOW *key_window;

struct CONSOLE *new_console(void) {
  ++ now_running_con;
  struct CONSOLE *console = &econs[cons_tot ++];
  console_task_init(console);
  console_window_init(console);
  console_key_on(console);
  win_key_off(key_window);
  win_key_on(console -> window);
  layer_ud(console -> window -> layer, dctl -> top - 1);
  console -> in_app = 0;
  console -> run_out = 1;
  return console;
}

void console_close(struct LAYER *layer) {
  struct CONSOLE *console = layer -> window -> console;
  memory_free_4k(memc, (unsigned int)console -> window -> img, 256 * 165);
  layer_del(console -> window -> layer);
  struct TASK *task = console -> task;
  task_sleep(task);
  console -> window -> layer = 0;
  memory_free_4k(memc, task -> cons_stack, 64 * 1024);
  task -> flags = 0;
  now_running_con --;
  if(!now_running_con) {
    key_console = 0;
  }
  return;
}

void console_key_on(struct CONSOLE *console) {
  key_console = console;
  // *((int *) 0x0fec) = (int) console;
  return;
}

void move_oneline(unsigned char *img , int xsize) {
  for(int y = 28; y < 28 + 112; ++ y) for(int x = 8; x < 8 + 240; ++ x)
    img[x + y * xsize] = img[x + (y + 16) * xsize];
  for(int y = 28 + 112; y < 28 + 128; ++ y) for(int x = 8; x < 8 + 240; ++ x)
    img[x + y * xsize] = COL8_BLACK;
  return;
}

void pf_nline(struct CONSOLE *console) {
  struct LAYER *layer = console -> window -> layer;
  int *cx = &(console -> cursor_x);
  int *cy = &(console -> cursor_y);
  if(*cy < 28 + 112)
    *cy += 16;
  else {
    move_oneline(layer -> img, layer -> xsize);
    display_refresh_layer_sub(layer, 8, 28, 8 + 240, 28 + 128);
  }
  *cx = 8;
  return;
}

void console_main(struct CONSOLE *console) {
  struct TIMER *timer;
  struct LAYER *layer = console -> window -> layer;
  struct TASK *task = console -> task;
  int fifobuf[128], cursor_col = COL8_WHITE;
  console -> cursor_x = 16;
  console -> cursor_y = 28;
  char command_line[128]; int command_len = 0;
  fifo32_init(&task -> fifo, 128, fifobuf, console -> task);
  timer = timer_alloc();
  console -> timer = timer;
  timer_init(timer, &task -> fifo, 1);
  timer_countdown(timer, 50);
  int dat, itype0, itype1, ntot = 0; char cstr[32];
  putfont_ascii_in_layer(layer, 8, 28, COL8_WHITE, COL8_BLACK, ">");

  for(;;) {
    io_cli();
    if(!fifo32_size(&task -> fifo)) {
      task_sleep(task);
      io_stihlt();
    } else {
      dat = fifo32_pop(&task -> fifo);
      io_sti();
      itype0 = (dat & 0xff000000) >> 24;
      itype1 = (dat & 0x00ff0000) >> 16;
      switch(itype0) {

        // Keyboard
        case 0:
          switch(itype1) {

            // BackSpace
            case 0x08:
              if(command_len && console -> cursor_x > 16) {
                -- command_len;
                putfont_ascii_in_layer(layer, console -> cursor_x, console -> cursor_y, COL8_WHITE, COL8_BLACK, " ");
                console -> cursor_x -= 8;
              } else if(ntot && console -> cursor_y > 28) {
                -- command_len;
                if(console -> cursor_x == 16) {
                  putfont_ascii_in_layer(layer, console -> cursor_x, console -> cursor_y, COL8_WHITE, COL8_BLACK, " ");
                  console -> cursor_x -= 8;
                } else {
                  putfont_ascii_in_layer(layer, console -> cursor_x, console -> cursor_y, COL8_WHITE, COL8_BLACK, " ");
                  console -> cursor_x = 240;
                  console -> cursor_y -= 16;
                  -- ntot;
                }
              }
              break;

            // Enter
            case 0x0a:
              ntot = 0;
              putfont_ascii_in_layer(layer, console -> cursor_x, console -> cursor_y, COL8_WHITE, COL8_BLACK, " ");
              pf_nline(console);
              command_line[command_len] = 0;
              if(command_len)
                command_handler(console, command_line);
              print_screen(console, ">", 1);
              command_len = 0;
              break;

            // Normal Characters
            default:
              command_line[command_len] = itype1;
              command_len ++;
              if(console -> cursor_x < 240) {
                cstr[0] = itype1, cstr[1] = 0;
                putfont_ascii_in_layer(layer, console -> cursor_x, console -> cursor_y, COL8_WHITE, COL8_BLACK, cstr);
                console -> cursor_x += 8;
              } else {
                ++ ntot;
                cstr[0] = itype1, cstr[1] = 0;
                putfont_ascii_in_layer(layer, console -> cursor_x, console -> cursor_y, COL8_WHITE, COL8_BLACK, cstr);
                pf_nline(console);
              }
              break;
          }
          boxfill8_in_layer(layer, cursor_col, console -> cursor_x, console -> cursor_y, console -> cursor_x + 7, console -> cursor_y + 15);
          break;

        // Console Timer
        case 2:
          cursor_col ^= 7;
          timer_countdown(timer, 50);
          boxfill8_in_layer(layer, cursor_col, console -> cursor_x, console -> cursor_y, console -> cursor_x + 7, console -> cursor_y + 15);
          break;

        // Application Timer - Not Here
        case 3:
          break;

        // Error
        default:
          putfont_ascii_in_layer(blayer, 0, 64, COL8_BLACK, COL8_WHITE, "ERROR");
    			for(;;);
      }

    }
  }
}

void print_screen(struct CONSOLE *console, char *str, int len) {
  // 30 Characters per Line
  struct LAYER *layer = console -> window -> layer;
  int *cx = &(console -> cursor_x);
  int *cy = &(console -> cursor_y);
  char pstack[256];
  int top = ((*cx) >> 3) - 1, x = 0, ex = 0;
  while(x < len) {
    if(str[x] == '\r') {
      goto label;
    }
    if(str[x] == '\t') {
      do {
        pstack[ex ++] = ' ';
        top ++;
        if(top == 30) {
          pstack[ex] = '\0';
          top = 0, ex = 0;
          putfont_ascii_in_layer(layer, *cx, *cy, COL8_WHITE, COL8_BLACK, pstack);
          pf_nline(console);
        }
      } while(top & 3);
    } else if(str[x] != '\n') {
      pstack[ex ++] = str[x];
      top ++;
    }
    label:
    if((x == len - 1) || (str[x] == '\n') || (top == 30)) {
      io_cli();
      pstack[ex] = '\0';
      putfont_ascii_in_layer(layer, *cx, *cy, COL8_WHITE, COL8_BLACK, pstack);
      *cx = (top + 1) << 3;
      if((str[x] == '\n') || (top == 30)) {
        pf_nline(console);
      }
      top = 0, ex = 0;
      io_sti();
    }
    ++ x;
  }
  return;
}

void command_handler(struct CONSOLE *console, char *command) {
  char com[256], para[256];
  int x = 0;
  for(;; ++ x) {
    if(command[x] == ' ' ) break;
    if(command[x] == '\0') break;
  }
  memcpy(com, command, x + 1);
  com[x] = '\0';
  memcpy(para, command + x + 1, strlen(command) - x);
  if(strcmp(com, "mem") == 0) {
    command_mem(console);
  } else if(strcmp(com, "clear") == 0) {
    command_clear(console);
  } else if(strcmp(com, "ls") == 0) {
    command_ls(console);
  } else if(strcmp(com, "cat") == 0) {
    command_cat(console, para);
  } else if(strcmp(com, "run") == 0){
    command_run(console, para);
  } else if(strcmp(com, "exit") == 0) {
    command_exit(console);
  } else {
    print_screen(console, "No such command.\n", 17);
  }
  return;
}

void console_window_init(struct CONSOLE *console) {
  console -> window = window_alloc();
  // dprint_int(console -> window);
  window_set(console -> window, "console", 256, 165, -1, 312, 184, 2, 0, console -> task, console, 0);
  make_textbox8(console -> window -> layer, 8, 28, 240, 128, COL8_BLACK);
  return;
}

void console_task_init(struct CONSOLE *console) {
  struct TASK **task = &(console -> task);
  *task = task_alloc();
  (*task) -> cons_stack = memory_alloc_4k(memc, 64 * 1024);
  (*task) -> tss.esp = (*task) -> cons_stack + 64 * 1024 - 8;
  (*task) -> tss.eip = (int) &console_main;
  (*task) -> tss.es = 1 * 8;
  (*task) -> tss.ss = 1 * 8;
  (*task) -> tss.ds = 1 * 8;
  (*task) -> tss.fs = 1 * 8;
  (*task) -> tss.gs = 1 * 8;
  (*task) -> tss.cs = 2 * 8;
  (*task) -> console = console;
  *((int *)((*task) -> tss.esp + 4)) = (int) console;
  // *((int *) 0x0fec) = (int) console;
  task_run(*task, 2, 2);
  return;
}

void command_clear(struct CONSOLE *console) {
  boxfill8_in_layer(console -> window -> layer, COL8_BLACK, 8, 28, 8 + 240 - 1, 28 + 128 - 1);
  console -> cursor_x = 8;
  console -> cursor_y = 28;
  return;
}

void command_run(struct CONSOLE *console, char *para) {
  struct FILEINFO *file;
  file = find_file(para);
  if(!file) {
    print_screen(console, "Error: no such file.\n", 21);
  } else {
    struct TASK *task = task_now();
    char *app_mem = (char *)memory_alloc_4k(memc, file -> size), *pro_mem;
    load_file(file, app_mem);
    if(file -> size >= 36 && strncmp(app_mem + 4, "Hari", 4) == 0 && *app_mem == 0x00) {
      int segsiz = *((int *) (app_mem + 0x0000));
      int    esp = *((int *) (app_mem + 0x000c));
      int datsiz = *((int *) (app_mem + 0x0010));
      int dathrb = *((int *) (app_mem + 0x0014));
      pro_mem = (char *) memory_alloc_4k(memc, segsiz);
      task -> cs_base = (int) pro_mem;
      struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
      set_segmdesc(gdt + task -> gdt_sel / 8 + 1000, file -> size - 1, (int)app_mem, AR_CODE32_ER + 0x60);
      set_segmdesc(gdt + task -> gdt_sel / 8 + 2000,       segsiz - 1, (int)pro_mem, AR_DATA32_RW + 0x60);
      for(int i = 0; i < datsiz; ++ i)
        pro_mem[esp + i] = app_mem[dathrb + i];
      console -> in_app = 1;
      console -> run_out = 0;
      start_app(0x1b, task -> gdt_sel + 1000 * 8, esp, task -> gdt_sel + 2000 * 8, &(task -> tss.esp0));
      console -> in_app = 0;
      for(int i = 0; i < wctl -> tot; ++ i) {
        struct WINDOW *window = &(wctl -> windows[i]);
        if(window -> layer == 0) {
          continue;
        }
        if(window -> task == task && (window != (console -> window))) {
          win_del(window);
        }
      }
    //  dprint_int(times);
      win_key_off(key_window);
      win_key_on(console -> window);
      console_key_on(console);
      layer_ud(console -> window -> layer, dctl -> top - 1);
      memory_free_4k(memc, (int) pro_mem, segsiz);
    } else {
      print_screen(console, "Error: not a standard executable file.\n", 39);
    }
    memory_free_4k(memc, (int) app_mem, file -> size);
  }
  console -> run_out = 1;
  fifo32_push(&(task_main -> fifo), 0x23);
  return;
}

extern struct TASK *task_main;

void command_exit(struct CONSOLE *console) {
  timer_free(console -> timer);
  struct FIFO32 *fifo = &(task_main -> fifo);
  io_cli();
  fifo32_push(fifo, (3 << 24) | ((console -> window -> layer - dctl -> layers0) << 16));
  io_sti();
  for(;;) {
    task_sleep(console -> task);
  }
  return;
}
