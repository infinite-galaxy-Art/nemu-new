#include "irq.h"

#include <sys/syscall.h>

void add_irq_handle(int, void (*)(void));
uint32_t mm_brk(uint32_t);
int fs_ioctl(int, uint32_t, void *);

static void sys_brk(TrapFrame *tf) {
	tf->eax = mm_brk(tf->ebx);
}

static void sys_ioctl(TrapFrame *tf) {
	tf->eax = fs_ioctl(tf->ebx, tf->ecx, (void *)tf->edx);
}

static void sys_write(TrapFrame *tf) {
	int fd = tf->ebx;
	char *buf = (char *)tf->ecx;
	int len = tf->edx;

	if(fd == 1 || fd == 2) {
		/* Output `len' bytes starting from `buf' through the NEMU trap
		 * (eax = 2), which the emulator interprets as "print the string". */
		asm volatile("movl %0, %%eax; movl %1, %%ecx; movl %2, %%edx; .byte 0xd6"
				: : "i"(2), "r"(buf), "r"(len) : "eax", "ecx", "edx");
	}

	/* write() returns the number of bytes written. */
	tf->eax = len;
}

void do_syscall(TrapFrame *tf) {
	switch(tf->eax) {
		/* The `add_irq_handle' system call is artificial. We use it to
		 * let user program register its interrupt handlers. But this is
		 * very dangerous in a real operating system. Therefore such a
		 * system call never exists in GNU/Linux.
		 */
		case 0:
			cli();
			add_irq_handle(tf->ebx, (void*)tf->ecx);
			sti();
			break;

		case SYS_brk: sys_brk(tf); break;
		case SYS_ioctl: sys_ioctl(tf); break;
		case SYS_write: sys_write(tf); break;

		/* TODO: Add more system calls. */

		default: panic("Unhandled system call: id = %d, eip = 0x%08x", tf->eax, tf->eip);
	}
}

