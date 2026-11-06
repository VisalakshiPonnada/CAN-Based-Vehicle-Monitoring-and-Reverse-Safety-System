#include <LPC21xx.h>
#include "types.h"
#include "delay.h"

/* PCLK in MHz. Verified by the working ultrasonic sensor test
   (echo timing only comes out correct if this value is right). */
#define PCLK_MHZ 60

void delay_init(void)
{
    T1TCR = 0x02;               /* reset */
    T1PR  = PCLK_MHZ - 1;       /* TC increments every 1 us */
    T1TCR = 0x00;                /* stopped */
}

void delay_us(u32 us)
{
    T1TCR = 0x02;
    T1TCR = 0x01;
    while (T1TC < us);
    T1TCR = 0x00;
}

void delay_ms(u32 ms)
{
    while (ms--) delay_us(1000);
}

void delay_s(u32 s)
{
    while (s--) delay_ms(1000);
}
