#include "FLOAT.h"
#include <stdint.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	/* (a/2^16) * (b/2^16) = (a*b)/2^32.  A FLOAT result is (a*b)/2^16,
	 * i.e. the 64-bit product shifted right by 16.  A single `imull'
	 * computes the full 32x32 -> 64 product in edx:eax without libgcc. */
	int32_t hi, lo;
	asm volatile("imull %3" : "=d"(hi), "=a"(lo) : "a"(a), "r"(b));
	return (FLOAT)(((uint32_t)hi << 16) | ((uint32_t)lo >> 16));
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* (a/2^16) / (b/2^16) = a/b.  The FLOAT for that real is (a/b)*2^16
	 * = (a<<16)/b, a "64 / 32" division done by a single `idivl'.
	 * The 64-bit dividend `(int64_t)a << 16' is split into its high and
	 * low 32-bit halves using only 32-bit shifts, avoiding `shld'. */
	int32_t q, r;
	uint32_t lo = (uint32_t)a << 16;
	int32_t hi = a >> 16;
	asm volatile("idivl %4"
		: "=a"(q), "=d"(r)
		: "a"(lo), "d"(hi), "r"(b));
	return q;
}

FLOAT f2F(float a) {
	/* Decode the IEEE-754 bit pattern of `a' (already on the stack)
	 * manually, so no x87 instruction is generated. */
	union {
		float f;
		uint32_t u;
	} u;
	u.f = a;

	uint32_t bits = u.u;
	uint32_t sign = bits >> 31;
	int32_t exp = ((bits >> 23) & 0xff) - 127;
	uint32_t mant = bits & 0x7fffff;

	if (exp == -127 && mant == 0) {
		return 0;
	}

	/* value = (1.mant) * 2^exp = sig * 2^(exp - 23), where sig is the
	 * 24-bit significand with the implicit leading 1.  A FLOAT is
	 * value * 2^16 = sig * 2^(exp - 7).  Everything fits in 32 bits. */
	int32_t sig = (int32_t)(0x800000 | mant);
	int32_t shift = exp - 7;
	int32_t result = 0;

	if (shift >= 0) {
		result = sig << shift;
	} else if (shift > -24) {
		result = sig >> (-shift);
	}

	return (FLOAT)(sign ? -result : result);
}

FLOAT Fabs(FLOAT a) {
	return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}
