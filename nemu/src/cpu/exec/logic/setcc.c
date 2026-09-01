#include "cpu/exec/helper.h"

/* SETcc r/m8: write 0/1 to the byte operand depending on the condition.
 * These are 2-byte opcodes (0x0f 0x90..0x9f), so `eip' here points at the
 * second opcode byte and the ModR/M byte is at eip + 1. */

#define SETCC(name, cond) \
	make_helper(name) { \
		int len = decode_rm_b(eip + 1); \
		write_operand_b(op_src, (cond) ? 1 : 0); \
		print_asm(str(name) " %s", op_src->str); \
		return len + 1; \
	}

SETCC(seto,   cpu.eflags.OF)
SETCC(setno,  !cpu.eflags.OF)
SETCC(setb,   cpu.eflags.CF)
SETCC(setae,  !cpu.eflags.CF)
SETCC(sete,   cpu.eflags.ZF)
SETCC(setne,  !cpu.eflags.ZF)
SETCC(setbe,  cpu.eflags.CF || cpu.eflags.ZF)
SETCC(seta,   !cpu.eflags.CF && !cpu.eflags.ZF)
SETCC(sets,   cpu.eflags.SF)
SETCC(setns,  !cpu.eflags.SF)
SETCC(setp,   cpu.eflags.PF)
SETCC(setnp,  !cpu.eflags.PF)
SETCC(setl,   cpu.eflags.SF != cpu.eflags.OF)
SETCC(setge,  cpu.eflags.SF == cpu.eflags.OF)
SETCC(setle,  cpu.eflags.ZF || (cpu.eflags.SF != cpu.eflags.OF))
SETCC(setg,   !cpu.eflags.ZF && (cpu.eflags.SF == cpu.eflags.OF))

#undef SETCC
