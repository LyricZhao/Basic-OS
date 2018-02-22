# include "bootpack.h"

int imax(int a, int b) { if(a > b) return a; else return b; }
int imin(int a, int b) { if(a < b) return a; else return b; }
void iswap(int *a, int *b) { *a ^= *b; *b ^= *a; *a ^= *b; return; }

void cUpper(char *str) {
  for(;; ++ str) {
    if(!(*str)) break;
    if('a' <= *str && *str <= 'z') {
      *str ^= 32;
    }
  }
  return;
}
