#include "common.h"
#include "memory/memory.h"
#include <string.h>

extern uint8_t *hw_mem;

/* A configurable L1 cache.
 *   - block size: 64B
 *   - total size: 64KB
 *   - 8-way set associative
 *   - only a valid bit per line
 *   - random replacement
 *   - write through, no write allocate */

#define CACHE_BLOCK_SIZE  64
#define CACHE_SIZE        (64 * 1024)
#define CACHE_WAY         8
#define CACHE_NR_SET      (CACHE_SIZE / CACHE_BLOCK_SIZE / CACHE_WAY)

#define OFFSET_BITS       6          /* log2(CACHE_BLOCK_SIZE) */
#define INDEX_BITS        7          /* log2(CACHE_NR_SET) */

typedef struct {
	bool valid;
	uint32_t tag;
	uint8_t data[CACHE_BLOCK_SIZE];
} CacheLine;

static CacheLine cache[CACHE_NR_SET][CACHE_WAY];

/* statistics for observing the effect of the cache */
uint64_t cache_hit = 0;
uint64_t cache_miss = 0;
uint64_t cache_cycles = 0;		/* simulated access cost: hit +2, miss +200 */

void init_cache() {
	memset(cache, 0, sizeof(cache));
	cache_hit = cache_miss = cache_cycles = 0;
}

static inline uint32_t get_index(hwaddr_t addr) {
	return (addr >> OFFSET_BITS) & (CACHE_NR_SET - 1);
}

static inline uint32_t get_tag(hwaddr_t addr) {
	return addr >> (OFFSET_BITS + INDEX_BITS);
}

/* Find the way holding `tag' in set `index'.  Return -1 on miss. */
static int find_way(uint32_t index, uint32_t tag) {
	int i;
	for(i = 0; i < CACHE_WAY; i ++) {
		if(cache[index][i].valid && cache[index][i].tag == tag) {
			return i;
		}
	}
	return -1;
}

/* Read `len' bytes from a single cache block (the access is guaranteed not
 * to cross a block boundary).  On a miss, read the whole block from DRAM
 * into a randomly-chosen way. */
static void cache_read_block(hwaddr_t addr, size_t len, uint8_t *out) {
	uint32_t offset = addr & (CACHE_BLOCK_SIZE - 1);
	uint32_t index = get_index(addr);
	uint32_t tag = get_tag(addr);
	int way = find_way(index, tag);

	if(way < 0) {
		/* miss: fill a line from DRAM */
		static uint32_t counter = 0;
		way = (counter ++) % CACHE_WAY;
		cache[index][way].valid = true;
		cache[index][way].tag = tag;
		memcpy(cache[index][way].data, hw_mem + addr - offset, CACHE_BLOCK_SIZE);
		cache_miss ++;
		cache_cycles += 200;
	}
	else {
		cache_hit ++;
		cache_cycles += 2;
	}

	memcpy(out, cache[index][way].data + offset, len);
}

/* Write `len' bytes to a single cache block (no boundary crossing). */
static void cache_write_block(hwaddr_t addr, size_t len, const uint8_t *data) {
	uint32_t offset = addr & (CACHE_BLOCK_SIZE - 1);
	uint32_t index = get_index(addr);
	uint32_t tag = get_tag(addr);
	int way = find_way(index, tag);

	/* write through: always update DRAM */
	memcpy(hw_mem + addr, data, len);

	if(way >= 0) {
		/* write hit: keep the cache line coherent */
		memcpy(cache[index][way].data + offset, data, len);
		cache_hit ++;
		cache_cycles += 2;
	}
	else {
		/* write miss: not write allocate */
		cache_miss ++;
		cache_cycles += 200;
	}
}

uint32_t cache_read(hwaddr_t addr, size_t len) {
	uint32_t offset = addr & (CACHE_BLOCK_SIZE - 1);
	uint32_t result = 0;

	if(offset + len > CACHE_BLOCK_SIZE) {
		/* the access crosses a block boundary: split it into two parts */
		size_t len1 = CACHE_BLOCK_SIZE - offset;
		size_t len2 = len - len1;
		cache_read_block(addr, len1, (uint8_t *)&result);
		cache_read_block(addr + len1, len2, (uint8_t *)&result + len1);
	}
	else {
		cache_read_block(addr, len, (uint8_t *)&result);
	}

	return result;
}

void cache_write(hwaddr_t addr, size_t len, uint32_t data) {
	uint32_t offset = addr & (CACHE_BLOCK_SIZE - 1);

	if(offset + len > CACHE_BLOCK_SIZE) {
		/* the access crosses a block boundary: split it into two parts */
		size_t len1 = CACHE_BLOCK_SIZE - offset;
		size_t len2 = len - len1;
		cache_write_block(addr, len1, (uint8_t *)&data);
		cache_write_block(addr + len1, len2, (uint8_t *)&data + len1);
	}
	else {
		cache_write_block(addr, len, (uint8_t *)&data);
	}
}
