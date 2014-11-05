#ifndef __SI4844_I2C_H__
#define __SI4844_I2C_H__
#include "compiler_defs.h"
/*****************************************************************************/
void si4844_i2c_reset(void);
void si4844_i2c_reset_disable(void);
void si4844_i2c_reset_enable(void);
void si4844_i2c_stop(void);
void si4844_i2c_start(void);
void si4844_i2c_write_byte(U8 wrbuf);
void si4844_i2c_address(U8 addr,U8 rdwr);
U8 si4844_i2c_read_byte(void);
void si4844_i2c_send_ack(void);
void si4844_i2c_send_nack(void);
void si4844_i2c_read_buf(U8 len, U8 idata *buf);
void si4844_i2c_write_buf(U8 len, U8 idata *buf);
#endif
