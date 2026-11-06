/* lcd.c - user's original 8-bit LCD driver (cleaned up, unchanged logic) */
#include <LPC21xx.h>
#include "types.h"
#include "defines.h"
#include "delay.h"
#include "lcd.h"

static void WriteLCD(u8 byte)
{
    /* Write byte on data pins P0.8-P0.15 */
    WRITEBYTE(IOPIN0, LCD_DATA, byte);

    /* Select write operation (RW = 0) */
    IOCLR0 = (1 << LCD_RW);

    /* High-to-low pulse on Enable pin (EN) */
    IOSET0 = (1 << LCD_EN);
    delay_us(1);
    IOCLR0 = (1 << LCD_EN);

    delay_ms(2);
}

void CmdLCD(u8 cmd)
{
    /* Select instruction register (RS = 0) */
    IOCLR0 = (1 << LCD_RS);
    WriteLCD(cmd);
}

void CharLCD(u8 asciiVal)
{
    /* Select data register (RS = 1) */
    IOSET0 = (1 << LCD_RS);
    WriteLCD(asciiVal);
}

void StrLCD(s8 *str)
{
    while (*str)
        CharLCD(*str++);
}

void U32LCD(u32 n)
{
    u8 a[10];
    s32 i = 0;

    if (n == 0)
    {
        CharLCD('0');
    }
    else
    {
        while (n > 0)
        {
            a[i] = (n % 10) + '0';
            n /= 10;
            i++;
        }
        for (--i; i >= 0; i--)
            CharLCD(a[i]);
    }
}

void S32LCD(s32 n)
{
    if (n < 0)
    {
        CharLCD('-');
        n = -n;
    }
    U32LCD(n);
}

void F32LCD(f32 fn, u8 nDP)
{
    u32 n;
    s32 i;

    if (fn < 0.0)
    {
        CharLCD('-');
        fn = -fn;
    }
    n = (u32)fn;
    U32LCD(n);
    CharLCD('.');
    for (i = 0; i < nDP; i++)
    {
        fn = (fn - n) * 10;
        n = (u32)fn;
        CharLCD(n + '0');
    }
}

void InitLCD(void)
{
    /* Configure P0.8-P0.15 (Data), P0.16 (RS), P0.17 (RW), P0.18 (EN) as outputs */
    IODIR0 |= ((0xFF << LCD_DATA) | (1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_EN));

    delay_ms(15);
    CmdLCD(0x30);
    delay_ms(4);
    delay_us(100);
    CmdLCD(0x30);
    delay_us(100);
    CmdLCD(0x30);
    CmdLCD(MODE_8BIT_2LINE);
    CmdLCD(DSP_ON_CUR_OFF);
    CmdLCD(CLEAR_LCD);
    CmdLCD(SHIFT_CUR_RIGHT);
}

void BuildCGRAM(u8 *p, u8 nBytes)
{
    u8 i;
    CmdLCD(GOTO_CGRAM_START);
    IOSET0 = (1 << LCD_RS);
    for (i = 0; i < nBytes; i++)
    {
        WriteLCD(p[i]);
    }
    CmdLCD(GOTO_LINE1_POS0);
}
