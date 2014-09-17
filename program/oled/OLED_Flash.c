/************************************************************************************
*  Copyright (c), 2015, HelTec Automation Technology co.,LTD.
*            All rights reserved.
*
* Http:    www.heltec.cn
* Email:   cn.heltec@gmail.com
* WebShop: heltec.taobao.com
*
* File name: flash.c
* Project  : HelTec
* Processor: STC89C52
* Compiler : Keil C51 V9.52.0.0
* 
* Author : Aaron.Lee
* Version: 1.00
* Date   : 2014.4.20
* Email  : leehunter8801@gmail.com
* Modification: none
* 
* Description: 惠利特自动化自制字库芯片的驱动文件，仅适用于惠利特自动化(Heltec.taobao.com)所售字库版OLED显示屏
*
* Others: none;
*
* Function List:
*
* 1. void FlashInit(void) -- 字库芯片初始化
* 2. u8 Write_8bit_FLASH(u8 value) -- 向字库芯片写一个byte的数据(命令)
* 3. void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead) -- 读取字库芯片中的内容
*
* History: none;
*
*************************************************************************************/
#include "OLED_Config.h"
#include "OLED_Flash.h"
#include "OLED.h"
#include "Delay.h"

void SPI_FLASH_CS(u8 value)
{
    FLASH_CS = value;
}

void SPI_FLASH_SO(u8 value)
{
    Lcd_SDA = value;
}

void SPI_FLASH_CLK(u8 value)
{
    Lcd_CLK = value;
}

void FlashInit(void)
{
	SPI_FLASH_CS(1);
	SPI_FLASH_CLK(1);
}

u8 Write_8bit_FLASH(u8 value)
{
   u8 i;
   u8 temp=0;
   SPI_FLASH_CLK(1);
   for(i=0;i<8;i++)
   {
	   SPI_FLASH_CLK(0);
//	   DelayUs(2);
	   if((value&0x80)==0x80)
	       SPI_FLASH_SO(1);
	   else
	       SPI_FLASH_SO(0);
	   value<<=1;
//	   DelayUs(2);
 	   SPI_FLASH_CLK(1);
//	   DelayUs(2);
	   temp<<=1;
	   if(FLASH_SI==1)
	      temp++;
   }
   return(temp);
}

void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
	LCD_CS(1);
	SPI_FLASH_CS(0);
	Write_8bit_FLASH(0x03);
	Write_8bit_FLASH((ReadAddr & 0xFF0000) >> 16);
	Write_8bit_FLASH((ReadAddr& 0xFF00) >> 8);
	Write_8bit_FLASH(ReadAddr & 0xFF);
	
	while(NumByteToRead--)
	{
		*pBuffer = Write_8bit_FLASH(0xA5);
		pBuffer++;
	}
	
	SPI_FLASH_CS(1);
	LCD_CS(0);
}
