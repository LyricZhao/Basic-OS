# include "bootpack.h"

# include <stdio.h>
# include <string.h>

extern struct LYRCTL *dctl;
extern struct LAYER *blayer;
extern struct WINDOW *key_window;

void con_putchar(struct CONSOLE *con, char c) {
  print_screen(con, &c, 1);
  return;
}

void con_print(struct CONSOLE *con, char *str) {
  int len = strlen(str);
  print_screen(con, str, len);
  return;
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
  int cs_base = *((int *) 0xfe8);
  struct TASK *task = task_now();
  struct CONSOLE *con = (struct CONSOLE *) *((int *) 0x0fec);
  int *reg = &eax + 1;
    /* REG[0] : EDI, REG[1] : ESI, REG[2] : EBP, REG[3] : ESP */
    /* REG[4] : EBX, REG[5] : EDX, REG[6] : ECX, REG[7] : EAX */
  switch (edx) {
    // Force Terminating
    case 0: {
      return &(task -> tss.esp0);
    }

    // Print A Character In Console
    case 1: {
      con_putchar(con, eax & 0xff);
      break;
    }

    // Print A String In Console
    case 2: {
      con_print(con, (char *)ebx + cs_base);
      break;
    }

    // Window
    case 3: {
      struct WINDOW *window = window_alloc();
      window_set(window, (char *) ecx + cs_base, esi, edi, eax, 100, 50, 2, 1, task);
      win_key_off(key_window);
      win_key_on(window);
      reg[7] = (int) window;
      break;
    }

    // GUI Put String
    case 4: {
      struct WINDOW *window = (struct WINDOW *) ebx;
      putfont_ascii(window -> img, window -> layer -> xsize, esi, edi, eax, (char *)ebp + cs_base);
      display_refresh_layer_sub(dctl, window -> layer, esi, edi, esi + strlen((char *)ebp + cs_base) * 8, edi + 16);
      break;
    }

    // GUI BoxFill
    case 5: {
      struct WINDOW *window = (struct WINDOW *) ebx;
      boxfill8_in_layer(window -> layer, ebp, eax, ecx, esi, edi);
      break;
    }

    // Memory Manager Initialization
    case 6: {
      struct MEM_MANAGER *app_memc = (struct MEM_MANAGER *) (ebx + cs_base);
      memory_management_init(app_memc);
      ecx &= 0xfffffff0;
      memory_free(app_memc, eax, ecx);
      break;
    }

    // MM: malloc
    case 7: {
      struct MEM_MANAGER *app_memc = (struct MEM_MANAGER *) (ebx + cs_base);
      ecx = (ecx + 0x0f) & 0xfffffff0;
      reg[7] = memory_alloc(app_memc, ecx);
      break;
    }

    // MM: free
    case 8: {
      struct MEM_MANAGER *app_memc = (struct MEM_MANAGER *) (ebx + cs_base);
      ecx = (ecx + 0x0f) & 0xfffffff0;
      memory_free(app_memc, eax, ecx);
      break;
    }

    // GUI Draw Point
    case 9: {
      struct WINDOW *window = (struct WINDOW *) (ebx & 0xfffffffe);
      window -> img[window -> layer -> xsize * edi + esi] = eax;
      if(ebx & 1) {
        display_refresh_layer_sub(dctl, window -> layer, esi, edi, esi + 1, edi + 1);
      }
        break;
    }

    // GUI Refresh Sub
    case 10: {
      struct WINDOW *window = (struct WINDOW *) ebx;
      display_refresh_layer_sub(dctl, window -> layer, eax, ecx, esi, edi);
      break;
    }

    // GUI Draw Line
    case 11: {
      struct WINDOW *window = (struct WINDOW *) (ebx & 0xfffffffe);
      api_win_dw_line(window, eax, ecx, esi, edi, ebp);
      if(ebx & 1) {
        display_refresh_layer_sub(dctl, window -> layer, eax, ecx, esi, edi);
      }
      break;
    }

    // GUI Close Window
    case 12: {
      struct WINDOW *window = (struct WINDOW *) ebx;
      layer_del(dctl, window -> layer);
      break;
    }

    // Keyboard
    case 13: {

      for(;;) {
        io_cli();
        if(fifo32_size(&task -> fifo) == 0) {
          if(eax) {
            task_sleep(task);
          } else {
            io_sti();
            reg[7] = -1;
            return 0;
          }
        } else {
          int dat, itype0, itype1;
          dat = fifo32_pop(&task -> fifo);
          io_sti();
          itype0 = (dat & 0xff000000) >> 24;
          itype1 = (dat & 0x00ff0000) >> 16;
          switch(itype0) {
            case 0: {
              reg[7] = itype1;
              return 0;
            }
            case 2: {
              timer_countdown(con -> timer, 50);
              break;
            }
            default: {
              putfont_ascii_in_layer(blayer, 0, 64, COL8_BLACK, COL8_WHITE, "ERROR");
              break;
            }
          }
        }
      }
      break;
    }

    // Unknown Call
    default:
      print_screen(con, "Error in API call\n", 18);
      break;
  }
  return 0;
}

int *inthandler0d(int *esp) {
  struct CONSOLE *con = (struct CONSOLE *) *((int *) 0x0fec);
  struct TASK *task = task_now();
  con_print(con, "\nINT 0D:\nGeneral Protected Exception.\n");
  char esp_info[32];
  sprintf(esp_info, "EIP = %08x\n", esp[11]);
  con_print(con, esp_info);
  return &(task -> tss.esp0);
}

int *inthandler0c(int *esp) {
  struct CONSOLE *con = (struct CONSOLE *) *((int *) 0x0fec);
  struct TASK *task = task_now();
  con_print(con, "\nINT 0C:\nStack Exception.\n");
  char esp_info[32];
  sprintf(esp_info, "EIP = %08x\n", esp[11]);
  con_print(con, esp_info);
  return &(task -> tss.esp0);
}

void api_win_dw_line(struct WINDOW *window, int x0, int y0, int x1, int y1, int col) {
  char *img = window -> img;
  int x, y, dx, dy;
  int len, xsize = window -> layer -> xsize;
  dx = x1 - x0, dy = y1 - y0;
  x = x0 << 10, y = y0 << 10;
  if(dx >= dy) {
    len = dx + 1;
    if(x0 > x1) dx = - 1024;
    else dx = 1024;
    if(y0 <= y1) dy = ((y1 - y0 + 1) << 10) / len;
    else dy = ((y1 - y0 - 1) << 10) / len;
  } else {
    len = dy + 1;
    if(y0 > y1) dy = - 1024;
    else dy = 1024;
    if(x0 <= x1) dx = ((x1 - x0 + 1) << 10) / len;
    else dx = ((x1 - x0 - 1) << 10) / len;
  }
  for(int i = 0; i < len; ++ i) {
    img[(y >> 10) * xsize + (x >> 10)] = col;
    x += dx, y += dy;
  }
  return;
}
