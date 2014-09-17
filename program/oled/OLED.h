/************************************************************************************
*  Copyright (c), 2013, HelTec Automatic Technology co.,LTD.
*            All rights reserved.
*
* Http:    www.heltec.cn
* Email:   cn.heltec@gmail.com
* WebShop: heltec.taobao.com
*
* File name: OLED.h
* Project  : OLED
* Processor: STC89C52
* Compiler : Keil C51 Compiler
* 
* Author : Aaron.Lee
* Version: 1.00
* Date   : 2014.4.20
* Email  : leehunter8801@gmail.com
* Modification: none
* 
* Description:128*64点整OLED模块驱动文件，仅适用heltec.taobao.com所售产品
*
* Others: none;
*
* Function List: --
*
* History: none;
*
*************************************************************************************/

#ifndef __OLED_H__
#define __OLED_H__

#include "string.h"

void LCD_CS(u8 value);
void LCD_RES(u8 value);
void LCD_RD(u8 value);
void LCD_CLK(u8 value);
void LCD_SDA(u8 value);
void LCD_WData(u8 value);
void TransData(u8 value,u8 RD);
void PageSet(u8 page,u8 column);
void LCDFill(unsigned char bmp_dat) ;
void Lcdclear(void);
void Lcd_Init(void);
void LcdDisChar(u8 xPos,u8 yPos,u8 zknum,u8 *zkzip);
u8 LcdDisplay_HZ(u8 xPos,u8 yPos,u8 *GBCodeptr);
void LcdDisplay_Chinese(u8 xPos,u8 yPos,u8 *GBCodeptr);
void LcdDisplay_char(u8 xPos,u8 yPos,u8 *GBCodeptr);
void Draw_BMP(unsigned char x0, y0,x1, y1,unsigned char BMP[]);


#endif
