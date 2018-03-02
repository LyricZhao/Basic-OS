#include<stdio.h>
#include<stdlib.h>
#include"gobang.h"
#include"api.h"

#define LEN 20
#define MAPLEN 360
#define MAXLEN 5

int length = MAPLEN / LEN;
int map[LEN + 1][LEN + 1];

int HariMain() {
  api_memory_init();
  int win = buildMap(LEN);
  initial();
  int result = 0, color = 1;
  do {
    onestep(win, color);
    color = -color;
  } while(!(result = check()));
  winning(win, result);
  api_getkey(1);
  api_win_close(win);
  api_end();
}

int buildMap() {
  int win = api_win_open(MAPLEN + 12, MAPLEN + 100 , -1, "Gobang");
  api_win_boxfill(win, 6, 26, MAPLEN + 6, MAPLEN + 26, 0);
  for (int i = 1; i < LEN; i++) {
    api_win_dw_line(win, 6 + i * length, 26, 6 + i * length, MAPLEN + 26, 1);
    api_win_dw_line(win, 6, 26 + i * length, MAPLEN + 6, 26 + i * length, 1);
  }
  api_win_boxfill(win, 6, MAPLEN + 26 + 6, MAPLEN + 6, MAPLEN + 24 + 26 + 6, 0);
  api_win_boxfill(win, 6, MAPLEN + 26 + 36, MAPLEN + 6, MAPLEN + 26 + 36 + 24, 0);
  //drawchess(win, 10, 10, 1);
  api_win_refresh_sub(win, 6, 26, MAPLEN + 6, MAPLEN + 26);
  return win;
}

void drawchess(int win, int pi, int pj, int color) {
  int x0 = 6 + (pi - 1) * length;
  int y0 = 26 + (pj - 1) * length;
  if (color == 1)
    api_win_boxfill(win, x0 + length / 4, y0 + length / 4, x0 + 3 * length / 4, y0 + 3 * length / 4, 3);
  else
    api_win_boxfill(win, x0 + length / 4, y0 + length / 4, x0 + 3 * length / 4, y0 + 3 * length / 4, 5);
}

void onestep(int win, int color) {
  int succeed = 0;
  while (!succeed){
  int input = 0;
  int pi = 0, pj = 0, k = 0;

  while((input = getinput(api_getkey(1))) != ' ') {
    if(input < '0' || input > '9') {
      writestr(win, "Invalid Input! Press any key to continue.", 1);
      api_getkey(1);
      api_win_boxfill(win, 6, MAPLEN + 26 + 36, MAPLEN + 6, MAPLEN + 26 + 36 + 24, 0);
      continue;
    }
    pi = pi * 10 + char_to_num(input);
    char pi_str[2];  sprintf(pi_str, "%d", char_to_num(input));
    api_win_print(win, 8 + k * 8, MAPLEN + 26 + 12, 1, 1, pi_str);
    k++;
  }
  k++;
  while((input = getinput(api_getkey(1))) != '\n'){
    if(input < '0' || input > '9') {
      writestr(win, "Invalid Input! Press any key to continue.", 1);
      api_getkey(1);
      api_win_boxfill(win, 6, MAPLEN + 26 + 36, MAPLEN + 6, MAPLEN + 26 + 36 + 24, 0);
      continue;
    }
    pj = pj * 10 + char_to_num(input);
    char pj_str[2];  sprintf(pj_str, "%d", char_to_num(input));
    api_win_print(win, 8 + k * 8, MAPLEN + 26 + 12, 1, 1, pj_str);
    k++;
  }

  if (pi <=0 || pi >= LEN || pj <= 0 || pj >= LEN || map[pi][pj]) {
    char invalid[60] = "Invalid Input! Press any key to continue.";
    writestr(win, invalid, 1);
    api_getkey(1);
    api_win_boxfill(win, 6, MAPLEN + 26 + 6, MAPLEN + 6, MAPLEN + 24 + 26 + 6, 0);
    api_win_boxfill(win, 6, MAPLEN + 26 + 36, MAPLEN + 6, MAPLEN + 26 + 36 + 24, 0);
    api_win_refresh_sub(win, 6, MAPLEN + 26 + 6, MAPLEN + 6, MAPLEN + 26 + 36 + 24);
  }
  else {
    succeed = 1;
    map[pi][pj] = color;
    drawchess(win, pi, pj, color);
    api_win_boxfill(win, 6, MAPLEN + 26 + 6, MAPLEN + 6, MAPLEN + 24 + 26 + 6, 0);
    api_win_refresh_sub(win, 6, MAPLEN + 26 + 6, MAPLEN + 6, MAPLEN + 26 + 36 + 24);
  }
}
}

