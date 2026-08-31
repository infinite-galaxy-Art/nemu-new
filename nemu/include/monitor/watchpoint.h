#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	char expr[128];		/* the expression string to watch */
	uint32_t old_val;	/* the value of the expression at the last check */

} WP;

WP* new_wp(void);
void free_wp(WP *wp);
void delete_wp(int no);
void print_wp(void);
bool check_watchpoints(void);

#endif
