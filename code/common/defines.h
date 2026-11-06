/* defines.h - LCD command bytes, pin shifts, and the WRITEBYTE helper
   used by lcd.c. Edit LCD_DATA/LCD_RS/LCD_RW/LCD_EN if your board
   wires the LCD to different pins.                                   */
#ifndef __DEFINES_H__
#define __DEFINES_H__

#include "types.h"

/* ---- LCD pin shifts on Port 0 (8-bit data mode) ---- */
#define LCD_DATA   8     /* P0.8 - P0.15 = D0-D7 */
#define LCD_RS     16    /* P0.16 */
#define LCD_RW     17    /* P0.17 */
#define LCD_EN     18    /* P0.18 */

/* Writes 'byte' into an 8-bit field of 'reg' starting at bit 'shift',
   without disturbing the other bits of that register.                */
#define WRITEBYTE(reg, shift, byte) \
        ((reg) = ((reg) & ~(0xFFUL << (shift))) | (((u32)(byte) & 0xFFUL) << (shift)))

/* ---- HD44780 command bytes ---- */
#define MODE_8BIT_2LINE   0x38
#define DSP_ON_CUR_OFF    0x0C
#define CLEAR_LCD         0x01
#define SHIFT_CUR_RIGHT   0x06
#define GOTO_CGRAM_START  0x40
#define GOTO_LINE1_POS0   0x80
#define GOTO_LINE2_POS0   0xC0

#endif
