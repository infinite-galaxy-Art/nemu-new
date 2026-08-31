#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* Take a free watchpoint out of the free_ list. */
WP* new_wp(void) {
	if(free_ == NULL) {
		panic("no free watchpoint available");
	}

	WP *wp = free_;
	free_ = free_->next;

	/* Insert into the head of the used list. */
	wp->next = head;
	head = wp;

	return wp;
}

/* Return a watchpoint to the free_ list. */
void free_wp(WP *wp) {
	/* Remove from the used list. */
	WP *p = head;
	WP *prev = NULL;
	while(p != NULL) {
		if(p == wp) {
			if(prev == NULL) head = p->next;
			else prev->next = p->next;
			break;
		}
		prev = p;
		p = p->next;
	}

	/* Insert into the free_ list. */
	wp->next = free_;
	free_ = wp;
}

/* Delete the watchpoint whose NO equals `no'. */
void delete_wp(int no) {
	WP *p = head;
	while(p != NULL) {
		if(p->NO == no) {
			free_wp(p);
			printf("Watchpoint %d deleted\n", no);
			return;
		}
		p = p->next;
	}
	printf("No watchpoint number %d\n", no);
}

/* Print the information of all watchpoints in use. */
void print_wp(void) {
	if(head == NULL) {
		printf("No watchpoints\n");
		return;
	}

	WP *p = head;
	while(p != NULL) {
		printf("Watchpoint %d: %s = 0x%08x\n", p->NO, p->expr, p->old_val);
		p = p->next;
	}
}

/* Evaluate all watchpoints.  If any value changes, return true (the caller
 * should stop the program).  Otherwise update the old value and return false. */
bool check_watchpoints(void) {
	WP *p = head;
	bool triggered = false;

	while(p != NULL) {
		bool success;
		uint32_t new_val = expr(p->expr, &success);
		if(!success) {
			panic("watchpoint expression '%s' is invalid", p->expr);
		}

		if(new_val != p->old_val) {
			printf("\nHint watchpoint %d at address 0x%08x\n", p->NO, cpu.eip);
			p->old_val = new_val;
			triggered = true;
		}
		p = p->next;
	}

	return triggered;
}
