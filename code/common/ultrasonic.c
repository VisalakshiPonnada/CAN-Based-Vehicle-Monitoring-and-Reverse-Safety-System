#include <LPC21xx.h>
#include "types.h"
#include "delay.h"
#include "ultrasonic.h"

void InitUltrasonic(void)
{
    PINSEL1 &= ~((3u << ((TRIG_PIN - 16) * 2)) | (3u << ((ECHO_PIN - 16) * 2)));
    IODIR0 |=  TRIG_MASK;
    IODIR0 &= ~ECHO_MASK;
    IOCLR0  =  TRIG_MASK;

    T0TCR = 0x02;
    T0PR  = PCLK_MHZ - 1;
    T0MCR = 0;
    T0TCR = 0x00;
}

u32 Ultrasonic_GetDistanceCm(void)
{
    u32 start, width;

    T0TCR = 0x02;
    T0TCR = 0x01;

    IOSET0 = TRIG_MASK;
    delay_us(10);
    IOCLR0 = TRIG_MASK;

    while (!(IOPIN0 & ECHO_MASK))
    {
        if (T0TC > US_WAIT_START_US) { T0TCR = 0; return US_NO_ECHO; }
    }
    start = T0TC;

    while (IOPIN0 & ECHO_MASK)
    {
        if ((T0TC - start) > US_MAX_ECHO_US) { T0TCR = 0; return US_NO_ECHO; }
    }
    width = T0TC - start;
    T0TCR = 0;

    return width / 58;
}
