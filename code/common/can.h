/* can.h - LPC2129 on-chip CAN1 driver, standard 11-bit frames.
   Tx and Rx are both non-blocking / self-timing out, so that one
   missing or disconnected node can never freeze another node.     */
#ifndef __CAN_H__
#define __CAN_H__
#include "types.h"

/* ---- Project message table: every node must agree on these ---- */
#define CAN_ID_INDICATOR  0x01   /* Main -> Indicator : data[0] = command */
#define CAN_ID_REVERSE    0x02   /* Reverse -> Main   : data[0] = 1/0     */

#define IND_OFF    0
#define IND_LEFT   1
#define IND_RIGHT  2

#define CAN_PCLK       60000000u
#define CAN_BIT_RATE   125000u

struct CAN_Frame
{
    u32 ID;
    struct BitField
    {
        u8 RTR : 1;
        u8 DLC : 4;
    } vbf;
    u32 Data1;                /* bytes 1-4, byte1 = lowest 8 bits */
    u32 Data2;                /* bytes 5-8 */
};

void Init_CAN1(void);
u8   CAN1_Tx(struct CAN_Frame *f);      /* 0 = sent, 1 = no ACK / timeout, never hangs */
u8   CAN1_RxReady(void);                /* 1 if a frame is waiting, 0 otherwise */
void CAN1_Rx(struct CAN_Frame *f);      /* call ONLY after CAN1_RxReady()==1  */
u8   CAN1_SendByte(u32 id, u8 value);   /* helper: 1-byte data frame */

#endif
