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
******************************************************************************/
#ifndef __OLED_H__
#define __OLED_H__
#include "compiler_defs.h"
#include "string.h"
/*****************************************************************************/
void SPI_FLASH_CS(U8 value);
void SPI_FLASH_SO(U8 value);
void SPI_FLASH_CLK(U8 value);
void FlashInit(void);
U8 Write_8bit_FLASH(U8 value);
void SPI_FLASH_BufferRead(U8* pBuffer, U32 ReadAddr, U16 NumByteToRead);
/*****************************************************************************/
void LCD_CS(U8 value);
void LCD_RES(U8 value);
void LCD_RD(U8 value);
void LCD_CLK(U8 value);
void LCD_SDA(U8 value);
void LCD_WData(U8 value);
void TransData(U8 value,U8 RD);
void PageSet(U8 page,U8 column);
void LCDFill(unsigned char bmp_dat) ;
void Lcdclear(void);
void Lcd_Init(void);
void LcdDisChar(U8 xPos,U8 yPos,U8 zknum,U8 *zkzip);
U8 LcdDisplay_HZ(U8 xPos,U8 yPos,U8 *GBCodeptr);
void LcdDisplay_Chinese(U8 xPos,U8 yPos,U8 *GBCodeptr);
void LcdDisplay_char(U8 xPos,U8 yPos,U8 *GBCodeptr);
void Draw_BMP(unsigned char x0, y0,x1, y1,unsigned char BMP[]);
#endif
