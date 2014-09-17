//include the necessory header file and macro definition
#ifndef __SI4844_CONFIG_H__
#define __SI4844_CONFIG_H__
#include "compiler_defs.h"
#include "C8051F340_defs.h"
#include "stddef.h"
#include "si4844_i2c.h"
#include "si4844_clockradio.h"
/*****************************************************************************/
#define u32 unsigned long
#define u16 unsigned int
#define u8  unsigned char
#define i8  char
//define the state machine constant
#define	SM_RADIO_READY 0x80
#define	SM_RADIO_RESET 0x81
#define	SM_RADIO_POWERUP 0x82
#endif





