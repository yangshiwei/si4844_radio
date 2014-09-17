#ifndef __DELAY_H__
#define __DELAY_H__
/*****************************************************************************/
#include "compiler_defs.h"
/*****************************************************************************/
#define CONFIG_DELAY_MS_PARA			100
#define CONFIG_DELAY_S_PARA				1021
/*****************************************************************************/
void DelayUs(unsigned int time);
void DelayMs(unsigned int time);
void DelayS(unsigned int time);

#endif

