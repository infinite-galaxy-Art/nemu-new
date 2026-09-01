#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "jmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "jmp-template.h"
#undef DATA_BYTE

/* jmp ptr16:32 (0xea): far jump, load CS (selector + descriptor cache)
 * and set EIP to the 4-byte offset.  Used to switch to protected mode. */
make_helper(ljmp) {
	uint32_t offset = instr_fetch(eip + 1, 4);
	uint16_t selector = instr_fetch(eip + 5, 2);

	uint8_t sreg = SREG_CS;
	cpu.sreg[sreg].val = selector;
	uint32_t desc_base = cpu.gdtr.base + (selector & ~0x7);
	uint32_t lo = swaddr_read(desc_base, 4, SREG_DS);
	uint32_t hi = swaddr_read(desc_base + 4, 4, SREG_DS);
	cpu.sreg[sreg].base = (lo >> 16) | (hi & 0xff000000) | ((hi & 0xff) << 16);
	cpu.sreg[sreg].limit = (lo & 0xffff) | (hi & 0x000f0000);

	cpu.eip = offset;

	print_asm("ljmp $0x%x,$0x%x", selector, offset);
	return 0;
}
