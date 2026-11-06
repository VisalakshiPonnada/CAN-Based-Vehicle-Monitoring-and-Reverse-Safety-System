/* lcd.h - 16x2 HD44780 LCD, 8-bit mode, RW driven by the MCU */
#ifndef __LCD_H__
#define __LCD_H__

#include "types.h"

void InitLCD(void);
void CmdLCD(u8 cmd);
void CharLCD(u8 asciiVal);
void StrLCD(s8 *str);
void U32LCD(u32 n);
void S32LCD(s32 n);
void F32LCD(f32 fn, u8 nDP);
void BuildCGRAM(u8 *p, u8 nBytes);

#endif
