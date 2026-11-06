/* ds18b20.h - DS18B20 driver (single sensor, external 3.3V power on VDD) */
#ifndef __DS18B20_H__
#define __DS18B20_H__
#include "types.h"

/* P0.8-P0.18 = LCD, P0.23-P0.25 = CAN. P0.19 is free.
   Needs a 4.7k pull-up from DQ to 3.3V, and VDD wired to 3.3V. */
#define DQ_PIN   19
#define DQ_MASK  (1u << DQ_PIN)

#define DS_OK          0
#define DS_NO_DEVICE   1
#define DS_CRC_ERROR   2
#define DS_TIMEOUT     3
#define DS_BAD_DATA    4

extern u8 ds_sp[9];         /* last scratchpad bytes, for debugging */

void InitDS18B20(void);
s32  DS18B20_ReadTemp(s32 *temp_x100);          /* 1/100 degC: 2506 = 25.06 C */
void DS18B20_Diag(u8 *idle, u8 *present, u8 *family);

#endif
