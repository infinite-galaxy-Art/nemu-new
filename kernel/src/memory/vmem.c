#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)

/* Use the function to get the start address of user page directory. */
PDE* get_updir();

void create_video_mapping() {
	/* Create an identical mapping from virtual memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) for the user program.
	 * We cannot use mm_malloc() because it only allocates pages above
	 * 16MB, while video memory lives below 16MB. */
	static PTE vptable[NR_PTE] align_to_page;

	PDE *updir = get_updir();
	PTE *ptable = (PTE *)va_to_pa(vptable);

	/* 0xa0000 lies in the first 4MB, so it uses page directory entry 0. */
	updir[VMEM_ADDR / PT_SIZE].val = make_pde(ptable);

	/* Fill the PTEs that cover [0xa0000, 0xa0000 + SCR_SIZE). */
	int i;
	int nr_page = (SCR_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
	int pte_idx = (VMEM_ADDR >> 12) & (NR_PTE - 1);
	for(i = 0; i < nr_page; i ++) {
		ptable[pte_idx + i].val = make_pte(VMEM_ADDR + i * PAGE_SIZE);
	}
}

void video_mapping_write_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		buf[i] = i;
	}
}

void video_mapping_read_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		assert(buf[i] == i);
	}
}

void video_mapping_clear() {
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}

