#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"
#include "monitor/monitor.h"

make_helper(inv) {
	/* invalid opcode */

	uint32_t temp[8];
	temp[0] = instr_fetch(eip, 4);
	temp[1] = instr_fetch(eip + 4, 4);

	uint8_t *p = (void *)temp;
	printf("invalid opcode(eip = 0x%08x): %02x %02x %02x %02x %02x %02x %02x %02x ...\n\n", 
			eip, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

	extern char logo [];
	printf("There are two cases which will trigger this unexpected exception:\n\
1. The instruction at eip = 0x%08x is not implemented.\n\
2. Something is implemented incorrectly.\n", eip);
	printf("Find this eip value(0x%08x) in the disassembling result to distinguish which case it is.\n\n", eip);
	printf("\33[1;31mIf it is the first case, see\n%s\nfor more details.\n\nIf it is the second case, remember:\n\
* The machine is always right!\n\
* Every line of untested code is always wrong!\33[0m\n\n", logo);

	assert(0);
}

make_helper(nemu_trap) {
	print_asm("nemu trap (eax = %d)", cpu.eax);

	switch(cpu.eax) {
		case 2:
		   	break;

		default:
			printf("\33[1;31mnemu: HIT %s TRAP\33[0m at eip = 0x%08x\n\n",
					(cpu.eax == 0 ? "GOOD" : "BAD"), cpu.eip);
			nemu_state = END;
	}

	return 1;
}

/* lgdt m16&32: load GDTR from the memory operand (base + 16-bit limit) */
make_helper(lgdt) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	int len = load_addr(eip + 1, &m, op_src);
	/* the memory operand holds a 6-byte descriptor: 2-byte limit, 4-byte base */
	cpu.gdtr.limit = swaddr_read(op_src->addr, 2, op_src->sreg) & 0xffff;
	cpu.gdtr.base = swaddr_read(op_src->addr + 2, 4, op_src->sreg);

	print_asm("lgdt 0x%x", op_src->addr);
	return len + 1;
}

/* mov r32, crN (0x0f 0x20) : read CR0/CR3.  The reg field of ModR/M
 * selects the control register (0 = CR0, 3 = CR3). */
make_helper(mov_cr_r) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	if(m.mod == 3) {
		if(m.reg == 0) {
			reg_l(m.R_M) = cpu.cr0.val;
		}
		else if(m.reg == 3) {
			reg_l(m.R_M) = cpu.cr3.val;
		}
	}
	print_asm("movl %%cr%d,%%%s", m.reg, regsl[m.R_M]);
	return 2;
}

/* mov crN, r32 (0x0f 0x22) : write CR0/CR3.  Writing CR3 flushes the TLB. */
make_helper(mov_r_cr) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	if(m.mod == 3) {
		if(m.reg == 0) {
			cpu.cr0.val = reg_l(m.R_M);
		}
		else if(m.reg == 3) {
			cpu.cr3.val = reg_l(m.R_M);
			tlb_flush();
		}
	}
	print_asm("movl %%%s,%%cr%d", regsl[m.R_M], m.reg);
	return 2;
}

