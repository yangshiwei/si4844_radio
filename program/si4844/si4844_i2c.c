#include "si4844_i2c.h"
#include "delay.h"
#include "C8051F340.h"
/*****************************************************************************/
#define I2C_READ    			1
#define I2C_WRITE   			0
#define I2C_CLK_CYCLE   		30
#define DELAY_FOR_I2C_CYCLE     DelayUs(I2C_CLK_CYCLE);
#define CONFIG_SDIO_INPUT       SDIO = 1;
#define SI4844_ADDRESS 			0x22
#define CONFIG_RESET_DELAY		1000//200uS
/*****************************************************************************/
sbit SDIO = P0^7;// Routed to SDIO on the Si47xx Part
sbit SCLK = P0^6;// Routed to SCLK on the Si47xx Part
sbit RSTB = P1^0;// Routed to RSTB on the baseboard
/*****************************************************************************/

/*****************************************************************************/
void si4844_i2c_reset(void)
{//Reset the Si47xx and select the appropriate bus mode. 
	SDIO = 1;
	SCLK = 1;
	RSTB = 0;
	DelayUs(CONFIG_RESET_DELAY);
	RSTB = 1;
	DelayUs(CONFIG_RESET_DELAY);
}
void si4844_i2c_reset_disable(void)
{//put the chip into the reset mode
	RSTB = 0;
	DelayUs(CONFIG_RESET_DELAY);
}
void si4844_i2c_reset_enable(void)
{//take the chip out of the reset mode
	SDIO = 1;
	SCLK = 1;
	RSTB = 1;
	DelayUs(CONFIG_RESET_DELAY);
}
#if 1
void si4844_i2c_stop(void) 
{// This routine send a stop condition      
	SDIO = 0;
	DELAY_FOR_I2C_CYCLE
	SCLK = 1;
	DELAY_FOR_I2C_CYCLE
	DELAY_FOR_I2C_CYCLE
	SDIO = 1;
	DELAY_FOR_I2C_CYCLE
}
/*****************************************************************************
 * this roution send a start condition
 * ***************************************************************************/
void si4844_i2c_start(void)
{ 
	SDIO = 0;
	DelayUs(50);
	SCLK = 0;
	DELAY_FOR_I2C_CYCLE
}
#endif
//-----------------------------------------------------------------------------
// This routine writes one byte of data
// in this function, use the pull-high resistor as the output high
// Inputs:
//       buf: The data to write
//-----------------------------------------------------------------------------
void si4844_i2c_write_byte(U8 wrbuf)
{
	U8 i;
	for (i=0; i < 8; i++) 
	{
		DELAY_FOR_I2C_CYCLE
		SDIO = (wrbuf & 0x80);
		wrbuf <<= 1;
		DELAY_FOR_I2C_CYCLE 
		SCLK = 1;
		DELAY_FOR_I2C_CYCLE
		DELAY_FOR_I2C_CYCLE
		SCLK = 0; 
	}
	CONFIG_SDIO_INPUT
	DELAY_FOR_I2C_CYCLE        
	SCLK = 1;
	if( SDIO == 1 ) 
	{
		_nop_();
	}
	DELAY_FOR_I2C_CYCLE
	DELAY_FOR_I2C_CYCLE
	SCLK = 0;
	DELAY_FOR_I2C_CYCLE
}
//-----------------------------------------------------------------------------
// This routine writes the device address and opereation type
// Inputs:
//       addr: the address to write
//       rdwr: read or write
//-----------------------------------------------------------------------------
void si4844_i2c_address(U8 addr,U8 rdwr)
{       
        si4844_i2c_start();
        if(rdwr == I2C_READ) 
		{
			addr |= 0x01;
        }
        si4844_i2c_write_byte(addr);
}

//-----------------------------------------------------------------------------
// This routine reads a byte from device
// return:
//       the data read from device
//-----------------------------------------------------------------------------
U8 si4844_i2c_read_byte(void)
{
        U8 i;
        U8 buf;   
        CONFIG_SDIO_INPUT 
        for (i = 0; i < 8; i++) {
                DELAY_FOR_I2C_CYCLE 
                SCLK = 1;
                buf = (buf << 1) | SDIO;
                DELAY_FOR_I2C_CYCLE
				DELAY_FOR_I2C_CYCLE    
				SCLK = 0;
        }
		DELAY_FOR_I2C_CYCLE
        return buf;
}
/***************************************************************************
 * send ack to the i2c device after a read action
 * ************************************************************************/
void si4844_i2c_send_ack(void)
{
        DELAY_FOR_I2C_CYCLE
        SDIO = 0;
        DELAY_FOR_I2C_CYCLE
        SCLK = 1;
		DELAY_FOR_I2C_CYCLE
        DELAY_FOR_I2C_CYCLE
        SCLK = 0;
		DELAY_FOR_I2C_CYCLE
}
/***************************************************************************
 * send nack to the i2c device after a read action
 * ************************************************************************/
void si4844_i2c_send_nack(void)
{
        DELAY_FOR_I2C_CYCLE
        SDIO = 1;
        DELAY_FOR_I2C_CYCLE
        SCLK = 1;
		DELAY_FOR_I2C_CYCLE    
		DELAY_FOR_I2C_CYCLE
        SCLK = 0;
		DELAY_FOR_I2C_CYCLE
}
//-----------------------------------------------------------------------------
// This routine reads indicated bytes data from the device
//
// Inputs:
//      len: Number of bytes to read
//      buf: the buffer in where the data is saved
//
//-----------------------------------------------------------------------------
void si4844_i2c_read_buf(U8 len, U8 idata *buf)
{
        U8 i = 0;       
        si4844_i2c_address(SI4844_ADDRESS,I2C_READ);
        while( i < len) {
                buf[i++] = si4844_i2c_read_byte();
                if( i < len) {
                        si4844_i2c_send_ack();
                }
        }
        si4844_i2c_send_nack();
        si4844_i2c_stop();  // Stop condition
}
//-----------------------------------------------------------------------------
// This routine writes indicated bytes data to the device.
// Inputs:
//              len: number of bytes to write
//              buf: data to be write
//-----------------------------------------------------------------------------
void si4844_i2c_write_buf(U8 len, U8 idata *buf)
{
        U8 i;
        si4844_i2c_address(SI4844_ADDRESS,I2C_WRITE);
        for (i = 0; i < len; i++) {
                si4844_i2c_write_byte(buf[i]);
        }
        si4844_i2c_stop();  // Stop condition  
}


