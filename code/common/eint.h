/* eint.h - External interrupts for SW1 (left), SW2 (right), MODE switch.
   Used by the Main Node only.                                          */
#ifndef __EINT_H__
#define __EINT_H__
#include "types.h"

/* EINT0 -> P0.1  (SW1, left indicator)
   EINT1 -> P0.3  (SW2, right indicator)
   EINT2 -> P0.7  (MODE, forward/reverse)
   P0.14/P0.15/P0.16 cannot be used: the LCD occupies them.             */
#define VIC_EINT0  14
#define VIC_EINT1  15
#define VIC_EINT2  16

extern volatile u8 sw1Flag, sw2Flag, modeFlag;
extern volatile u8 reverseMode;         /* 0 = forward, 1 = reverse */

void InitEINT(void);
void EINT_ReEnable(u8 which);           /* 0 = SW1, 1 = SW2, 2 = MODE */

#endif
