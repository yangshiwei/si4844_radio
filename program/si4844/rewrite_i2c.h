bit   I2C_Start(void);
void  I2C_Stop(void);
void  I2C_Ack(void);
void  I2C_Nack(void);
bit   I2C_Send_Byte(uchar);
uchar I2C_Receive_Byte(void);
void  AT24C64_R(void *mcu_address,uint AT24C64_address,uint count);
void  AT24C64_W(void *mcu_address,uint AT24C64_address,uint count);