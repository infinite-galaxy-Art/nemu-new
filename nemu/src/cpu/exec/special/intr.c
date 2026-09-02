#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"
#include <setjmp.h>

extern jmp_buf jbuf;

/* Simulate the IA-32 interrupt/exception processing: push eflags, cs and eip
 * onto the stack (in that order), fetch the gate descriptor from the IDT and
 * jump to the handler.  The handler entry is reached by longjmp-ing back into
 * cpu_exec(), so that execution continues with the freshly-set cpu.eip. */
void raise_intr(uint8_t NO) {
	/* Hardware pushes eflags, cs, eip in order (eip ends up on top). */
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.eflags.val, SREG_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.sreg[SREG_CS].val, SREG_SS);
	cpu.esp -= 4;
	swaddr_write(cpu.esp, 4, cpu.eip, SREG_SS);

	/* Fetch the gate descriptor from the IDT (the IDT base is a linear address,
	 * so it must go through page translation). */
	uint32_t lo = lnaddr_read(cpu.idtr.base + NO * 8, 4);
	uint32_t hi = lnaddr_read(cpu.idtr.base + NO * 8 + 4, 4);
	uint32_t offset = (lo & 0xffff) | (hi & 0xffff0000);

	/* Clear IF to disable further interrupts. */
	cpu.eflags.IF = 0;

	cpu.eip = offset;

	longjmp(jbuf, 1);
}

/* int imm8 (0xcd): the saved eip must point to the next instruction. */
make_helper(int_) {
	uint8_t NO = instr_fetch(eip + 1, 1);
	cpu.eip += 2;
	print_asm("int $0x%x", NO);
	raise_intr(NO);
	return 0;
}

/* iret (0xcf): pop eip, cs, eflags.  No privilege-level switch is needed here. */
make_helper(iret) {
	cpu.eip = swaddr_read(cpu.esp, 4, SREG_SS);
	cpu.sreg[SREG_CS].val = swaddr_read(cpu.esp + 4, 4, SREG_SS);
	cpu.eflags.val = swaddr_read(cpu.esp + 8, 4, SREG_SS);
	cpu.esp += 12;
	print_asm("iret");
	return 0;
}

/* cli (0xfa) */
make_helper(cli) {
	cpu.eflags.IF = 0;
	print_asm("cli");
	return 1;
}

/* sti (0xfb) */
make_helper(sti) {
	cpu.eflags.IF = 1;
	print_asm("sti");
	return 1;
}

/* pusha (0x60): push eax, ecx, edx, ebx, original esp, ebp, esi, edi.
 * The first register pushed (eax) ends up at the highest address, and the
 * last one (edi) ends up on top of the stack. */
make_helper(pusha) {
	uint32_t tmp = cpu.esp;
	swaddr_write(tmp - 4,  4, cpu.eax, SREG_SS);
	swaddr_write(tmp - 8,  4, cpu.ecx, SREG_SS);
	swaddr_write(tmp - 12, 4, cpu.edx, SREG_SS);
	swaddr_write(tmp - 16, 4, cpu.ebx, SREG_SS);
	swaddr_write(tmp - 20, 4, tmp,    SREG_SS);
	swaddr_write(tmp - 24, 4, cpu.ebp, SREG_SS);
	swaddr_write(tmp - 28, 4, cpu.esi, SREG_SS);
	swaddr_write(tmp - 32, 4, cpu.edi, SREG_SS);
	cpu.esp = tmp - 32;
	print_asm("pusha");
	return 1;
}

/* popa (0x61): pop edi, esi, ebp, (skip esp), ebx, edx, ecx, eax. */
make_helper(popa) {
	cpu.edi = swaddr_read(cpu.esp, 4, SREG_SS);
	cpu.esi = swaddr_read(cpu.esp + 4, 4, SREG_SS);
	cpu.ebp = swaddr_read(cpu.esp + 8, 4, SREG_SS);
	/* old esp is discarded */
	cpu.ebx = swaddr_read(cpu.esp + 16, 4, SREG_SS);
	cpu.edx = swaddr_read(cpu.esp + 20, 4, SREG_SS);
	cpu.ecx = swaddr_read(cpu.esp + 24, 4, SREG_SS);
	cpu.eax = swaddr_read(cpu.esp + 28, 4, SREG_SS);
	cpu.esp += 32;
	print_asm("popa");
	return 1;
}

/* lidt m16&32 (0x0f 0x01 /3): load IDTR from a 6-byte memory operand. */
make_helper(lidt) {
	ModR_M m;
	m.val = instr_fetch(eip + 1, 1);
	int len = load_addr(eip + 1, &m, op_src);
	cpu.idtr.limit = swaddr_read(op_src->addr, 2, op_src->sreg) & 0xffff;
	cpu.idtr.base = swaddr_read(op_src->addr + 2, 4, op_src->sreg);
	print_asm("lidt 0x%x", op_src->addr);
	return len + 1;
}

/* in/out: port-mapped I/O, dispatched to pio_read()/pio_write(). */
#include "device/port-io.h"

make_helper(in_i_b) {
	uint8_t port = instr_fetch(eip + 1, 1);
	reg_b(R_AL) = pio_read(port, 1);
	print_asm("in $0x%x,%%al", port);
	return 2;
}

make_helper(in_i_v) {
	uint8_t port = instr_fetch(eip + 1, 1);
	reg_l(R_EAX) = pio_read(port, 4);
	print_asm("in $0x%x,%%eax", port);
	return 2;
}

make_helper(out_i_b) {
	uint8_t port = instr_fetch(eip + 1, 1);
	pio_write(port, 1, reg_b(R_AL));
	print_asm("out %%al,$0x%x", port);
	return 2;
}

make_helper(out_i_v) {
	uint8_t port = instr_fetch(eip + 1, 1);
	pio_write(port, 4, reg_l(R_EAX));
	print_asm("out %%eax,$0x%x", port);
	return 2;
}

make_helper(in_dx_b) {
	uint16_t port = reg_w(R_DX);
	reg_b(R_AL) = pio_read(port, 1);
	print_asm("in (%%dx),%%al");
	return 1;
}

make_helper(in_dx_v) {
	uint16_t port = reg_w(R_DX);
	reg_l(R_EAX) = pio_read(port, 4);
	print_asm("in (%%dx),%%eax");
	return 1;
}

make_helper(out_dx_b) {
	uint16_t port = reg_w(R_DX);
	pio_write(port, 1, reg_b(R_AL));
	print_asm("out %%al,(%%dx)");
	return 1;
}

make_helper(out_dx_v) {
	uint16_t port = reg_w(R_DX);
	pio_write(port, 4, reg_l(R_EAX));
	print_asm("out %%eax,(%%dx)");
	return 1;
}

/* hlt (0xf4): stop until a hardware interrupt arrives. */
make_helper(hlt) {
	print_asm("hlt");
	while(!(cpu.INTR && cpu.eflags.IF));
	return 1;
}
