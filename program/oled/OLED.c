/************************************************************************************
*  Copyright (c), 2015, HelTec Automation Technology co.,LTD.
*            All rights reserved.
*
* Http:    www.heltec.cn
* Email:   cn.heltec@gmail.com
* WebShop: heltec.taobao.com
*
* File name: OLED.c
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
* Function List: --
*
* History: none;
*
*************************************************************************************/
#include "OLED_Config.h"
#include "OLED.h"
#include "Delay.h"
#include "OLED_Flash.h"
void LCD_CS(u8 value)	  
{
	 Lcd_CS = value;
}
void LCD_RD(u8 value)
{
	Lcd_RD = value;
}
void LCD_CLK(u8 value)
{
	Lcd_CLK = value;
}
void LCD_SDA(u8 value)
{
	Lcd_SDA= value;
}
void LCD_WData(u8 value)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		if((value << i) & 0x80)
		{
			LCD_SDA(1);
		}
		else
		LCD_SDA(0);
		LCD_CLK(0);
		LCD_CLK(1);
	}
}
void TransData(u8 value,u8 RD)
{
	LCD_RD(RD);
	LCD_WData(value);
}	

void PageSet(u8 page,u8 column)//page = y,column = x
{
	TransData(0xb0+page,0);
	TransData(((column&0xf0)>>4)|0x10,0);
	TransData((column&0x0f)|0x01,0);
}

void LCDFill(unsigned char bmp_dat)
{
	unsigned char y,x;
	
	LCD_CS(0);
	for(y=0;y<8;y++)
	{
		TransData(0xb0+y,0);
		TransData(0x01,0);
		TransData(0x10,0);
		for(x=0;x<LCD_X_Parameter;x++)
		TransData(bmp_dat,1);
	}
	LCD_CS(1);
}
void Lcdclear(void)
{
	LCDFill(0x00);
}

void Lcd_Init(void)
{
	DelayMs(500);	
	LCD_CS(0);
	TransData(0xae,0);
	TransData(0x02,0);
	TransData(0x10,0);
	TransData(0x40,0);
	TransData(0xb0,0);
	TransData(0x81,0);
	TransData(0x80,0);
	TransData(0xa1,0);
	TransData(0xa6,0);
	TransData(0xa8,0);
	TransData(0x3f,0);
	TransData(0xad,0);
	TransData(0x8b,0);
	TransData(0x30,0);
	TransData(0xc8,0);
	TransData(0xd3,0);
	TransData(0x00,0);
	TransData(0xd5,0);
	TransData(0x80,0);
	TransData(0xd9,0);
	TransData(0x1f,0);
	TransData(0xda,0);
	TransData(0x12,0);
	TransData(0xdb,0);
	TransData(0x40,0);
	TransData(0xaf,0);
	LCD_CS(1);
	Lcdclear();//清除显示
}

void LcdDisChar(u8 xPos,u8 yPos,u8 zknum,u8 *zkzip)
{
	u8 i;
	PageSet(yPos,xPos);
	for(i=0; i<zknum;i++)
	{
		TransData(zkzip[i],1);
	}
	PageSet(yPos+1,xPos);
	for(i=zknum; i<zknum*2;i++)
	{
		TransData(zkzip[i],1);
	}
}

/*******************************************************************************
* 功能           : 调取单个汉字字库显示汉字或字符函数
* Description    : 
* Input          : xPos ：      Lcd 横向地址0-128
                   yPos ：      Lcd 纵向地址0-64
* GBCodeptr      ：单个汉字或ASCII字符的字符串。
* Output         : None	
* Return         : 0：Lcd设置超出。1：设置正确。
*******************************************************************************/

u8 LcdDisplay_HZ(u8 xPos,u8 yPos,u8 *GBCodeptr)
{
	u8 msb,lsb,zknum;
	u8 zkzip[32];  //读取字库数据的缓存区
	u32 offset;	   //字库地址索引
	
	LCD_CS(0);
	if(xPos>=LCD_X_Parameter || yPos>=LCD_Y_Parameter) return 0 ;  //超范围退出
	msb= *GBCodeptr;     //汉字或ASCII的机器码的低8位。
	lsb= *(GBCodeptr+1); //汉字或ASCII的机器码的高8位。
	if (msb>128 && lsb>128)	//表明为汉字
	{
		if(xPos+16>LCD_X_Parameter || yPos+16>LCD_Y_Parameter)return 0; //超范围退出
		offset =ZK_InitAddr+((u32)((msb-0xA1)*94+(lsb-0xa1))*32);//具体算法详细查看字库原理
		zknum =16;	//汉字为16*16的字库
	}
	else	        //否则为ASCII码
	{
		if(xPos+8>LCD_X_Parameter || yPos+16>LCD_Y_Parameter)return 0;	//超范围退出
		offset =ASCII_InitAdd+(msb-32)*16;  //查看提供的2012_KZ.txt文档中的“!”的首地址
		zknum =8;   // ASCII码位8*16的字库
	}
	SPI_FLASH_BufferRead(zkzip,offset,zknum*2);	 //从FLASH中读取字库数据。
	LcdDisChar(xPos,yPos,zknum,zkzip);
	LCD_CS(1);

	return 1;
}

/*******************************************************************************
* 功能           : 显示多个汉字或字符的函数
* Description    : 
* Input          : xPos ：      Lcd 横向地址0-128   
                   yPos ：      Lcd 纵向地址0-64
* GBCodeptr      ：多个汉字的字符串。
* Output         : None	
* Return         : None
*******************************************************************************/
void LcdDisplay_Chinese(u8 xPos,u8 yPos,u8 *GBCodeptr)
{
	u8 i, len;
	len =  strlen((const char*)GBCodeptr);
	for(i=0;i<len;i++)
	{
		LcdDisplay_HZ(xPos+i*8,yPos,GBCodeptr+i);
		i++;
	}
}

/*******************************************************************************
* 功能           : 显示多个汉字或字符的函数
* Description    : 
* Input          : xPos ：      Lcd 横向地址0-127
                   yPos ：      Lcd 纵向地址0-7
* GBCodeptr      ：多个ASCII字符的字符串。
* Output         : None	
* Return         : None
*******************************************************************************/
void LcdDisplay_char(u8 xPos,u8 yPos,u8 *GBCodeptr)
{
	u8 i, len;
	len =  strlen((const char*)GBCodeptr);
	for(i=0;i<len;i++)
	{
		LcdDisplay_HZ(xPos+i*8,yPos,GBCodeptr+i);
	}
}
void Draw_BMP(unsigned char x0, y0,x1, y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;
  
	LCD_CS(0);
  if(y1%8==0) y=y1/8;
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		PageSet(y,x0);
    for(x=x0;x<x1;x++)
		{
			TransData(BMP[j++],1);
		}
	}
	LCD_CS(1);
}
