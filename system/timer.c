# include "bootpack.h"

struct TIMERCTL timerctl;
extern struct TIMER *task_timer;

extern struct LAYER *wlayer;

void init_PIT(void) {
  io_out8(PIT_CTRL, 0x34);
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e); // DEC(0x2e9c) = OCT(10), 100 Hz, 0.01s per interrupt
  timerctl.count = 0;
  timerctl.next  = 0xffffffff;
  timerctl.tot   = 0;
  for(int i = 0; i < MAX_TIMERS; ++ i)
    timerctl.timers0[i].flags = 0;
  return;
}

void inthandler20(int *esp) {
  io_out8(PIC0_OCW2, 0x60); /* Tell PIC IRQ-00 ok */
  timerctl.count ++;
  if(timerctl.next > timerctl.count)
    return;
  char tsw = 0;
  for(int i = 0; i < timerctl.tot; ++ i) if(timerctl.timers[i] -> flags == TIMER_FLAGS_USING) {
    struct TIMER *timer = timerctl.timers[i];
    if(timer -> timeout <= timerctl.count) {
      timer -> flags = TIMER_FLAGS_ALLOC;
      if(timer == task_timer) {
        tsw = 1;
      } else {
        fifo32_push(timer -> tq, 0x02000000 | (((int)timer -> data) << 16));
      }
    } else if(timer -> timeout < timerctl.next)
      timerctl.next = timer -> timeout;
  }
  if(tsw) {
    task_switch();
  }
  return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *tq, unsigned char data) {
  timer -> tq = tq;
  timer -> data = data;
  return;
}

struct TIMER *timer_alloc(void) {
  for(int i = 0; i < MAX_TIMERS; ++ i) {
    if(!timerctl.timers0[i].flags) {
      timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
      timerctl.timers[timerctl.tot ++] = &timerctl.timers0[i];
      timerctl.timers[timerctl.tot - 1] -> pos = i;
      return &timerctl.timers0[i];
    }
  }
  return 0;
}

void timer_free(struct TIMER *timer) {
  timer -> flags = 0;
  -- timerctl.tot;
  if(timer -> pos != timerctl.tot) {
    timerctl.timers[timer -> pos] = timerctl.timers[timerctl.tot];
    timerctl.timers[timerctl.tot] -> pos = timer -> pos;
  }
  return;
}

void timer_countdown(struct TIMER *timer, unsigned int timeout) {
  int eflags = io_load_eflags();
  io_cli();
  timer -> count = timeout;
  timer -> timeout = timeout + timerctl.count;
  timer -> flags = TIMER_FLAGS_USING;
  if(timer -> timeout < timerctl.next)
    timerctl.next = timer -> timeout;
  io_store_eflags(eflags);
  return;
}
