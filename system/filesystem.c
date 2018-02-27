# include "bootpack.h"

# include <stdio.h>
# include <string.h>

int *fat_table;
struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
extern struct MEM_MANAGER *memc;

void FAT_init(void) {
  fat_table = (int *)memory_alloc_4k(memc, 4 * 2880);
  unsigned char *img = (unsigned char *)(ADR_DISKIMG + 0x000200);
  for(int i = 0, j = 0; i < 2880; i += 2, j += 3) {
    fat_table[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
    fat_table[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
  }
  return;
}

void command_ls(struct CONSOLE *con) {
  char output[32];
  for(int i = 0; i < 224; ++ i) {
    if(finfo[i].name[0] == 0) {
      break;
    }
    if(finfo[i].name[0] != 0xe5) {
      if((finfo[i].type & 0x18) == 0) {
        sprintf(output, "filename.ext   %7d\n", finfo[i].size);
        for(int j = 0; j < 8; ++ j) {
          output[j] = finfo[i].name[j];
        }
        output[ 9] = finfo[i].ext[0];
        output[10] = finfo[i].ext[1];
        output[11] = finfo[i].ext[2];
        print_screen(con, output, strlen(output));
      }
    }
  }
  return;
}

void command_cat(struct CONSOLE *con, char *para) {
  struct FILEINFO *file = find_file(para);
  if(file == 0) {
    print_screen(con, "No Such File.\n", 14);
  } else {
    cat_output(con, file);
  }
  return;
}

char check_fname(struct FILEINFO *file, char *name) {
  int len = strlen(name), x, y;
  char fname[8];
  memcpy(fname, file -> name, 8);
  cUpper(fname), cUpper(name);
  for(x = 7; ; -- x) {
    if(fname[x] != ' ') {
      break;
    }
  }
  if(x + 1 >= len) {
    return 0;
  }
  for(int i = 0; i <= x; ++ i) {
    if(fname[i] != name[i]) {
      return 0;
    }
  }
  ++ x;
  if(name[x] != '.') {
    return 0;
  }
  memcpy(fname, file -> ext, 3);
  for(y = 2; ; -- y) {
    if(fname[y] != ' ') {
      break;
    }
  }
  ++ x;
  if(x + y != len - 1) {
    return 0;
  }
  for(int i = 0; i <= y; ++ i) {
    if(fname[i] != name[x + i]) {
      return 0;
    }
  }
  return 1;
}

void load_file(struct FILEINFO *file, char *targ) {
  char *img = (char *)(ADR_DISKIMG + 0x003e00);
  int cno = file -> clustno, size = file -> size, len;
  for(;size; cno = fat_table[cno]) {
    len = imin(512, size);
    memcpy(targ, img + cno * 512, len);
    targ += len;
    size -= len;
  }
  return;
}

struct FILEINFO *find_file(char *para) {
  for(int i = 0; i < 224; ++ i) {
    if(finfo[i].name[0] == 0) {
      break;
    }
    if(finfo[i].name[0] != 0xe5) {
      if((finfo[i].type & 0x18) == 0) {
        if(check_fname(&finfo[i], para)) {
          return &finfo[i];
        }
      }
    }
  }
  return 0;
}

void cat_output(struct CONSOLE *con, struct FILEINFO *file) {
  char *img = (char *)(ADR_DISKIMG + 0x003e00);
  int cno = file -> clustno, size = file -> size, len;
  for(;size; cno = fat_table[cno]) {
    // dprint_int(cno);
    len = imin(512, size);
    print_screen(con, img + cno * 512, len);
    size -= len;
  }
  return;
}
