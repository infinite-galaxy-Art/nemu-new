#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
	char *arg = strtok(NULL, " ");
	int n = 1;
	if(arg != NULL) {
		n = atoi(arg);
		if(n <= 0) n = 1;
	}
	cpu_exec(n);
	return 0;
}

static int cmd_info(char *args) {
	char *arg = strtok(NULL, " ");
	if(arg == NULL) {
		printf("info: missing subcommand\n");
		return 0;
	}

	if(strcmp(arg, "r") == 0) {
		printf("eax\t\t0x%08x\t%d\n", cpu.eax, cpu.eax);
		printf("ecx\t\t0x%08x\t%d\n", cpu.ecx, cpu.ecx);
		printf("edx\t\t0x%08x\t%d\n", cpu.edx, cpu.edx);
		printf("ebx\t\t0x%08x\t%d\n", cpu.ebx, cpu.ebx);
		printf("esp\t\t0x%08x\t%d\n", cpu.esp, cpu.esp);
		printf("ebp\t\t0x%08x\t%d\n", cpu.ebp, cpu.ebp);
		printf("esi\t\t0x%08x\t%d\n", cpu.esi, cpu.esi);
		printf("edi\t\t0x%08x\t%d\n", cpu.edi, cpu.edi);
		printf("eip\t\t0x%08x\n", cpu.eip);
		printf("eflags\t\t0x%08x\n", cpu.eflags.val);
	}
	else if(strcmp(arg, "w") == 0) {
		print_wp();
	}
	else if(strcmp(arg, "c") == 0) {
		extern uint64_t cache_hit, cache_miss, cache_cycles;
		printf("cache: hit=%llu miss=%llu cycles=%llu\n",
				(unsigned long long)cache_hit,
				(unsigned long long)cache_miss,
				(unsigned long long)cache_cycles);
	}
	else {
		printf("Unknown info subcommand '%s'\n", arg);
	}
	return 0;
}

static int cmd_x(char *args) {
	if(args == NULL) {
		printf("usage: x N EXPR\n");
		return 0;
	}

	char *p = args;
	while(*p == ' ') p ++;

	char *end;
	long n = strtol(p, &end, 10);
	if(end == p) {
		printf("usage: x N EXPR\n");
		return 0;
	}

	p = end;
	while(*p == ' ') p ++;
	if(*p == '\0') {
		printf("usage: x N EXPR\n");
		return 0;
	}

	bool success;
	uint32_t addr = expr(p, &success);
	if(!success) {
		printf("invalid expression\n");
		return 0;
	}

	int i;
	for(i = 0; i < n; i ++) {
		printf("0x%08x:\t0x%08x\n", addr + i * 4, swaddr_read(addr + i * 4, 4, SREG_DS));
	}
	return 0;
}

static int cmd_p(char *args) {
	if(args == NULL) {
		printf("usage: p EXPR\n");
		return 0;
	}

	char *p = args;
	while(*p == ' ') p ++;
	if(*p == '\0') {
		printf("usage: p EXPR\n");
		return 0;
	}

	bool success;
	uint32_t val = expr(p, &success);
	if(!success) {
		printf("invalid expression\n");
		return 0;
	}
	printf("0x%08x\t%d\n", val, val);
	return 0;
}

static int cmd_w(char *args) {
	if(args == NULL) {
		printf("usage: w EXPR\n");
		return 0;
	}

	char *p = args;
	while(*p == ' ') p ++;
	if(*p == '\0') {
		printf("usage: w EXPR\n");
		return 0;
	}

	WP *wp = new_wp();
	strncpy(wp->expr, p, 127);
	wp->expr[127] = '\0';

	bool success;
	wp->old_val = expr(wp->expr, &success);
	if(!success) {
		free_wp(wp);
		printf("invalid expression\n");
		return 0;
	}

	printf("Watchpoint %d: %s = 0x%08x\n", wp->NO, wp->expr, wp->old_val);
	return 0;
}

static int cmd_d(char *args) {
	char *arg = strtok(NULL, " ");
	if(arg == NULL) {
		printf("usage: d N\n");
		return 0;
	}
	delete_wp(atoi(arg));
	return 0;
}

static int cmd_help(char *args);

static int cmd_bt(char *args) {
	swaddr_t ebp = cpu.ebp;
	swaddr_t pc = cpu.eip;
	int depth = 0;

	/* NEMU has 128MB of physical memory; a frame pointer outside this
	 * range marks the end of a valid frame chain. */
	const swaddr_t MEM_END = 128u << 20;

	/* Walk the stack frame chain via %ebp, printing the address, function
	 * name and the first 4 arguments of every frame. */
	while(depth < 32) {
		const char *name = get_func_name(pc);
		printf("#%d  %s (0x%08x)", depth, name ? name : "???", pc);

		if(ebp >= 8 && ebp + 20 < MEM_END) {
			printf("  args: 0x%x 0x%x 0x%x 0x%x",
					swaddr_read(ebp + 8, 4, SREG_SS),
					swaddr_read(ebp + 12, 4, SREG_SS),
					swaddr_read(ebp + 16, 4, SREG_SS),
					swaddr_read(ebp + 20, 4, SREG_SS));
		}
		printf("\n");

		if(ebp == 0 || ebp + 4 >= MEM_END) break;

		pc = swaddr_read(ebp + 4, 4, SREG_SS);	/* return address (in the caller) */
		ebp = swaddr_read(ebp, 4, SREG_SS);		/* saved %ebp of the previous frame */
		depth ++;
	}
	return 0;
}

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Execute N instructions and stop (default: 1)", cmd_si },
	{ "info", "Print the state of the program: info r / info w", cmd_info },
	{ "x", "Examine N words (4 bytes) of memory from EXPR: x N EXPR", cmd_x },
	{ "p", "Evaluate the expression EXPR: p EXPR", cmd_p },
	{ "w", "Set a watchpoint on EXPR: w EXPR", cmd_w },
	{ "d", "Delete watchpoint N: d N", cmd_d },
	{ "bt", "Print the call stack (backtrace)", cmd_bt },

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
