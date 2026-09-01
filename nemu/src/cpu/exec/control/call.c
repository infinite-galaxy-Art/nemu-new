#include "cpu/exec/helper.h"

make_helper(call_si_l) {
	/* The signed displacement is relative to the address of the *next*
	 * instruction (eip + 5). */
	int32_t disp = (int32_t)instr_fetch(eip + 1, 4);
	swaddr_t next = eip + 5;

	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, next, SREG_SS);
	cpu.eip = next + disp;

	print_asm("call 0x%x", cpu.eip);
	return 0;
}

make_helper(call_rm_l) {
	int len = decode_rm_l(eip + 1);
	swaddr_t next = eip + 1 + len;

	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, next, SREG_SS);
	cpu.eip = op_src->val;

	print_asm("call *%s", op_src->str);
	return 0;
}
