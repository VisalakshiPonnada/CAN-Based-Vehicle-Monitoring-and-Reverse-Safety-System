/* =====================================================================
   REVERSE ALERT NODE
   - Continuously reads the HC-SR04 ultrasonic sensor
   - If distance <= THRESHOLD_CM, sends logic 1 (obstacle) to the
     Main Node over CAN (CAN_ID_REVERSE); otherwise sends logic 0
   - Sends periodically REGARDLESS of whether the Main Node is there

   DISCONNECTION SAFETY:
   - CAN1_Tx() (inside CAN1_SendByte) times itself out (see can.c).
     If the Main Node is unplugged, sending simply fails and this
     node carries straight on to the next ultrasonic reading - it
     never waits for an acknowledgement that will never come.
   ===================================================================== */
#include <LPC21xx.h>
#include "types.h"
#include "delay.h"
#include "ultrasonic.h"
#include "can.h"

#define THRESHOLD_CM   30      /* obstacle alert distance; tune as needed */

int main(void)
{
    u32 dist;
    u8  obstacle;

    delay_init();
    InitUltrasonic();
    Init_CAN1();

    while (1)
    {
        dist = Ultrasonic_GetDistanceCm();

        /* no echo = nothing detected in range = treat as clear */
        obstacle = (dist != US_NO_ECHO && dist <= THRESHOLD_CM) ? 1 : 0;

        CAN1_SendByte(CAN_ID_REVERSE, obstacle);   /* failure is ignored on purpose */

        delay_ms(100);        /* >=60ms required between HC-SR04 triggers */
    }
}
