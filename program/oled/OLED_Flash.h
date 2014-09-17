#ifndef __FLASH_H__
#define __FLASH_H__

#include "c8051F340.h"

void SPI_FLASH_CS(u8 value);
void SPI_FLASH_SO(u8 value);
void SPI_FLASH_CLK(u8 value);
void FlashInit(void);
u8 Write_8bit_FLASH(u8 value);
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

#endif
