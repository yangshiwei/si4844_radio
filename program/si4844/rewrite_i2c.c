/*24C64×Ó³ÌÐò */
#include "pulsegen.h"
bit   I2C_Start(void);
void  I2C_Stop(void);
void  I2C_Ack(void);
void  I2C_Nack(void);
bit   I2C_Send_Byte(uchar);
unsigned char I2C_Receive_Byte(void);
void  AT24C64_R(void *mcu_address,uint AT24C64_address,uint count);
void  AT24C64_W(void *mcu_address,uint AT24C64_address,uint count);
sbit  I2C_SDA = P0^4;
sbit  I2C_SCK = P0^5;

bit I2C_Start(void)
{
 Delay_10_uS();
 I2C_SDA =1;
 Delay_10_uS();
 I2C_SCK =1;
 Delay_10_uS();
 if ( I2C_SDA == 0) return 0;
 if ( I2C_SCK == 0) return 0;
 I2C_SDA = 0;
 Delay_10_uS();
 I2C_SCK = 0;
 Delay_10_uS();
 return 1;
}
void  I2C_Stop(void)
{
 Delay_10_uS();
 I2C_SDA = 0;
 Delay_10_uS();
 I2C_SCK = 1;
 Delay_10_uS();
 I2C_SDA = 1;
 Delay_10_uS();
}
void I2C_Ack(void)
{
 Delay_10_uS();
 I2C_SDA=0;
 Delay_10_uS();
 I2C_SCK=1;
 Delay_10_uS();
 I2C_SCK=0;
 Delay_10_uS();
}
void I2C_Nack(void)
{
 Delay_10_uS();
 I2C_SDA=1;
 Delay_10_uS();
 I2C_SCK=1;
 Delay_10_uS();
 I2C_SCK=0;
 Delay_10_uS();
}
bit I2C_Send_Byte( uchar d)
{
 uchar i = 8;
 bit bit_ack;
 while( i-- )
 {
  Delay_10_uS();
  if ( d &0x80 )   I2C_SDA =1;
  else             I2C_SDA =0;
  Delay_10_uS();
  I2C_SCK = 1;
  Delay_10_uS();
  I2C_SCK = 0;
  d = d << 1;
 }
 Delay_10_uS();
 I2C_SDA = 1;
 Delay_10_uS();
 I2C_SCK = 1;
 Delay_10_uS();
 bit_ack = I2C_SDA;
 I2C_SCK =0;
 Delay_10_uS();
 return bit_ack;
}
uchar I2C_Receive_Byte(void)
{
 uchar i = 8, d;
 Delay_10_uS();
 I2C_SDA = 1;
 while ( i--)
 {
  d = d << 1;
  Delay_10_uS();
  I2C_SCK =1;
  if ( I2C_SDA ) d++;
  Delay_10_uS();
  I2C_SCK =0;
 }
 return d;
}

