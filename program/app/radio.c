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


sbit VOL_ADD_KEY = P1^2;
sbit VOL_DEC_KEY = P1^1;
sbit BAND_ADD_KEY = P1^4;
sbit BAND_DEC_KEY = P1^3;

void key_process(void);
/*
const char *BandInfo[82] = {
	"FM1:87-108M 75us","SS=6dB,RT=20dB",//0
	"FM1:87-108M 75us","SS=12dB,RT=28dB",//1
	"FM1:87-108M 50us","SS=6dB,RT=20dB",//2
	"FM1:87-108M 50us","SS=12dB,RT=28dB",//3
	"FM2:86.5-109M 75us","SS=6dB,RT=20dB",//4
	"FM2:86.5-109M 75us","SS=12dB,RT=28dB",//5
	"FM2:86.5-109M 50us","SS=6dB,RT=20dB",//6
	"FM2:86.5-109M 50us","SS=12dB,RT=28dB",//7
	"FM3:87.3-108.25M 75us","SS=6dB,RT=20dB",//8
	"FM3:87.3-108.25M 75us","SS=12dB,RT=28dB",//9
	"FM3:87.3-108.25M 50us","SS=6dB,RT=20dB",//10
	"FM3:87.3-108.25M 50us","SS=12dB,RT=28dB",//11
	"FM4:76-90M 75us","SS=6dB,RT=20dB",//12
	"FM4:76-90M 75us","SS=12dB,RT=28dB",//13
	"FM4:76-90M 50us","SS=6dB,RT=20dB",//14
	"FM4:76-90M 50us","SS=12dB,RT=28dB",//15
	"FM5:64-87M 75us","SS=6dB,RT=20dB",//16
	"FM5:64-87M 75us","SS=12dB,RT=28dB",//17
	"FM5:64-87M 50us","SS=6dB,RT=20dB",//18
	"FM5:65-87M 50us","SS=12dB,RT=28dB",//19
	"AM1:520-1710K 10K","",//20
	"AM2:522-1620K 9K","",//21
	"AM3:504-1665K 9K","",//22
	"AM4:520-1730K 10K","",//23
	"AM5:510-1750K 10K","",//24
	"SW1:5.6-6.4M 5K","",//25
	"SW2:5.9-5.62M 5K","",//26
	"SW3:6.8-7.6M 5K","",//27
	"SW4:7.1-7.6M 5K","",//28
	"SW5:9.2-10M 5K","",//29
	"SW6:9.2-9.9M 5K","",//30
	"SW7:11.45-12.25M 5K","",//31
	"SW8:11.6-12.2M 5K","",//32
	"SW9:13.4-14.2M 5K","",//33
	"SW10:13.57-13.87M 5K",""//,34
	"SW11:15-15.9M 5K","",//35
	"SW12:15.1-15.8M 5K","",//36
	"SW13:17.1-18M 5K","",//37
	"SW14:17.47-17.9M 5K",""//38
	"SW15:21.2-22M 5K",""//39
	"SW16:21.45-21.85M",""//40
};*/
#define KEY_LOOP_TIME	1//1mS
/*****************************************************************************/
void main(void)
{   
	Init_Device();//configure the mcu system registers
    Lcd_Init();
	FlashInit();
	LCDFill(0xff);
	Lcdclear();	
	//LcdDisplay_Chinese(8,0,"成都惠特自动化");
	//LcdDisplay_Chinese(0,2,"显示屏带中文字库");
	//LcdDisplay_Chinese(0,4,"独家提供给力资料");
	//LcdDisplay_Chinese(0,6,"欢迎选购量大从优");
	//DelayS(2);    	
	flag_tuner_irq = 0;
	EA = 1;
    si4844_initialize();//initialize the variables and the peripherals 
	si4844_i2c_reset_enable();
    state_machine = SM_RADIO_RESET;	
    Lcdclear();	
	//LcdDisplay_char(0,0,BandIndex0a);
	//LcdDisplay_char(0,2,BandIndex0b);
	DelayMs(10);
	LM4881_MUTE = 0;//打开OP
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
	{//波段增加
		DelayMs(KEY_LOOP_TIME);
		if(BAND_DEC_KEY == 0)
		{
			adjust_band(1);
		}
	}
	if(BAND_ADD_KEY == 0)
	{//波段减小
		DelayMs(KEY_LOOP_TIME);
		if(BAND_ADD_KEY == 0)
		{
			adjust_band(0);
		}
	}      
}

U8 Bcd2Char(U8 bcd)
{//BCD码转换为CHAR
	U8 l_temp,h_temp,temp;
	l_temp = (bcd & 0x0f);
	h_temp = ((bcd >> 4) & 0x0f);
	temp = h_temp * 100 + l_temp;
	return temp;
}



