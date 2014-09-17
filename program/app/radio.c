/*****************************************************************************
demo.c this file include the main function of the clock radio demo
void main(void) the entrance of the program
void initialize(void) config the system initial the global variable
author; Wuliang
system config list
******************************************************************************/
//#include <C8051F340.H>
//#include "si4844_config.h"
//C8051 初始化头文件
#include "init_device.h"
//OLED 头文件
#include "OLED.h"
#include "OLED_codetab.h"
#include "delay.h"
/*****************************************************************************/
//extern u8 state_machine;
//extern u8 flag_tuner_irq;
void main(void)
{
        //configure the mcu system registers
        Init_Device();
		//EA = 1;
    Lcd_Init();
	FlashInit();
	
	LCDFill(0xff);
	DelayS(2);
	Lcdclear();
	
	LcdDisplay_Chinese(8,0,"成都惠特自动化");
	LcdDisplay_Chinese(0,2,"显示屏带中文字库");
	LcdDisplay_Chinese(0,4,"独家提供给力资料");
	LcdDisplay_Chinese(0,6,"欢迎选购量大从优");
	DelayS(2);    
	
	
	//initialize the variables and the peripherals
        //initialize(); 
        //i2c_reset_enable();
        //state_machine = SM_RADIO_RESET;
        //wait_ms(10);   
        while(1) {
                //key_scanning();
               // parse_atdd_status();
        }

}
/*******************************************************************************
 * this function config the peripherals and  initial the global variables used 
 * in the other functions.
 * ******************************************************************************/
void initialize()
{
        // take the si484x into reset status
        //i2c_reset_disable();
        // config the lcd driver
//      lcd_initial();
        //flag_tuner_irq = 0;
}
/********************************************************************************
 * this function scan the key matrix and get the key code if any key is pressed
 * ******************************************************************************/
/*void key_scanning(void)
{
        u8 io_status;
        u8 scan_status;

        io_scan_out = 0x0F;// set all the scan line to 0 to get if any key is pressed
        io_status = io_scan_in;
        if(io_status & 0x0F != 0x0F && !key_pressed && !key_debounce--) {
                //there is at least one key is pressed,then scan key matrix one line by one line
                scan_line = 1110B;
                key_code = 0;
                do{
                        io_scan_out = (scan_line<<4) + 0x0F;
                        io_status = io_scan_in;
                        if(io_status & 0x0F != 0x0F) {
                                break;
                        }
                        scan_line = (scan_line<<1) + 1;  
                        key_code += 4;
                }while( scan_status & 0x08 );
                
                if(io_status & 0x0F != 0x0F) {
                        while(io_status & 0x01); {
                                key_code++;
                                io_status >>=1;
                                key_pressed = 1;
                        }               
                }
        } else {
                key_pressed = 0;
                key_debounce = 2;
        }
}*/
/***********************************************************************
 * this function process the key event
 * *********************************************************************/
/*void key_process()
{
        switch(key_code) {
        case POWER_KEY: 
                switch_power(); 
                break;      
        case BAND_KEY: 
                adjust_band();
                break;      
        case VOL_DEC_KEY://volume decrease key 
                adjust_volume(0);
                break;    
        case VOL_INC_KEY: //volume increase key 
                adjust_volume(1);
                break;        
        case BASS_KEY:
                adjust_bass_treble(0);
                break;
        case TREBLE_KEY:
                adjust_bass_treble(1);
                break;
        default: 
                break;
        }
}*/





