#ifndef __SI4844_I2C_H__
#define __SI4844_I2C_H__
#include "compiler_defs.h"
/*****************************************************************************/
void i2c_reset(void);
void i2c_reset_disable(void);
void i2c_reset_enable(void);
void i2c_stop(void);
void i2c_start(void);
void i2c_write_byte(U8 wrbuf);
void i2c_address(U8 addr,U8 rdwr);
U8 i2c_read_byte(void);
void i2c_send_ack(void);
void i2c_send_nack(void);
void i2c_read_buf(U8 len, U8 idata *buf);
void i2c_write_buf(U8 len, U8 idata *buf);
//#endif
