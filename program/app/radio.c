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
//C8051 ��ʼ��ͷ�ļ�
#include "init_device.h"
//OLED ͷ�ļ�
#include "OLED.h"
#include "OLED_codetab.h"
#include "delay.h"
//SI4844 ͷ�ļ�
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
	LM4881_MUTE = 0;//��OP
    VOL_DEC_KEY = 1;
	VOL_ADD_KEY = 1;
	BAND_DEC_KEY = 1;
	BAND_ADD_KEY = 1;
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
	//��ȡ������Ϣ ���ȼ� VOL- VOL+ Band- Band+
	if(VOL_DEC_KEY == 0)
	{//������С
		DelayMs(KEY_LOOP_TIME);
		if(VOL_DEC_KEY == 0)
		{
			adjust_volume(0);
		}
	}
	if(VOL_ADD_KEY == 0)
	{//��������
		DelayMs(KEY_LOOP_TIME);
		if(VOL_ADD_KEY == 0)
		{
			adjust_volume(1);
		}
	}
	if(BAND_DEC_KEY == 0)
	{//���μ�С 
		DelayMs(KEY_LOOP_TIME);
		if(BAND_DEC_KEY == 0)
		{
			adjust_band(0);
			Lcdclear();
		}
	}
	if(BAND_ADD_KEY == 0)
	{//��������
		DelayMs(KEY_LOOP_TIME);
		if(BAND_ADD_KEY == 0)
		{
			adjust_band(1);
			Lcdclear();
		}
	}      
}

U8 Bcd2Char(U8 bcd)
{//BCD��ת��ΪCHAR
	U8 l_temp,h_temp,temp;
	l_temp = (bcd & 0x0f);
	h_temp = ((bcd >> 4) & 0x0f);
	temp = h_temp * 100 + l_temp;
	return temp;
}




