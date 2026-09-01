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
/* 2-byte opcode form (0x0f 0x8x rel32): `eip' points at the second opcode
 * byte.  `_2byte_esc' adds 1 to our return value, and `cpu_exec' then adds
 * that length back onto cpu.eip.  The full instruction is 6 bytes (0x0f,
 * 0x8x, 4-byte disp); the byte after the second opcode byte is at eip+5.
 *
 *  - not taken: return 5, so _2byte_esc yields 6 = full length.
 *  - taken:     set cpu.eip = target, and return -1 so the net adjustment
 *               from _2byte_esc + cpu_exec is 0. */
make_helper(concat3(instr, _, SUFFIX)) {
	int32_t disp = (int32_t)instr_fetch(eip + 1, 4);
	bool taken = (JCC_COND);
	if (taken) {
		cpu.eip = eip + 5 + disp;
	}

	print_asm(JCC_NAME " 0x%x", cpu.eip);
	return (taken ? -1 : 5);
}
#endif

#include "cpu/exec/template-end.h"
