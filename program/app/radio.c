/*****************************************************************************
demo.c this file include the main function of the clock radio demo
void main(void) the entrance of the program
void initialize(void) config the system initial the global variable
author; Wuliang
system config list
******************************************************************************/
//#include <C8051F340.H>
#include "string.h"
#include "stdio.h"

//#include "si4844_config.h"
//C8051 初始化头文件
#include "init_device.h"
//OLED 头文件
#include "OLED.h"
#include "OLED_codetab.h"
#include "delay.h"
//SI4844 头文件
#include "si4844_clockradio.h"
/*****************************************************************************/
#define KEY_LOOP_TIME	1//1mS
/*****************************************************************************/
sbit VOL_ADD_KEY = P1^2;
sbit VOL_DEC_KEY = P1^1;
sbit BAND_ADD_KEY = P1^4;
sbit BAND_DEC_KEY = P1^3;
/*****************************************************************************/
void key_process(void);
/*****************************************************************************/

/*****************************************************************************/
void main(void)
{   
	Init_Device();//configure the mcu system registers
//    LED_R = 1;
//	LED_ER = 1;
//	LED_OV = 1;
//	LED_ST = 1;
//	LED_TU = 1;
	VOL_DEC_KEY = 1;
	VOL_ADD_KEY = 1;
	BAND_DEC_KEY = 1;
	BAND_ADD_KEY = 1;
	DelayS(2);  
	Lcd_Init();
	FlashInit();
	LCDFill(0xff);
	Lcdclear();	
	flag_tuner_irq = 0;
	EA = 1;
    si4844_initialize();//initialize the variables and the peripherals 
	DelayMs(10);
	si4844_i2c_reset_enable();
    state_machine = SM_RADIO_RESET;	
	LM4881_MUTE = 0;//打开OP
//	LED_R = 0;//ON
	while(1)
	{
		key_process();
		parse_atdd_status();	
    }

}



/***********************************************************************
 * this function process the key event
 * *********************************************************************/
void key_process(void)
{
	VOL_DEC_KEY = 1;
	VOL_ADD_KEY = 1;
	BAND_DEC_KEY = 1;
	BAND_ADD_KEY = 1;
	//读取按键信息 优先级 VOL- VOL+ Band- Band+
	if(VOL_DEC_KEY == 0)
	{//音量减小
		DelayMs(KEY_LOOP_TIME);
		if(VOL_DEC_KEY == 0)
		{
			adjust_volume(0);
		}
	}
	if(VOL_ADD_KEY == 0)
	{//音量增加
		DelayMs(KEY_LOOP_TIME);
		if(VOL_ADD_KEY == 0)
		{
			adjust_volume(1);
		}
	}
	if(BAND_DEC_KEY == 0)
	{//波段减小 
		DelayMs(KEY_LOOP_TIME);
		if(BAND_DEC_KEY == 0)
		{
			adjust_band(0);
			Lcdclear();
		}
	}
	if(BAND_ADD_KEY == 0)
	{//波段增加
		DelayMs(KEY_LOOP_TIME);
		if(BAND_ADD_KEY == 0)
		{
			adjust_band(1);
			Lcdclear();
		}
	}      
}




