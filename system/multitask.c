# include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;
extern struct MEM_MANAGER *memc;

struct TASK *multitask_init(void) {
  struct TASK *task;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;

  taskctl = (struct TASKCTL *) memory_alloc_4k(memc, sizeof(struct TASKCTL));
  for (int i = 0; i < MAX_TASKS; ++ i) {
		taskctl -> tasks0[i].flags = 0;
		taskctl -> tasks0[i].gdt_sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}

  for (int i = 0; i < MAX_LEVELS; ++ i) {
		taskctl -> levels[i].tot = 0;
		taskctl -> levels[i].current = 0;
	}

  task = task_alloc();
	task -> flags = 2;
	task -> priority = 2;
	task -> level = 0;
	task_add(task);
	task_switchsub();
	load_tr(task -> gdt_sel);
	task_timer = timer_alloc();
	timer_countdown(task_timer, task -> priority);

  return task;
}

struct TASK *task_alloc(void) {
	struct TASK *task;
	for (int i = 0; i < MAX_TASKS; ++ i) {
		if (taskctl -> tasks0[i].flags == 0) {
			task = &taskctl -> tasks0[i];
			task -> flags = 1;
			task -> tss.eflags = 0x00000202;
			task -> tss.eax = 0;
			task -> tss.ecx = 0;
			task -> tss.edx = 0;
			task -> tss.ebx = 0;
			task -> tss.ebp = 0;
			task -> tss.esi = 0;
			task -> tss.edi = 0;
			task -> tss.es = 0;
			task -> tss.ds = 0;
			task -> tss.fs = 0;
			task -> tss.gs = 0;
			task -> tss.ldtr = 0;
			task -> tss.iomap = 0x40000000;
			task -> tss.ss0 = 0;
			return task;
		}
	}
	return 0;
}

void task_run(struct TASK *task, int level, int priority) {
	if(level < 0) {
		level = task -> level;
	}
	if(priority > 0) {
		task -> priority = priority;
	}
	if (task -> flags == 2 && task -> level != level) {
		task_del(task);
	}
	if (task -> flags != 2) {
		task -> level = level;
		task_add(task);
	}
	taskctl -> lv_change = 1;
	return;
}

void task_sleep(struct TASK *task) {
	struct TASK *ntask;
	if (task -> flags == 2) {
		ntask = task_now();
		task_del(task);
		if (task == ntask) {
			task_switchsub();
			ntask = task_now();
			farjump(0, ntask -> gdt_sel);
		}
	}
	return;
}

void task_switch(void) {
	struct TASKLV *lv = &taskctl -> levels[taskctl -> clv];
	struct TASK *ntask, *ctask = lv -> tasks[lv -> current];
	lv -> current++;
	if(lv -> current == lv -> tot) {
		lv -> current = 0;
	}
	if(taskctl -> lv_change) {
		task_switchsub();
		lv = &taskctl -> levels[taskctl -> clv];
	}
	ntask = lv -> tasks[lv -> current];
	timer_countdown(task_timer, ntask -> priority);
	if (ntask != ctask) {
		farjump(0, ntask -> gdt_sel);
	}
	return;
}

struct TASK *task_now(void) {
  struct TASKLV *lv = &taskctl -> levels[taskctl -> clv];
  return lv -> tasks[lv -> current];
}

void task_add(struct TASK *task) {
  struct TASKLV *lv = &taskctl -> levels[task -> level];
  lv -> tasks[lv -> tot ++] = task;
  task -> flags = TASK_FLAGS_USING;
  return;
}

void task_del(struct TASK *task) {
  struct TASKLV *lv = &taskctl -> levels[task -> level];
  int i;
  for(i = 0; i < lv -> tot; ++ i) {
    if(lv -> tasks[i] == task) {
      break;
    }
  }
  -- lv -> tot;
  if(i < lv -> current) {
    -- lv -> current;
  }
  if(lv -> current >= lv -> tot) {
    lv -> current = 0;
  }
  task -> flags = TASK_FLAGS_ALLOC;
  for(; i < lv -> tot; ++ i) {
    lv -> tasks[i] = lv -> tasks[i + 1];
  }
  return;
}

void task_switchsub(void) {
  int i;
  for(i = 0; i < MAX_LEVELS; ++ i) {
    if(taskctl -> levels[i].tot > 0) {
      break;
    }
  }
  taskctl -> clv = i;
  taskctl -> lv_change = 0;
  return;
}
