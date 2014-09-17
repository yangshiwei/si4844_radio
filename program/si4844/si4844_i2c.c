#include "si4844_i2c.h"
/*****************************************************************************/
#define I2C_READ    1
#define I2C_WRITE   0
#define I2C_CLK_CYCLE   10
#define DELAY_FOR_I2C_CYCLE     wait_us(I2C_CLK_CYCLE);
#define CONFIG_SDIO_INPUT       SDIO = 1;
#define IO2W_ADDRESS 0x22
/*****************************************************************************/
sbit SDIO        = P0^2;  // Routed to SDIO on the Si47xx Part
sbit SCLK        = P0^3;  // Routed to SCLK on the Si47xx Part
sbit RSTB        = P0^6;  // Routed to RSTB on the baseboard
//-----------------------------------------------------------------------------
// Reset the Si47xx and select the appropriate bus mode.
//-----------------------------------------------------------------------------
void i2c_reset(void)
{ 
        SDIO = 1;
        SCLK = 1;
        RSTB = 0;
        wait_us(200);
        RSTB = 1; 
        wait_us(200);
}
/******************************************************************************
 * put the chip into the reset mode
 ******************************************************************************/
void i2c_reset_disable(void)
{
        RSTB = 0;
        wait_us(200);
}
/******************************************************************************
 * take the chip out of the reset mode
 * ****************************************************************************/
void i2c_reset_enable(void)
{
        SDIO = 1;
        SCLK = 1;
        RSTB = 1;
        //wait_us(200);
}
//-----------------------------------------------------------------------------
// This routine send a stop condition
//-----------------------------------------------------------------------------
void i2c_stop(void) 
{
        
        SDIO = 0;
        DELAY_FOR_I2C_CYCLE
        SCLK = 1;
        DELAY_FOR_I2C_CYCLE
        SDIO = 1;
        DELAY_FOR_I2C_CYCLE
}
/*****************************************************************************
 * this roution send a start condition
 * ***************************************************************************/
void i2c_start(void)
{ 
        SDIO = 0;
        DELAY_FOR_I2C_CYCLE
        SCLK = 0;
		DELAY_FOR_I2C_CYCLE
}
//-----------------------------------------------------------------------------
// This routine writes one byte of data
// in this function, use the pull-high resistor as the output high
// Inputs:
//       buf: The data to write
//-----------------------------------------------------------------------------
void i2c_write_byte(u8 wrbuf)
{
        u8 i;
        
        for (i=0; i < 8; i++) {
                DELAY_FOR_I2C_CYCLE
                SDIO = (wrbuf & 0x80);
                wrbuf <<= 1;
                DELAY_FOR_I2C_CYCLE 
                SCLK = 1;
                DELAY_FOR_I2C_CYCLE
                SCLK = 0; 
        }
        CONFIG_SDIO_INPUT
        DELAY_FOR_I2C_CYCLE        
        SCLK = 1;
        if( SDIO == 1 ) {
			_nop_();
        }
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
void i2c_address(u8 addr,u8 rdwr)
{       
        i2c_start();
        if(rdwr == I2C_READ) 
		{
			addr |= 0x01;
        }
        i2c_write_byte(addr);
}

//-----------------------------------------------------------------------------
// This routine reads a byte from device
// return:
//       the data read from device
//-----------------------------------------------------------------------------
u8 i2c_read_byte(void)
{
        u8 i;
        u8 buf;   
        CONFIG_SDIO_INPUT 
        for (i = 0; i < 8; i++) {
                DELAY_FOR_I2C_CYCLE 
                SCLK = 1;
                buf = (buf << 1) | SDIO;
                DELAY_FOR_I2C_CYCLE
                SCLK = 0;
        }
		DELAY_FOR_I2C_CYCLE
        return buf;
}
/***************************************************************************
 * send ack to the i2c device after a read action
 * ************************************************************************/
void i2c_send_ack(void)
{
        DELAY_FOR_I2C_CYCLE
        SDIO = 0;
        DELAY_FOR_I2C_CYCLE
        SCLK = 1;
        DELAY_FOR_I2C_CYCLE
        SCLK = 0;
		DELAY_FOR_I2C_CYCLE
}
/***************************************************************************
 * send nack to the i2c device after a read action
 * ************************************************************************/
void i2c_send_nack(void)
{
        DELAY_FOR_I2C_CYCLE
        SDIO = 1;
        DELAY_FOR_I2C_CYCLE
        SCLK = 1;
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
void i2c_read_buf(u8 len, u8 idata *buf)
{
        u8 i = 0;       
        i2c_address(IO2W_ADDRESS,I2C_READ);
        while( i < len) {
                buf[i++] = i2c_read_byte();
                if( i < len) {
                        i2c_send_ack();
                }
        }
        i2c_send_nack();
        i2c_stop();  // Stop condition
}
//-----------------------------------------------------------------------------
// This routine writes indicated bytes data to the device.
// Inputs:
//              len: number of bytes to write
//              buf: data to be write
//-----------------------------------------------------------------------------
void i2c_write_buf(u8 len, u8 idata *buf)
{
        u8 i;
        i2c_address(IO2W_ADDRESS,I2C_WRITE);
        for (i = 0; i < len; i++) {
                i2c_write_byte(buf[i]);
        }
        i2c_stop();  // Stop condition  
}


