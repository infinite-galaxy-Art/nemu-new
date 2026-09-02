#ifndef __IRQ_H__
#define __IRQ_H__

#include "common.h"

/* TODO: The decleration order of the members in the `TrapFrame'
 * structure below is wrong. Please re-orgainize it for the C
 * code to use the trap frame correctly.
 */

typedef struct TrapFrame {
	/* The order must match the trap frame built in `do_irq.S':
	 * `pushal' pushes eax, ecx, edx, ebx, old esp, ebp, esi, edi in
	 * this order, so edi ends up on top of the stack.  The vec/irq
	 * entries then push the error code and the irq number on top. */
	uint32_t edi, esi, ebp, old_esp, ebx, edx, ecx, eax;
	int32_t irq;			/* pushed by the vec/irq entries */
	uint32_t error_code;	/* pushed by the vec/irq entries */
	uint32_t eip, cs, eflags;	/* pushed by hardware before the handler */
} TrapFrame;

#endif
