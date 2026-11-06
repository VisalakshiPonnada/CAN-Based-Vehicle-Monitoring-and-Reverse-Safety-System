/* =====================================================================
   INDICATOR NODE
   - Waits for commands from the Main Node (CAN_ID_INDICATOR):
       0 = off, 1 = left, 2 = right
   - LEFT  : scrolls one LED at a time from RIGHT to LEFT
   - RIGHT : scrolls one LED at a time from LEFT to RIGHT
   - 8 LEDs on P1.16 - P1.23 (Port 1 upper half is the only GPIO-capable
     part of Port 1 on the LPC2129)

   DISCONNECTION SAFETY:
   - This node never blocks waiting for CAN. It polls CAN1_RxReady()
     every loop pass; if nothing has arrived it just keeps animating
     (or stays off) using the last known command.
   - If no message arrives for LINK_TIMEOUT loop passes (Main Node
     unplugged), the LEDs are forced OFF so they cannot get stuck on.
   ===================================================================== */
#include <LPC21xx.h>
#include "types.h"
#include "can.h"
#include "delay.h"

#define LED_SHIFT   16                 /* P1.16 = LED0 ... P1.23 = LED7 */
#define LED_MASK    (0xFFu << LED_SHIFT)

#define SCROLL_STEP_MS   150
#define LINK_TIMEOUT     20            /* ~20 * 150ms = 3s with no message */

static void InitLEDs(void)
{
    IODIR1 |= LED_MASK;
    IOCLR1  = LED_MASK;
}

static void ShowLED(u8 pos)            /* pos 0..7, one LED on at a time */
{
    IOCLR1 = LED_MASK;
    IOSET1 = (1u << (LED_SHIFT + pos));
}

int main(void)
{
    struct CAN_Frame rx;
    u8  command = IND_OFF;
    u8  pos = 0;
    u32 missed = 0;
    s32 tick = 0;

    delay_init();
    InitLEDs();
    Init_CAN1();

    while (1)
    {
        /* --- check CAN without ever waiting --- */
        if (CAN1_RxReady())
        {
            CAN1_Rx(&rx);
            if (rx.ID == CAN_ID_INDICATOR)
            {
                command = (u8)(rx.Data1 & 0xFF);
                missed  = 0;
            }
        }
        else
        {
            missed++;
            if (missed > LINK_TIMEOUT)
            {
                command = IND_OFF;      /* Main Node gone: fail safe */
            }
        }

        /* --- animate the LEDs on our own clock --- */
        if (command == IND_OFF)
        {
            IOCLR1 = LED_MASK;
        }
        else
        {
            ShowLED(pos);
            if (command == IND_LEFT)  pos = (pos == 0) ? 7 : pos - 1;  /* right->left */
            else                      pos = (pos == 7) ? 0 : pos + 1;  /* left->right */
        }

        delay_ms(SCROLL_STEP_MS);
        tick++;
    }
}
