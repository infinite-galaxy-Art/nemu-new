#include "cpu/exec/helper.h"

#define DATA_BYTE 2
#include "push-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "push-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(push_r)
make_helper_v(push_rm)
make_helper_v(push_i)

/* push imm8 (sign-extended to operand size) */
make_helper(push_si_b) {
	int8_t imm = (int8_t)instr_fetch(eip + 1, 1);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, (int32_t)imm, SREG_SS);

	print_asm("push $0x%x", (int32_t)imm);
	return 2;
}