int getinput(int key) { return ((key & 0x00FF0000) >> 16); }

int char_to_num(int input) { return input - '0'; }

void winning(int win, int color) {
  char winner1[20] = "Winner: yellow";
  char winner2[20] = "winner: pink";
  if(color == 1) {
    writestr(win, winner1, 1);
  }
  else writestr(win, winner2, 1);
}

void writestr(int win, char * str, int len) {
  api_win_boxfill(win, 6, MAPLEN + 26 + 36, MAPLEN + 6, MAPLEN + 26 + 36 + 24, 0);
  api_win_print(win, 8, MAPLEN + 26 + 36, 1, len, str);
  api_win_refresh_sub(win, 6, MAPLEN + 26 + 6, MAPLEN + 6, MAPLEN + 26 + 36 + 24);
}

void initial() {
  for(int i = 1; i <= LEN; i++) {
    for(int j = 1; j <= LEN; j++) {
      map[i][j] = 0;
    }
  }
}

int check_v() {
  for(int j = 1; j <= LEN; j++) {
    int color = 0, num = 1;
    for (int i = 1; i <= LEN; i++) {
      (!map[i][j] || map[i][j] != color) ? num = 1 : num++;
      color = map[i][j];
      if(num == MAXLEN) return color;
    }
  }
  return 0;
}

int check_h() {
  for (int i = 1; i <= LEN; i++) {
    int color = 0, num = 1;
    for (int j = 1; j <=LEN; j++) {
      (!map[i][j] || map[i][j] != color) ? num = 1 : num++;
      color = map[i][j];
      if(num == MAXLEN) return color;
    }
  }
  return 0;
}

int check_dia() {
  for (int k = 1; k <= LEN; k++) {
    int color = 0, num = 1;
    for (int i = k, j = 1; i <= LEN; i++, j++) {
      (!map[i][j] || map[i][j] != color) ? num = 1 : num++;
      color = map[i][j];
      if(num == MAXLEN) return color;
    }
  }
  for (int k = 2; k <= LEN; k++) {
    int color = 0, num = 1;
    for (int j = k, i = 1; j <= LEN; j++, i++) {
      (!map[i][j] || map[i][j] != color) ? num = 1 : num++;
      color = map[i][j];
      if(num == MAXLEN) return color;
    }
  }
  return 0;
}

int check_anti_dia() {
  for (int k = 1; k <= LEN; k++) {
    int color = 0, num = 1;
    for (int i = k, j = 1; i >= 1; i--, j++) {
      (!map[i][j] || map[i][j] != color) ? num = 1 : num++;
      color = map[i][j];
      if(num == MAXLEN) return color;
    }
  }
  for (int k = 2; k <= LEN; k++) {
    int color = 0, num = 1;
    for (int j = k, i = LEN; j <= LEN; j++, i--){
      (!map[i][j] || map[i][j] != color) ? num = 1 : num++;
      color = map[i][j];
      if(num == MAXLEN) return color;
    }
  }
  return 0;
}

int check() {
  int c = check_h() + check_v() + check_dia() + check_anti_dia();
  if(c == 0) return 0;
  else if (c > 0)return 1;
  else return -1;
}
