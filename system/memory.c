# include "bootpack.h"

# include <stdio.h>
# include <string.h>

extern struct BOOTINFO *binfo;

unsigned int mem_total;
struct MEM_MANAGER *memc = (struct MEM_MANAGER *) MEM_MANAGER_ADDR;
extern struct LAYER *blayer;

void memory_init(void) {
  mem_total = memtest(0x00400000, 0xbfffffff);
  memory_management_init(memc);
  memory_free(memc, 0x00001000, 0x0009e000);
  memory_free(memc, 0x00400000, mem_total - 0x00400000);
  return;
}

void memory_dprint(void) {
  char dstr[32];
  sprintf(dstr, "memory %d mb, free %d kb", mem_total >> 20, memory_total_free(memc) >> 10);
  putfont_ascii_in_layer(blayer, 0, 32, COL8_BLACK, COL8_WHITE, dstr);
  return;
}

unsigned int memory_alloc_4k(struct MEM_MANAGER *memc, unsigned int size) {
  size = (size + 0xfff) & 0xfffff000;
  return memory_alloc(memc, size);
}

char memory_free_4k(struct MEM_MANAGER *memc, unsigned int addr, unsigned int size) {
  size = (size + 0xfff) & 0xfffff000;
  return memory_free(memc, addr, size);
}

unsigned int memory_alloc(struct MEM_MANAGER *memc, unsigned int size) {
  for(int i = 0; i < memc -> free_count; ++ i) {
    if(size <= memc -> fb[i].size) {
      unsigned int addr = memc -> fb[i].addr;
      memc -> fb[i].size -= size;
      memc -> fb[i].addr += size;
      if(!memc -> fb[i].size) {
        -- memc -> free_count;
        for(int j = i; j < memc -> free_count; ++ j)
          memc -> fb[j] = memc -> fb[j + 1];
      }
      return addr;
    }
  }
  return 0;
}

void memory_management_init(struct MEM_MANAGER *memc) {
  memc -> free_count = 0;
  memc -> lost_size = 0;
  memc -> lost_time = 0;
  return;
}

char memory_free(struct MEM_MANAGER *memc, unsigned int addr, unsigned int size) {
  int pos = 0;
  for(;pos < memc -> free_count; ++ pos)
    if(memc -> fb[pos].addr > addr) break;
  char pr = (pos > 0 && memc -> fb[pos - 1].addr + memc -> fb[pos - 1].size == addr);
  char nt = (pos < memc -> free_count && addr + size == memc -> fb[pos].addr);
  if(!pr && !nt) {
    if(memc -> free_count == MAX_FREE_BLOCK_CNT)
      return 0;
    for(int i = memc -> free_count; i > pos; -- i)
      memc -> fb[i] = memc -> fb[i - 1];
    memc -> fb[pos].addr = addr;
    memc -> fb[pos].size = size;
    ++ memc -> free_count;
  } else if(!pr && nt) {
    memc -> fb[pos].addr  = addr;
    memc -> fb[pos].size += size;
  } else if(pr && !nt) {
    memc -> fb[pos - 1].size += size;
  } else {
    memc -> fb[pos - 1].size += size + memc -> fb[pos].size;
    -- memc -> free_count;
    for(int i = pos; i < memc -> free_count; ++ i)
      memc -> fb[i] = memc -> fb[i + 1];
  }
  return 1;
}

unsigned int memory_total_free(struct MEM_MANAGER *memc) {
  unsigned int total = 0;
  for(int i = 0; i < memc -> free_count; ++ i)
    total += memc -> fb[i].size;
  return total;
}

unsigned int memtest(unsigned int start, unsigned int end) {

 	char flag_486 = 0;
	unsigned int eflags, cr0, i;
	eflags  = io_load_eflags();
	eflags |= EFLAGS_AC_BIT;
	io_store_eflags(eflags);
	eflags  = io_load_eflags();
	if(eflags & EFLAGS_AC_BIT) flag_486 = 1;
	eflags &= ~ EFLAGS_AC_BIT;
	io_store_eflags(eflags);

	if(flag_486) {
		cr0  = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; // Disable Cache
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if(flag_486) {
		cr0  = load_cr0();
		cr0 &= ~ CR0_CACHE_DISABLE; // Disable Cache
		store_cr0(cr0);
	}

	return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end) {
	unsigned int *p, pre, i;
	char ok;
	for(i = start; i <= end; i += 0x1000) {
		p = (unsigned int *)(i + 0xffc);
		pre = *p;
		ok = asm_check_mem((unsigned int)p);
		*p = pre;
		if(!ok) break;
	}
	return i;
}

void command_mem(struct CONSOLE *con) {
  char output[256];
  sprintf(output, "memory %d mb\nfree %d kb\n", mem_total >> 20, memory_total_free(memc) >> 10);
  print_screen(con, output, strlen(output));
  return;
}
