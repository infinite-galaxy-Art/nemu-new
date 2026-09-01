#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

make_helper(nop) {
	print_asm("nop");
	return 1;
}

make_helper(std) {
	cpu.eflags.DF = 1;
	print_asm("std");
	return 1;
}

make_helper(cld) {
	cpu.eflags.DF = 0;
	print_asm("cld");
	return 1;
}

make_helper(int3) {
	void do_int3();
	do_int3();
	print_asm("int3");

	return 1;
}

make_helper(lea) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	int len = load_addr(eip + 1, &m, op_src);
	reg_l(m.reg) = op_src->addr;

	print_asm("leal %s,%%%s", op_src->str, regsl[m.reg]);
	return 1 + len;
}

/* mov r/m16, Sreg (0x8e): load a segment register and its descriptor cache */
make_helper(mov_rm2sreg) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	uint8_t sreg = m.reg;		/* the reg field selects the segment register */

	if(m.mod == 3) {
		cpu.sreg[sreg].val = reg_w(m.R_M);
	}
	else {
		load_addr(eip + 1, &m, op_src);
		cpu.sreg[sreg].val = swaddr_read(op_src->addr, 2, op_src->sreg);
	}

	/* load the descriptor cache (base and limit) from the GDT */
	uint32_t desc_base = cpu.gdtr.base + (cpu.sreg[sreg].val & ~0x7);
	uint32_t lo = swaddr_read(desc_base, 4, SREG_DS);
	uint32_t hi = swaddr_read(desc_base + 4, 4, SREG_DS);
	cpu.sreg[sreg].base = (lo >> 16) | (hi & 0xff000000) | ((hi & 0xff) << 16);
	cpu.sreg[sreg].limit = (lo & 0xffff) | (hi & 0x000f0000);

	print_asm("mov %s,%%%s", op_src->str, "sreg");
	return 2;
}
