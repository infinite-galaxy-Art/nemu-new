#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {
	cpu.esp -= DATA_BYTE;
	swaddr_write(cpu.esp, DATA_BYTE, op_src->val);

	print_asm_template1();
}

make_instr_helper(r)
make_instr_helper(rm)
make_instr_helper(i)

#include "cpu/exec/template-end.h"
