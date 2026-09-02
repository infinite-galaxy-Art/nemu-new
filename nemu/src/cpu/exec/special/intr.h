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

#endif
