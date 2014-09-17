/*****************************************************************
 * this file include the declaration of the functions in the si4844_i2c.c
 * **************************************************************/ 
#ifndef __SI4844_I2C_H__
#define __SI4844_I2C_H__
#include "compiler_defs.h"
#include "delay.h"
#include "C8051F340_defs.h"
void i2c_reset(void);
void i2c_reset_disable(void);
void i2c_reset_enable(void);
void i2c_stop(void);
void i2c_start(void);
void i2c_write_byte(u8 wrbuf);
void i2c_address(u8 addr,u8 rdwr);
u8 i2c_read_byte(void);
void i2c_send_ack(void);
void i2c_send_nack(void);
void i2c_read_buf(u8 len, u8 idata *buf);
void i2c_write_buf(u8 len, u8 idata *buf);

//#endif
