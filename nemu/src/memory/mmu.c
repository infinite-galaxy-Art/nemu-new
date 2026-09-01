#include "common.h"
#include "memory/memory.h"
#include "cpu/reg.h"
#include "x86-inc/mmu.h"
#include <string.h>

/* TLB: 64 entries, fully associative, valid bit only, random replacement */

#define NR_TLB 64

typedef struct {
	bool valid;
	uint32_t vpn;		/* virtual page number (high 20 bits of the linear address) */
	uint32_t pfn;		/* physical page frame number */
} TLBEntry;

static TLBEntry tlb[NR_TLB];
static uint32_t tlb_counter = 0;

void init_tlb() {
	memset(tlb, 0, sizeof(tlb));
	tlb_counter = 0;
}

/* invalidate all TLB entries (called when CR3 changes) */
void tlb_flush() {
	int i;
	for(i = 0; i < NR_TLB; i ++) {
		tlb[i].valid = false;
	}
}

static int tlb_lookup(uint32_t vpn) {
	int i;
	for(i = 0; i < NR_TLB; i ++) {
		if(tlb[i].valid && tlb[i].vpn == vpn) {
			return i;
		}
	}
	return -1;
}

/* Page-level address translation.  Returns the physical address, or
 * panics on an invalid page table entry (present bit == 0). */
uint32_t page_translate(lnaddr_t addr) {
	uint32_t vpn = addr >> 12;
	uint32_t offset = addr & 0xfff;
	uint32_t pfn;
	int hit = tlb_lookup(vpn);

	if(hit >= 0) {
		pfn = tlb[hit].pfn;
	}
	else {
		uint32_t pdir_idx = addr >> 22;
		uint32_t ptab_idx = (addr >> 12) & 0x3ff;
		PDE pde;
		PTE pte;

		/* page walk: read the PDE and PTE through the cache (physical access) */
		uint32_t pde_addr = (cpu.cr3.page_directory_base << 12) + pdir_idx * 4;
		pde.val = hwaddr_read(pde_addr, 4);
		Assert(pde.present,
				"page directory entry (vaddr 0x%08x, cr3=0x%08x, pde@0x%08x = 0x%08x) is not present",
				addr, cpu.cr3.val, pde_addr, pde.val);

		pte.val = hwaddr_read((pde.page_frame << 12) + ptab_idx * 4, 4);
		Assert(pte.present, "page table entry (vaddr 0x%08x) is not present", addr);

		pfn = pte.page_frame;

		/* fill TLB */
		int way = (tlb_counter ++) % NR_TLB;
		tlb[way].valid = true;
		tlb[way].vpn = vpn;
		tlb[way].pfn = pfn;
	}

	return (pfn << 12) | offset;
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	if(cpu.cr0.paging) {
		uint32_t offset = addr & 0xfff;
		if(offset + len > 0x1000) {
			/* data crosses a page boundary: not handled (KISS) */
			Assert(0, "cross-page access at 0x%08x is not supported", addr);
		}
		return hwaddr_read(page_translate(addr), len);
	}
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	if(cpu.cr0.paging) {
		uint32_t offset = addr & 0xfff;
		if(offset + len > 0x1000) {
			Assert(0, "cross-page access at 0x%08x is not supported", addr);
		}
		hwaddr_write(page_translate(addr), len, data);
		return;
	}
	hwaddr_write(addr, len, data);
}
