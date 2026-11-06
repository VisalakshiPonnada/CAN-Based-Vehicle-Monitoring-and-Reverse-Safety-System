/* delay.h - Timer1 based delays (1 us tick). Independent of PCLK
   guesswork because it reads PCLK from a #define you can verify.   */
#ifndef __DELAY_H__
#define __DELAY_H__
#include "types.h"

void delay_init(void);      /* call once, first line of main() */
void delay_us(u32 us);
void delay_ms(u32 ms);
void delay_s(u32 s);

#endif
