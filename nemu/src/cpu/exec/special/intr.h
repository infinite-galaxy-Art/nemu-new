#ifndef __INTR_H__
#define __INTR_H__

void raise_intr(uint8_t NO);

make_helper(int_);
make_helper(iret);
make_helper(cli);
make_helper(sti);
make_helper(pusha);
make_helper(popa);
make_helper(lidt);

make_helper(in_i_b);   /* in al, imm8  (0xe4) */
make_helper(in_i_v);   /* in eax, imm8 (0xe5) */
make_helper(out_i_b);  /* out imm8, al  (0xe6) */
make_helper(out_i_v);  /* out imm8, eax (0xe7) */
make_helper(in_dx_b);  /* in al, dx   (0xec) */
make_helper(in_dx_v);  /* in eax, dx  (0xed) */
make_helper(out_dx_b); /* out dx, al  (0xee) */
make_helper(out_dx_v); /* out dx, eax (0xef) */
make_helper(hlt);

#endif
