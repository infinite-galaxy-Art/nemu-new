#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "common.h"

enum { STOP, RUNNING, END };
extern int nemu_state;

bool get_symbol_addr(const char *name, swaddr_t *addr);
const char *get_func_name(swaddr_t addr);

#endif
