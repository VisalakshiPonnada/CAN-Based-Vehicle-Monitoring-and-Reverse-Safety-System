#include <LPC21xx.h>
#include "types.h"
#include "eint.h"

volatile u8 sw1Flag = 0, sw2Flag = 0, modeFlag = 0;
volatile u8 reverseMode = 0;

void EINT0_ISR(void) __irq
{
    sw1Flag = 1;
    VICIntEnClr = (1 << VIC_EINT0);
    EXTINT      = (1 << 0);
    VICVectAddr = 0;
}

void EINT1_ISR(void) __irq
{
    sw2Flag = 1;
    VICIntEnClr = (1 << VIC_EINT1);
    EXTINT      = (1 << 1);
    VICVectAddr = 0;
}

void EINT2_ISR(void) __irq
{
    reverseMode ^= 1;
    modeFlag = 1;
    VICIntEnClr = (1 << VIC_EINT2);
    EXTINT      = (1 << 2);
    VICVectAddr = 0;
}

void InitEINT(void)
{
    PINSEL0 &= ~((3u << 2) | (3u << 6) | (3u << 14));
    PINSEL0 |=  ((3u << 2) | (3u << 6) | (3u << 14));

    EXTMODE  |=  0x07;      /* edge sensitive          */
    EXTPOLAR &= ~0x07;      /* falling edge (active low switch) */
    EXTINT    =  0x07;      /* clear stale flags        */

    VICIntSelect &= ~((1 << VIC_EINT0) | (1 << VIC_EINT1) | (1 << VIC_EINT2));

    VICVectCntl0 = 0x20 | VIC_EINT0;
    VICVectAddr0 = (u32)EINT0_ISR;
    VICVectCntl1 = 0x20 | VIC_EINT1;
    VICVectAddr1 = (u32)EINT1_ISR;
    VICVectCntl2 = 0x20 | VIC_EINT2;
    VICVectAddr2 = (u32)EINT2_ISR;

    VICIntEnable = (1 << VIC_EINT0) | (1 << VIC_EINT1) | (1 << VIC_EINT2);
}

void EINT_ReEnable(u8 which)
{
    if (which == 0) { EXTINT = (1 << 0); VICIntEnable = (1 << VIC_EINT0); }
    if (which == 1) { EXTINT = (1 << 1); VICIntEnable = (1 << VIC_EINT1); }
    if (which == 2) { EXTINT = (1 << 2); VICIntEnable = (1 << VIC_EINT2); }
}
