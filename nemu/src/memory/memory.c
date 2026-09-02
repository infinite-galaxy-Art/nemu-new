#include "common.h"
#include "memory/memory.h"
#include "cpu/reg.h"
#include "device/mmio.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int map_NO = is_mmio(addr);
	if(map_NO != -1) {
		return mmio_read(addr, len, map_NO);
	}
	return cache_read(addr, len);
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	int map_NO = is_mmio(addr);
	if(map_NO != -1) {
		mmio_write(addr, len, data, map_NO);
		return;
	}
	cache_write(addr, len, data);
}

/* Segment-level address translation.  Only performed in protected mode;
 * in real mode the address is used as-is. */
uint32_t seg_translate(swaddr_t addr, uint8_t sreg) {
	if(cpu.cr0.protect_enable) {
		addr += cpu.sreg[sreg].base;
	}
	return addr;
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(seg_translate(addr, sreg), len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(seg_translate(addr, sreg), len, data);
}

