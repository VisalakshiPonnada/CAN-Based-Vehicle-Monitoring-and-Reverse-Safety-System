#include <LPC21xx.h>
#include "types.h"
#include "can.h"

#define CAN_QUANTA     16u
#define CAN_BRP        (CAN_PCLK / (CAN_BIT_RATE * CAN_QUANTA))
#define SAMPLE_POINT   ((7u * CAN_QUANTA) / 10u)
#define TSEG1          (SAMPLE_POINT - 1u)
#define TSEG2          (CAN_QUANTA - (1u + TSEG1))
#define SJW            ((TSEG2 >= 5u) ? 4u : (TSEG2 - 1u))
#define SAM            0u
#define BTR_LVAL  ((SAM << 23) | ((TSEG2 - 1u) << 20) | ((TSEG1 - 1u) << 16) | \
                   ((SJW - 1u) << 14) | (CAN_BRP - 1u))

#define TR_BIT_SET    (1u << 0)
#define AT_BIT_SET    (1u << 1)
#define RRB_BIT_SET   (1u << 2)
#define STB1_BIT_SET  (1u << 5)

#define RBS_BIT_READ   (1u << 0)
#define TBS1_BIT_READ  (1u << 2)
#define TCS1_BIT_READ  (1u << 3)

#define CAN_TX_TIMEOUT  500000u   /* bounded software wait, not a real timer */

void Init_CAN1(void)
{
    PINSEL1 = (PINSEL1 & ~(3u << 18)) | (1u << 18);  /* P0.25 = RD1 */

    C1MOD = 1;              /* reset mode */
    AFMR  = 2;               /* accept every message (no filtering) */
    C1BTR = BTR_LVAL;
    C1MOD = 0;               /* operating mode */
}

u8 CAN1_Tx(struct CAN_Frame *f)
{
    u32 t;

    t = CAN_TX_TIMEOUT;
    while (!(C1GSR & TBS1_BIT_READ))
        if (--t == 0) return 1;               /* bus busy too long: give up */

    C1TID1 = f->ID;
    C1TFI1 = ((u32)f->vbf.RTR << 30) | ((u32)f->vbf.DLC << 16);

    if (f->vbf.RTR == 0)
    {
        C1TDA1 = f->Data1;
        C1TDB1 = f->Data2;
    }

    C1CMR = STB1_BIT_SET | TR_BIT_SET;

    t = CAN_TX_TIMEOUT;
    while (!(C1GSR & TCS1_BIT_READ))
        if (--t == 0)
        {
            C1CMR = AT_BIT_SET;                /* abort: no other node ACKed it */
            return 1;
        }
    return 0;
}

u8 CAN1_RxReady(void)
{
    return (C1GSR & RBS_BIT_READ) ? 1 : 0;
}

void CAN1_Rx(struct CAN_Frame *f)
{
    /* NOTE: caller must check CAN1_RxReady() first. This function does
       NOT wait, so it never blocks the node if no message ever arrives. */
    f->ID       = C1RID;
    f->vbf.RTR  = (C1RFS >> 30) & 1;
    f->vbf.DLC  = (C1RFS >> 16) & 0x0F;

    if (f->vbf.RTR == 0)
    {
        f->Data1 = C1RDA;
        f->Data2 = C1RDB;
    }
    C1CMR = RRB_BIT_SET;
}

u8 CAN1_SendByte(u32 id, u8 value)
{
    struct CAN_Frame f;
    f.ID      = id;
    f.vbf.RTR = 0;
    f.vbf.DLC = 1;
    f.Data1   = value;
    f.Data2   = 0;
    return CAN1_Tx(&f);
}
