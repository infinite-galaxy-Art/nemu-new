#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "trap.h"

typedef int FLOAT;

/* A FLOAT is a 32-bit fixed point number: the highest bit is the sign,
 * the next 15 bits are the integer part, and the low 16 bits are the
 * fraction part (i.e. the real value is `FLOAT / 2^16`). */

static inline int F2int(FLOAT a) {
	return a >> 16;
}

static inline FLOAT int2F(int a) {
	return a << 16;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
	return a * b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
	return a / b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT sqrt(FLOAT);
FLOAT pow(FLOAT, FLOAT);

// used when calling printf/sprintf to format a FLOAT argument
#define FLOAT_ARG(f) (long long)f

void init_FLOAT_vfprintf(void);

#endif
