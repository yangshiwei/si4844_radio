//系统晶振 -- 11.0592MHz
//时钟周期 -- To = 1/11.0592 (uS)
//指令周期 -- TC = 12*To = 12/11.0592 (uS)

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "C8051F340_defs.h"

#define u32  unsigned long
#define u16  unsigned int
#define u8	 unsigned char

#define LCD_X_Parameter			132	   			//LCD宽度
#define LCD_Y_Parameter			64			  	//LCD高度
#define ZK_InitAddr       	0x00000
#define ASCII_InitAdd				0x8100

sbit Lcd_SDA  = P1^5;//SDI
sbit Lcd_CLK  = P1^6;//CLK
sbit Lcd_RD   = P1^4;//D/C
sbit Lcd_CS   = P1^3;//CS1

sbit FLASH_CS = P1^1;//CS2
sbit FLASH_SI = P1^2;//FSO


#endif
