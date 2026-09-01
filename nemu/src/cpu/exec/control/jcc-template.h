#include "cpu/exec/template-start.h"

/* Conditional jump.
 *  - instr:     prefix of the helper name (e.g. jcc_je)
 *  - JCC_NAME:  string literal of the instruction (e.g. "je")
 *  - JCC_COND:  C expression over the EFLAGS state in `cpu`, deciding
 *               whether the branch is taken.                               */
#ifndef JCC_COND
#error "jcc-template.h requires a JCC_COND macro"
#endif

#if DATA_BYTE == 1
make_helper(concat3(instr, _, SUFFIX)) {
	int8_t disp = (int8_t)instr_fetch(eip + 1, 1);
	bool taken = (JCC_COND);
	if (taken) {
		cpu.eip = eip + 2 + disp;
	}

	print_asm(JCC_NAME " 0x%x", cpu.eip);
	return (taken ? 0 : 2);
}
#endif

#if DATA_BYTE == 4
/* 2-byte opcode form: `eip' points at the second opcode byte (0x0f xx).
 * `_2byte_esc' adds 1 to our return value, so we return the length after
 * the second opcode byte (4 for the disp). */
make_helper(concat3(instr, _, SUFFIX)) {
	int32_t disp = (int32_t)instr_fetch(eip + 1, 4);
	bool taken = (JCC_COND);
	if (taken) {
		cpu.eip = eip + 5 + disp;
	}

	print_asm(JCC_NAME " 0x%x", cpu.eip);
	return (taken ? 0 : 5);
}
#endif

#include "cpu/exec/template-end.h"
