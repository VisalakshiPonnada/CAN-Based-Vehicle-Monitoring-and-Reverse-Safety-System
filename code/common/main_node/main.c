/* =====================================================================
   MAIN NODE
   - Reads engine temperature (DS18B20) and shows it on the LCD
   - SW1 / SW2 (EINT0/EINT1) send LEFT / RIGHT indicator commands to
     the Indicator Node over CAN
   - MODE switch (EINT2) toggles FORWARD / REVERSE
   - In REVERSE mode, listens for the Reverse Alert Node's obstacle
     status and sounds a buzzer/LED when an obstacle is close

   DISCONNECTION SAFETY:
   - CAN1_Tx() times itself out (see can.c), so if the Indicator Node
     is unplugged, sending to it never blocks this node.
   - CAN1_Rx() is only ever called after CAN1_RxReady()==1, so this
     node never waits for the Reverse Alert Node. If no reverse-alert
     message has arrived for REVERSE_LINK_TIMEOUT loop passes, the
     node shows "No Signal" and switches the buzzer off (fail-safe),
     but temperature display and switches keep working normally.
   ===================================================================== */
#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "delay.h"
#include "lcd.h"
#include "ds18b20.h"
#include "can.h"
#include "eint.h"

/* Buzzer / alert LED - pick a free pin (not LCD/DS18B20/CAN/switches) */
#define BUZZER_PIN   0
#define BUZZER_MASK  (1u << BUZZER_PIN)

/* how many loop passes without a reverse-alert message before we
   treat the Reverse Alert Node as disconnected (each pass takes
   roughly 1 s, dominated by the 800 ms DS18B20 conversion)         */
#define REVERSE_LINK_TIMEOUT  5

static u8  indicatorState = IND_OFF;   /* 0=off,1=left,2=right */
static u8  reverseAlert   = 0;         /* last known obstacle state */
static u8  reverseLinkOk  = 0;
static u32 missedReverse  = 0;

static void InitBuzzer(void)
{
    IODIR0 |= BUZZER_MASK;
    IOCLR0  = BUZZER_MASK;
}

static void ShowTemp(s32 t100)
{
    u32 a;
    CmdLCD(GOTO_LINE1_POS0);
    StrLCD((s8 *)"T:");
    if (t100 < 0) { CharLCD('-'); a = (u32)(-t100); }
    else          { CharLCD(' '); a = (u32)t100;   }
    U32LCD(a / 100);
    CharLCD('.');
    CharLCD('0' + (a % 100) / 10);
    CharLCD('0' + (a % 10));
    CharLCD(0xDF);
    CharLCD('C');
    StrLCD((s8 *)"  ");
}

static void ShowTempError(s32 status)
{
    CmdLCD(GOTO_LINE1_POS0);
    if      (status == DS_NO_DEVICE) StrLCD((s8 *)"T: no sensor    ");
    else if (status == DS_CRC_ERROR) StrLCD((s8 *)"T: CRC err      ");
    else if (status == DS_BAD_DATA)  StrLCD((s8 *)"T: bad data     ");
    else                              StrLCD((s8 *)"T: timeout      ");
}

static void ShowLine2(void)
{
    CmdLCD(GOTO_LINE2_POS0);
    StrLCD((s8 *)reverseMode ? "REV " : "FWD ");

    if (indicatorState == IND_LEFT)  StrLCD((s8 *)"L ");
    else if (indicatorState == IND_RIGHT) StrLCD((s8 *)"R ");
    else                                   StrLCD((s8 *)"- ");

    if (!reverseMode)
    {
        StrLCD((s8 *)"        ");
    }
    else if (!reverseLinkOk)
    {
        StrLCD((s8 *)"No Signal ");
    }
    else if (reverseAlert)
    {
        StrLCD((s8 *)"OBSTACLE! ");
    }
    else
    {
        StrLCD((s8 *)"Clear     ");
    }
}

/* Send LEFT/RIGHT to the Indicator Node. If it is disconnected,
   CAN1_SendByte() simply reports failure and we carry on - the
   Main Node's own display keeps working either way.              */
static void SendIndicator(u8 state)
{
    indicatorState = state;
    CAN1_SendByte(CAN_ID_INDICATOR, state);   /* return value ignored on purpose */
}

/* Non-blocking poll: read every waiting CAN frame, act on the ones
   this node cares about. Never waits if nothing has arrived.      */
static void PollCAN(void)
{
    struct CAN_Frame rx;
    u8 gotReverseMsg = 0;

    while (CAN1_RxReady())
    {
        CAN1_Rx(&rx);
        if (rx.ID == CAN_ID_REVERSE)
        {
            reverseAlert   = (u8)(rx.Data1 & 0xFF);
            gotReverseMsg  = 1;
        }
    }

    if (gotReverseMsg)
    {
        missedReverse = 0;
        reverseLinkOk = 1;
    }
    else if (reverseMode)
    {
        missedReverse++;
        if (missedReverse > REVERSE_LINK_TIMEOUT)
        {
            reverseLinkOk = 0;
            reverseAlert  = 0;          /* fail-safe: no false alarm */
        }
    }
}

int main(void)
{
    s32 t, status;

    delay_init();
    InitLCD();
    InitDS18B20();
    InitBuzzer();
    Init_CAN1();
    InitEINT();

    CmdLCD(GOTO_LINE1_POS0);
    StrLCD((s8 *)"Main Node Ready");
    delay_ms(1000);

    while (1)
    {
        /* --- switches (set by the EINT ISRs) --- */
        if (sw1Flag)
        {
            SendIndicator(indicatorState == IND_LEFT ? IND_OFF : IND_LEFT);
            delay_ms(200);              /* debounce */
            sw1Flag = 0;
            EINT_ReEnable(0);
        }
        if (sw2Flag)
        {
            SendIndicator(indicatorState == IND_RIGHT ? IND_OFF : IND_RIGHT);
            delay_ms(200);
            sw2Flag = 0;
            EINT_ReEnable(1);
        }
        if (modeFlag)
        {
            if (!reverseMode) { missedReverse = 0; reverseLinkOk = 0; }
            delay_ms(200);
            modeFlag = 0;
            EINT_ReEnable(2);
        }

        /* --- CAN: never blocks --- */
        PollCAN();

        /* --- buzzer follows the (fail-safe) alert state --- */
        if (reverseMode && reverseLinkOk && reverseAlert) IOSET0 = BUZZER_MASK;
        else                                               IOCLR0 = BUZZER_MASK;

        /* --- temperature: this is the only blocking call (~800 ms,
               the DS18B20's real conversion time) --- */
        status = DS18B20_ReadTemp(&t);
        if (status == DS_OK) ShowTemp(t);
        else                 ShowTempError(status);

        ShowLine2();

        /* re-check CAN/switches more often while waiting out the
           rest of a ~1s cycle, so the buzzer/indicators stay responsive */
        delay_ms(100);
        PollCAN();
        if (reverseMode && reverseLinkOk && reverseAlert) IOSET0 = BUZZER_MASK;
        else                                               IOCLR0 = BUZZER_MASK;
    }
}
