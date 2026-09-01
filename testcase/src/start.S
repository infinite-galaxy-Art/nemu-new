#include "trap.h"

.globl _start
_start:
# The kernel has already set up the stack (%esp = 0xc0000000) and %ebp
# for the user program before jumping here.  Do not reset them.
	movl $0, %ebp
	call main

	HIT_GOOD_TRAP
