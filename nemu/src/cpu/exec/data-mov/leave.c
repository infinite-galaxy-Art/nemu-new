#include "cpu/exec/helper.h"

make_helper(leave) {
	cpu.esp = cpu.ebp;
	cpu.ebp = swaddr_read(cpu.esp, 4, SREG_SS);
	cpu.esp += 4;

	print_asm("leave");
	return 1;
}
