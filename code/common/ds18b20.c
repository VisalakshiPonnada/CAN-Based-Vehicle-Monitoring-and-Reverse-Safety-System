#include <LPC21xx.h>
#include "types.h"
#include "delay.h"
#include "ds18b20.h"

u8 ds_sp[9];

/* Open-drain emulation: output latch stays 0.
   Pin OUTPUT -> line pulled LOW.  Pin INPUT -> released, pull-up = HIGH. */
#define DQ_LOW()      (IODIR0 |=  DQ_MASK)
#define DQ_RELEASE()  (IODIR0 &= ~DQ_MASK)
#define DQ_READ()     ((IOPIN0 & DQ_MASK) ? 1 : 0)

void InitDS18B20(void)
{
    PINSEL1 &= ~(3u << ((DQ_PIN - 16) * 2));   /* P0.19 as GPIO */
    IOCLR0   = DQ_MASK;
    DQ_RELEASE();
}

static u8 ow_reset(void)
{
    u8 present;

    DQ_RELEASE();
    delay_us(10);
    if (!DQ_READ()) return 0;      /* bus stuck low: no pull-up or short */

    DQ_LOW();
    delay_us(480);
    DQ_RELEASE();
    delay_us(70);
    present = !DQ_READ();
    delay_us(410);
    return present;
}

static void ow_write_bit(u8 b)
{
    DQ_LOW();
    if (b) { delay_us(6);  DQ_RELEASE(); delay_us(64); }
    else   { delay_us(60); DQ_RELEASE(); delay_us(10); }
}

static u8 ow_read_bit(void)
{
    u8 b;
    DQ_LOW();
    delay_us(2);
    DQ_RELEASE();
    delay_us(8);
    b = DQ_READ();
    delay_us(55);
    return b;
}

static void ow_write_byte(u8 v)
{
    u8 i;
    for (i = 0; i < 8; i++) { ow_write_bit(v & 1); v >>= 1; }
}

static u8 ow_read_byte(void)
{
    u8 i, v = 0;
    for (i = 0; i < 8; i++) v |= (ow_read_bit() << i);
    return v;
}

static u8 crc8(u8 *d, u8 len)
{
    u8 crc = 0, i, j;
    for (i = 0; i < len; i++) {
        crc ^= d[i];
        for (j = 0; j < 8; j++)
            crc = (crc & 1) ? ((crc >> 1) ^ 0x8C) : (crc >> 1);
    }
    return crc;
}

s32 DS18B20_ReadTemp(s32 *temp_x100)
{
    u8  i;
    s16 raw;

    if (!ow_reset()) return DS_NO_DEVICE;
    ow_write_byte(0xCC);           /* Skip ROM */
    ow_write_byte(0x44);           /* Convert T */

    /* Strong pull-up for the conversion time: works with external VDD
       and also with parasite power.                                  */
    IOSET0  = DQ_MASK;
    IODIR0 |= DQ_MASK;
    delay_ms(800);
    IOCLR0  = DQ_MASK;
    DQ_RELEASE();

    if (!ow_reset()) return DS_NO_DEVICE;
    ow_write_byte(0xCC);
    ow_write_byte(0xBE);           /* Read Scratchpad */
    for (i = 0; i < 9; i++) ds_sp[i] = ow_read_byte();

    if (crc8(ds_sp, 8) != ds_sp[8]) return DS_CRC_ERROR;
    if ((ds_sp[4] & 0x9F) != 0x1F)  return DS_BAD_DATA;

    raw = (s16)((ds_sp[1] << 8) | ds_sp[0]);
    *temp_x100 = ((s32)raw * 625) / 100;
    return DS_OK;
}

void DS18B20_Diag(u8 *idle, u8 *present, u8 *family)
{
    *idle = 0; *present = 0; *family = 0;
    DQ_RELEASE();
    delay_us(50);
    *idle = DQ_READ();
    if (!*idle) return;
    if (!ow_reset()) return;
    *present = 1;
    ow_write_byte(0x33);           /* Read ROM */
    *family = ow_read_byte();
}
