#include "cpu/exec/helper.h"

make_helper(ret) {
	cpu.eip = swaddr_read(cpu.esp, 4, SREG_SS);
	cpu.esp += 4;

	print_asm("ret");
	return 0;
}

make_helper(ret_iw) {
	uint16_t imm = instr_fetch(eip + 1, 2);
	cpu.eip = swaddr_read(cpu.esp, 4, SREG_SS);
	cpu.esp += 4 + imm;

	print_asm("ret $0x%x", imm);
	return 0;
}
