/* ultrasonic.h - HC-SR04 driver (Timer0 as stopwatch) */
#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__
#include "types.h"

#define TRIG_PIN   21
#define ECHO_PIN   22
#define TRIG_MASK  (1u << TRIG_PIN)
#define ECHO_MASK  (1u << ECHO_PIN)

#define PCLK_MHZ          60
#define US_WAIT_START_US  5000
#define US_MAX_ECHO_US    25000
#define US_NO_ECHO        0xFFFF

void InitUltrasonic(void);
u32  Ultrasonic_GetDistanceCm(void);

#endif
