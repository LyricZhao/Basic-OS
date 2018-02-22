# include "bootpack.h"

# define FLAGS_OVERRUN 0x0001

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf) {
  fifo -> size = size;
  fifo -> buf = buf;
  fifo -> free = size;
  fifo -> flags =  fifo -> p = fifo -> q = 0;
  return;
}

int fifo8_push(struct FIFO8 *fifo, unsigned char data) {
  if(!fifo -> free) {
    fifo -> flags |= FLAGS_OVERRUN;
    return 0;
  }
  fifo -> buf[fifo -> p ++ ] = data;
  if(fifo -> p == fifo -> size)
    fifo -> p = 0;
  -- fifo -> free;
  return 1;
}

unsigned char fifo8_pop(struct FIFO8 *fifo) {
  if(fifo -> free == fifo -> size)
    return -1;
  unsigned char data = fifo -> buf[fifo -> q];
  ++ fifo -> q;
  if(fifo -> q == fifo -> size)
    fifo -> q = 0;
  ++ fifo -> free;
  return data;
}

int fifo8_size(struct FIFO8 *fifo) {
  return fifo -> size - fifo -> free;
}

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task) {
  fifo -> size = size;
  fifo -> buf = buf;
  fifo -> free = size;
  fifo -> flags =  fifo -> p = fifo -> q = 0;
  fifo -> task = task;
  return;
}

int fifo32_push(struct FIFO32 *fifo, int data) {
  if(!fifo -> free) {
    fifo -> flags |= FLAGS_OVERRUN;
    return 0;
  }
  fifo -> buf[fifo -> p ++ ] = data;
  if(fifo -> p == fifo -> size)
    fifo -> p = 0;
  -- fifo -> free;
  if(fifo -> task) {
    if(fifo -> task -> flags != TASK_FLAGS_USING) {
      task_run(fifo -> task, -1, 0);
    }
  }
  return 1;
}

int fifo32_pop(struct FIFO32 *fifo) {
  if(fifo -> free == fifo -> size)
    return -1;
  int data = fifo -> buf[fifo -> q];
  ++ fifo -> q;
  if(fifo -> q == fifo -> size)
    fifo -> q = 0;
  ++ fifo -> free;
  return data;
}

int fifo32_size(struct FIFO32 *fifo) {
  return fifo -> size - fifo -> free;
}
