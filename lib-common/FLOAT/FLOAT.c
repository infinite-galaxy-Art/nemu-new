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
	 * = (a<<16)/b, a "64 / 32" division done by a single `idivl'. */
	int32_t q, r;
	int64_t num = (int64_t)a << 16;
	asm volatile("idivl %3"
		: "=a"(q), "=d"(r)
		: "a"((uint32_t)num), "d"((uint32_t)((uint64_t)num >> 32)), "r"(b));
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

	/* value = (1.mant) * 2^exp = m * 2^(exp - 23), with m the 24-bit
	 * mantissa including the implicit leading 1.  FLOAT = value * 2^16,
	 * so the shift applied to m is (exp - 23 + 16). */
	int64_t m = 0x800000 | mant;
	int32_t shift = exp - 23 + 16;
	int64_t result;

	if (shift >= 0) {
		result = m << shift;
	} else {
		int32_t rshift = -shift;
		result = (rshift >= 24) ? 0 : (m >> rshift);
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
