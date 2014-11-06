/*************************************************************************
 * this file include the functions of write or read command
 * void atdd_power_up(U8 band_select_mode,U8 band_index)
 * void atdd_power_down(void)
 * void atdd_set_property(U16 property_id,U16 property_value);
 * U16 atdd_get_property(U16 property_id);
 * void atdd_wait_cts(void);
 * void atdd_command(U8 cmd_size,U8 idata *cmd_buf,U8 replay_size,U8 idata *reply);
 * ***************************************************************************/
#include "si4844_clockradio.h"
#include "si4844_i2c.h"
#include "delay.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "OLED.h"
#include "math.h"
#include "string.h"
/*****************************************************************************/
#define FM      0
#define AM      1
#define SW      2


//ATDD COMMAND
#define ADJPT_ATTN      0x08
#define ADJPT_STEO      0x10
#define ATDD_POWER_DOWN 0x11
#define ATDD_GET_STATUS 0xE0
#define ATDD_POWER_UP   0xE1
#define ATDD_AUDIO_MODE 0xE2
#define ATDD_XOSCEN     0x80
#define ATDD_XOWAIT     0x40


//1--> external 32768Hz crystal;0--> RCLK
#define XOSCEN          1
//0--> wait 600ms for crystal oscillator setup, for normal crystal
//1--> wait 850ms for crystal oscillator setup, for worse crystal which the setup time is long
#define XOWAIT_LONG     1

#define FM_MONO         0x04
#define HOSTRST         0x40
#define HOSTPWRUP       0x20
#define INFORDY         0x10
#define STATION         0x08
#define STEREO          0x04
#define TUNERPRI        0x02
#define HOSTBAND        0x01

/*********************************************************************
 * this function initial the tuner after powerup command
 * *******************************************************************/
#define FM_BAND_TOP     10800
#define FM_BAND_BOTTOM  8750
#define FM_BAND_SPACING 10

#define AM_BAND_TOP     1710
#define AM_BAND_BOTTOM  522
#define AM_BAND_SPACING 9

#define SW_BAND_TOP     26500
#define SW_BAND_BOTTOM  2300
#define SW_BAND_SPACING 5
/***************************************************
 * config fm band
 * if select host setting mode, user can initial the 
 *      de-emphasis
 *      blend_mono,band_stereo,band_separation
 * *************************************************/
#define ADJPT_STERRO_DISABLE 0
#define FM_ADJPT_ATTENUATION_DISABLE 0
#define FM_AUDIO_MODE 3
#define FM_FORCE_MONO 0//1:de-emphasis 50us 2:de-emphasis 75us
#define FM_DE_EMPHASIS 2
#define FM_BLEND_MONO  8
#define FM_BLEND_STEREO 49
#define FM_SEPARATION 0x9F
/*******************************************************************
 * define some global variable
 * ***************************************************************/

U8 data state_machine = 0;
U8 data band_index = 0;
U8 data band_mode;
U8 data flag_tuner_irq;
U8 data flag_station;
U8 data flag_stereo;
U8 data freq_bcd[2];
const U8 code max_vol_list[9] = {59,60,61,62,63,62,61,60,59};
U8 data max_vol;
bit data flag_host_band;
bit data flag_tuner_pri;
U8 data fm_bass_treble;
U8 data am_bass_treble;
U8 data volume;

code char sBandInfo[82][32] = {
	"FM1:87-108M","75u,S6,R20",//0
	"FM1:87-108M","75u,S12,R28",//1
	"FM1:87-108M","50u,S6,R20",//2
	"FM1:87-108M","50u,S12,R28",//3
	"FM2:86.5-109M","75u,S6,R20",//4
	"FM2:86.5-109M","75u,S12,R28",//5
	"FM2:86.5-109M","50u,S6,R20",//6
	"FM2:86.5-109M","50u,S12,R28",//7
	"FM3:87.3-108.25M","75u,S6,R20",//8
	"FM3:87.3-108.25M","75u,S12,R28",//9
	"FM3:87.3-108.25M","50u,S6,R20",//10
	"FM3:87.3-108.25M","50u,S12,R28",//11
	"FM4:76-90M","75u,S6,R20",//12
	"FM4:76-90M","75u,S12,R28",//13
	"FM4:76-90M","50u,S6,R20",//14
	"FM4:76-90M","50u,S12,R28",//15
	"FM5:64-87M","75u,S6,R20",//16
	"FM5:64-87M","75u,S12,R28",//17
	"FM5:64-87M","50u,S6,R20",//18
	"FM5:65-87M","50u,S12,R28",//19
	"AM1:520-1710K","Step:10K",//20
	"AM2:522-1620K","Step:9K",//21
	"AM3:504-1665K","Step:9K",//22
	"AM4:520-1730K","Step:10K",//23
	"AM5:510-1750K","Step:10K",//24
	"SW1:5.6-6.4M","Step:5K",//25
	"SW2:5.9-5.62M","Step:5K",//26
	"SW3:6.8-7.6M","Step:5K",//27
	"SW4:7.1-7.6M","Step:5K",//28
	"SW5:9.2-10M","Step:5K",//29
	"SW6:9.2-9.9M","Step:5K",//30
	"SW7:11.45-12.25M","Step:5K",//31
	"SW8:11.6-12.2M","Step:5K",//32
	"SW9:13.4-14.2M","Step:5K",//33
	"SW10:13.57-13.87M","Step:5K",//,34
	"SW11:15-15.9M","Step:5K",//35
	"SW12:15.1-15.8M","Step:5K",//36
	"SW13:17.1-18M","Step:5K",//37
	"SW14:17.47-17.9M","Step:5K",//38
	"SW15:21.2-22M","Step:5K",//39
	"SW16:21.45-21.85M","Step:5K"//40
};
/*****************************************************************************/
void INT0_irq() interrupt 0
{//this isr set a flag, but get atdd status in main loop
	if(state_machine & SM_RADIO_READY) 
	{
		flag_tuner_irq = 1;
	}
}

float fm_bcdfreq2float(U8 bcd_l, U8 bcd_h)
{//FM BCD转换字符串 
	float temp;
	temp = (bcd_l & 0x0f) / 10.0;
	temp += ((bcd_l >> 4) & 0x0f);
	temp += (bcd_h & 0x0f) * 10;
	temp += ((bcd_h >> 4) & 0x0f) * 100;
	return temp;
}
float am_bcdfreq2float(U8 bcd_l, U8 bcd_h)
{//AM
	float temp;
	temp = (bcd_l & 0x0f);
	temp += ((bcd_l >> 4) & 0x0f) * 10;
	temp += (bcd_h & 0x0f) * 100;
	temp += ((bcd_h >> 4) & 0x0f) * 1000;
	return temp;
}
float sw_bcdfreq2float(U8 bcd_l, U8 bcd_h)
{//SW
	float temp;
	U8	err;
	temp = (bcd_l & 0x0f) / 100.0;
	temp += ((bcd_l >> 4) & 0x0f) / 10.0;
	temp += (bcd_h & 0x0f);
	err = ((bcd_h >> 4) & 0x0f);
	if(err >= 8)
	{
		err -= 8;
	}
	temp += err * 10;
	return temp;
}
void si4844_initialize(void)
{ 
	si4844_i2c_reset_disable();//take the si484x into reset status
	flag_tuner_irq = 0;
}

/*
void switch_power()
{//this function enter radio mode or exit radio
        if(state_machine & SM_RADIO_READY) {
                state_machine = SM_POWER_OFF;
                i2c_reset_disable();
        } else {
                state_machine = SM_RADIO_RESET;
                i2c_reset_enable();
        }
}
*/
/**********************************************************************************
 * this function switch band
 * *******************************************************************************/
void adjust_band(U8 direction)
{
	if(state_machine & SM_RADIO_READY)
	{
		if(direction)
		{
			if(band_index < 41) 
			{
				band_index ++;
			}	
			si4844_i2c_reset();
			state_machine = SM_RADIO_RESET;
		}
		else
		{
			if(band_index > 0)
			{
				band_index --;
			}
			si4844_i2c_reset();
			state_machine = SM_RADIO_RESET;
		}
	}        
}
/**********************************************************************************
 * this function adjust volume
 * *******************************************************************************/
void adjust_volume(U8 direction)
{
	if(state_machine & SM_RADIO_READY)
	{
		if (direction == 0) 
		{
			if(volume)
			{
				volume--;
			}
		} else 
		{
			if(volume < 63)
			{
				volume++;
			}
			
		}
		if(volume == 0)
		{
			LM4881_MUTE = 1;
		}
		else 
		{
			LM4881_MUTE = 0;
		}
		atdd_set_volume(volume);
	//state_machine = SM_RADIO_VOLUME;     
	}
}
/***************************************************************
 * this function adjust bass treble
 * ************************************************************/
/*
void adjust_bass_treble(U8 direction)
{
        if(state_machine & SM_RADIO_READY) {
                if (direction == 0) {
                        if(bass_treble) {
                                bass_treble--;
                        }
                } else {
                        if(bass_treble < 8) {
                                bass_treble++;
                        }
                }
                si48xx_set_bass_treble(bass_treble);
                state_machine = SM_RADIO_BASS_TREBLE;     
        }
}
*/
/************************************************************************
 * this function parse the atdd status and process the tuner requirement
 * **********************************************************************/
void parse_atdd_status()
{
	U8 idata atdd_status[4];     
	S8 lcdstring[64];
	float ftemp;
	//S8 ctemp;
	if( flag_tuner_irq ) 
	{
		flag_tuner_irq = 0;
		atdd_get_status(atdd_status);
		flag_host_band = atdd_status[0] & HOSTBAND;
		flag_tuner_pri = atdd_status[0] & TUNERPRI;
		if(atdd_status[0] & INFORDY) 
		{// STATUS.4 1--> BAND INDEX AND FREQUENCY INFO IS READY
			//check who detect the band,Host mcu or tuner itself
			//atdd_status[0].1 0--> tuner; 1--> host mcu
			if(!flag_host_band) 
			{
				// if tuner detect the band, the current will be report to 
				// host through att_get_status command,and local at response 1
				// host need to memorize the band index reported by tuner and send
				// it back to tuner later 
				band_index = atdd_status[1] & 0x3F;
				band_mode = atdd_status[1] >> 6;
			}
			if(state_machine == SM_RADIO_POWERUP )
			{//after the power status and the info ready again,enter ready status
				state_machine = SM_RADIO_READY;
			}
					
			if(state_machine == SM_RADIO_READY) 
			{//extract the frequency and indicator information 
				//after enter the radio ready state,start to display frequency and indicators
				flag_station = 0;
				if(atdd_status[0] & STATION) 
				{
					flag_station = 1;
				}
				flag_stereo = 0;
				if(atdd_status[0] & STEREO) 
				{
					flag_stereo = 1;
				}
				freq_bcd[0] = atdd_status[2];
                freq_bcd[1] = atdd_status[3];
				switch(band_mode)
				{
					case FM:
					{
						freq_bcd[0] = atdd_status[2];
						freq_bcd[1] = atdd_status[3];
						ftemp = fm_bcdfreq2float(freq_bcd[1],freq_bcd[0]);
						memset( lcdstring, 0, sizeof(lcdstring));
						sprintf(lcdstring, "Freq:%05.1fMHz", ftemp);
						LcdDisplay_char(0, 4, lcdstring);
						//判断是否是立体声
						if(flag_stereo)
						{//立体声打开
							//LED_ST = 0;
						}
						else
						{//立体声关闭
							//LED_ST = 1;
						}
						break;
					}
					case AM:
					{
						//LED_ST = 1;
						freq_bcd[0] = atdd_status[2];
						freq_bcd[1] = atdd_status[3];
						ftemp = am_bcdfreq2float(freq_bcd[1],freq_bcd[0]);
						memset( lcdstring, 0, sizeof(lcdstring));
						sprintf(lcdstring, "Freq:%04.0fKHz", ftemp);
						LcdDisplay_char(0, 4, lcdstring);
						break;
					}
					case SW:
					{
						//LED_ST = 1;
						freq_bcd[0] = atdd_status[2];
						freq_bcd[1] = atdd_status[3];
						ftemp = sw_bcdfreq2float(freq_bcd[1],freq_bcd[0]);
						memset( lcdstring, 0, sizeof(lcdstring));
						sprintf(lcdstring, "Freq:%05.2fMHz", ftemp);
						LcdDisplay_char(0, 4, lcdstring);
						break;
					}
					default:break;
				}
			}
		} 
		else
		{
		}
		if(atdd_status[0] & HOSTRST)
		{// status.6 1--> need reset the tune
			si4844_i2c_reset();
			state_machine = SM_RADIO_RESET;
		}	
		if(atdd_status[0] & HOSTPWRUP) 
		{// STATUS.5 1--> need power up command
			config_tune();// powerup and config the systom .
			state_machine = SM_RADIO_POWERUP;
		}
	}
}
void config_tune(void)
{
	//确定波段 
	if(band_index <= 19)
	{
		band_mode = FM;
	}
	else if(band_index <= 24)
	{
		band_mode = AM;
	}
	else
	{
		band_mode = SW;
	}
	
	if( band_mode == FM) 
	{
		atdd_power_up(XOSCEN,XOWAIT_LONG,band_index,FM_BAND_BOTTOM,FM_BAND_TOP,FM_BAND_SPACING);
		// the fist power up after reset,wait 100ms
		// for the crystal stable
		if(state_machine == SM_RADIO_RESET) 
		{
			DelayMs(600);
		}
		config_fm();
	}
	if(band_mode == AM) {
			atdd_power_up(XOSCEN,XOWAIT_LONG,band_index,AM_BAND_BOTTOM,AM_BAND_TOP,AM_BAND_SPACING);
			// the fist power up after reset,wait 100ms
			// for the crystal stable
			if(state_machine == SM_RADIO_RESET) {
					DelayMs(600);
			}
			config_am();
	}
	if(band_mode == SW) {
			atdd_power_up(XOSCEN,XOWAIT_LONG,band_index,SW_BAND_BOTTOM,SW_BAND_TOP,SW_BAND_SPACING);
			// the fist power up after reset,wait 100ms
			// for the crystal stable
			if(state_machine == SM_RADIO_RESET) {
					DelayMs(600);
			}
			config_sw();
	}
	//显示波段
	LcdDisplay_char(0, 0, &sBandInfo[((band_index * 2) + 0)][0]);
	LcdDisplay_char(0, 2, &sBandInfo[((band_index * 2) + 1)][0]);
}
/***************************************************
 * config fm band
 * if select host setting mode, user can initial the 
 *      de-emphasis
 *      blend_mono,band_stereo,band_separation
 * *************************************************/
#define ADJPT_STERRO_DISABLE 0
#define FM_ADJPT_ATTENUATION_DISABLE 0
#define FM_AUDIO_MODE 3
#define FM_FORCE_MONO 0
//1:de-emphasis 50us
//2:de-emphasis 75us
#define FM_DE_EMPHASIS 2
#define FM_BLEND_MONO  8
#define FM_BLEND_STEREO 49
#define FM_SEPARATION 0x9F
void config_fm()
{
        if(!flag_tuner_pri) {
                atdd_set_deemphasis(FM_DE_EMPHASIS);
                atdd_set_rssi_blend_mono(FM_BLEND_MONO);
                atdd_set_rssi_blend_stereo(FM_BLEND_STEREO);
                atdd_set_stereo_separation(FM_SEPARATION);
        }
        atdd_audio_mode(ADJPT_STERRO_DISABLE,FM_ADJPT_ATTENUATION_DISABLE,FM_FORCE_MONO,FM_AUDIO_MODE);
#if  FM_AUDIO_MODE != 0
//        atdd_set_bass_treble(fm_bass_treble);
#endif
#if  FM_AUDIO_MODE != 1
 //       atdd_set_volume(volume);
#endif    
}
/***************************************************
 * config am band,initial volume and bass/treble
 * *************************************************/
void config_am()
{
        //atdd_set_bass_treble(am_bass_treble);
        //atdd_set_volume(volume);
}
/***************************************************
 * config sw band,initial volume ,bass/treble and tuning preference
 * a
 * *************************************************/
#define SW_ADJPT_ATTENUATION_DISABLE 0
void config_sw()
{
        //atdd_set_bass_treble(am_bass_treble);
        //atdd_set_volume(volume);
        atdd_audio_mode(0,SW_ADJPT_ATTENUATION_DISABLE,0,0);
}
//-----------------------------------------------------------------------------
// Take the tuner out of powerdown mode.
//-----------------------------------------------------------------------------
void atdd_power_up(U8 xoscen,U8 xowait,U8 band_index,U16 band_bottom,U16 band_top,U8 chspc)
{
	U8 idata cmd[7];
	U8 idata rsp[8];
	U8 len;        
	cmd[0] = ATDD_POWER_UP;  //ATDD powerup command
	cmd[1] = band_index;
	if( xoscen ) {
			cmd[1] |= ATDD_XOSCEN ;   
	}
	if(xowait) {
			cmd[1] |= ATDD_XOWAIT;
	}
	len = 2;
	if(!flag_tuner_pri) {
		cmd[2] = band_bottom>>8;
		cmd[3] = band_bottom;
		cmd[4] = band_top>>8;
		cmd[5] = band_top;
		cmd[6] = chspc;
		len = 7;
	}
	write_command(len, cmd, 1, rsp);
}

/*******************************************************************************
 * This function set the audio mode,fm force mono,+/-100K-adjance channel attenuation mode
 * and +/-100k-adjance channel stereo status
 * *****************************************************************************/
void atdd_audio_mode(U8 adj_st,U8 adj_attn,U8 force_mono,U8 audio_mode)
{
        U8 idata cmd[2];
        cmd[0] = ATDD_AUDIO_MODE;
        cmd[1] = audio_mode;
        if(adj_st) {
                cmd[1] |= ADJPT_STEO;
        }
        if(adj_attn) {
                cmd[1] |= ADJPT_ATTN;
        }
        if(force_mono) {
                cmd[1] |= FM_MONO;
        }
        write_command(2, cmd, 0, NULL);
}
/******************************************************************************
 * get atdd status,such as host reset requied,host power up required
 * bandindex/frequency info valid bit,stereo status,station indicator
 * ***************************************************************************/
void atdd_get_status(U8 idata * rspbuf)
{
        U8 idata cmd;
        //get atdd status and analyze them 
        cmd = ATDD_GET_STATUS;
        write_command(1,&cmd,4,rspbuf); 
}
/****************************************************************************
 * this function set the volume 
 * **************************************************************************/
void atdd_set_volume(U8 volume)
{
        set_property(0x4000,volume);
}
/****************************************************************************
 * this function get the actual volume
 * at the digital volume and bass/treble mix2 mode, the max digital volume is
 * dynamic changed depond on the bass/treble value
 *      the maxVolume = 63 - abs(tone - 4);
 * user could get the currren actual volume through this function
 * **************************************************************************/
#if FM_AUDIO_MODE == 3
#ifndef enable_get_actual_vol_command
#define enable_get_actual_vol_command
#endif
#endif

#ifdef  enable_get_actual_vol_command
#ifndef enable_get_property_command
#define enable_get_property_command
#endif
U8 atdd_get_actual_vol()
{
	U8 actual_vol;
	actual_vol = (U8) get_property(0x4003);
	return actual_vol;
}
#endif
/****************************************************************************
 * this function set the bass/treble
 * **************************************************************************/
void atdd_set_bass_treble(U8 tone)
{
	set_property(0x4002,tone);
	#if FM_AUDIO_MODE == 3
	if(band_mode == FM) 
	{
		volume = atdd_get_actual_vol();
		max_vol = max_vol_list[tone];
	}
	#endif
}
/****************************************************************************
 * this function set fm deemphasis,10B--> 75us,01B-->50us
 * **************************************************************************/
void atdd_set_deemphasis(U8 deemphasis)
{
	set_property(0x1100,deemphasis);
}
/****************************************************************************
 * this function set fm softmute rate.
 * **************************************************************************/
void atdd_set_fm_softmute_rate(U8 rate)
{
	set_property(0x1300,rate);
}
/****************************************************************************
 * this function set fm softmute slope.
 * **************************************************************************/
void atdd_set_fm_softmute_slope(U8 slope)
{
	set_property(0x1301,slope);
}
/****************************************************************************
 * this function set fm softmute max attenuation
 * **************************************************************************/
void atdd_set_fm_softmute_max_attenuation(U8 max_attn)
{
	set_property(0x1302,max_attn);
}
/****************************************************************************
 * this function set fm softmute snr threshold
 * **************************************************************************/
void atdd_set_fm_softmute_snr(U8 snr)
{
	set_property(0x1303,snr);
}
/****************************************************************************
 * this function set am softmute rate.
 * **************************************************************************/
void atdd_set_am_softmute_rate(U8 rate)
{
	set_property(0x3300,rate);
}
/****************************************************************************
 * this function set am softmute slope.
 * **************************************************************************/
void atdd_set_am_softmute_slope(U8 slope)
{
	set_property(0x3301,slope);
}
/****************************************************************************
 * this function set am softmute max attenuation
 * **************************************************************************/
void atdd_set_am_softmute_max_attenuation(U8 max_attn)
{
	set_property(0x3302,max_attn);
}
/****************************************************************************
 * this function set am softmute snr threshold
 * **************************************************************************/
void atdd_set_am_softmute_snr(U8 snr)
{
	set_property(0x3303,snr);
}
/****************************************************************************
 * this function set fm stereo blend threshold for stereo indicator set to 1
 * if user have a stereo icon to indicator the stereo signal,user could use 
 *      atdd_set_rssi_blend_mono() to set when start to get separation
 *      atdd_set_rssi_blend_stereo() to set when get the max separation
 *      atdd-set_separation() to set when set the stereo indicator
 *              RSSI_indicator_on = blend_mono + 
 *                      (blend_stereo-blend_mono) * 
 *                      (separation - 128);
 * **************************************************************************/
void atdd_set_stereo_separation(U8 separation)
{
        set_property(0x1207,separation);
}
/****************************************************************************
 * if user have a stereo icon to indicator the stereo signal,user could use 
 *      atdd_set_rssi_blend_mono() to set when start to get separation
 *      atdd_set_rssi_blend_stereo() to set when get the max separation
 *      atdd-set_separation() to set when set the stereo indicator
 *              RSSI_indicator_on = blend_mono + 
 *                      (blend_stereo-blend_mono) * 
 *                      (separation - 128);
 * **************************************************************************/
void atdd_set_rssi_blend_mono(U8 blend_mono)
{
        set_property(0x1801,blend_mono);
}
/****************************************************************************
 * if user have a stereo icon to indicator the stereo signal,user could use 
 *      atdd_set_rssi_blend_mono() to set when start to get separation
 *      atdd_set_rssi_blend_stereo() to set when get the max separation
 *      atdd-set_separation() to set when set the stereo indicator
 *              RSSI_indicator_on = blend_mono + 
 *                      (blend_stereo-blend_mono) * 
 *                      (separation - 128);
 * **************************************************************************/
void atdd_set_rssi_blend_stereo(U8 blend_stereo)
{
	set_property(0x1800,blend_stereo);
}
//-----------------------------------------------------------------------------
// Set the passed property number to the passed value.
//
// Inputs:
//      propNumber:  The number identifying the property to set
//      propValue:   The value of the property.
//-----------------------------------------------------------------------------
void set_property(U16 property_id,U16 property_value)
{
        U8 idata cmd[6];
        cmd[0] = 0x12;
        cmd[1] = 0;
        cmd[2] = (U8)(property_id >> 8);
        cmd[3] = (U8)(property_id);
        cmd[4] = (U8)(property_value >> 8);
        cmd[5] = (U8)(property_value);
        write_command(6, cmd, 0, NULL);
}
U16 get_property(U16 property_id)
{
        U8 idata cmd[4];
        U8 idata rsp[4];
        cmd[0] = 0x13;
        cmd[1] = 0;
        cmd[2] = (U8)(property_id >> 8);
        cmd[3] = (U8)(property_id);
        write_command(4, cmd,4, rsp);
        return (U16)(rsp[2]<<8+rsp[3]);
}
void atdd_get_version()
{//this function get the chip version from the tuner chip
	U8 idata cmd[1];
	U8 idata rsp[9];
	cmd[0] = 0x10;
	write_command(1, cmd, 9, rsp);
}
void atdd_power_down(void)
{//take the tuner into the power down mode
	U8 idata cmd;
	cmd = ATDD_POWER_DOWN;
	write_command(1,&cmd,0,NULL);
}
void wait_cts(void)
{// Command that will wait for CTS before returning
	U8 idata status;
	U8 i=10;
	do 
	{
		DelayUs(100);
		si4844_i2c_read_buf(1,&status); 
	} while (!(status & 0x80) && i--);
}
void write_command(U8 cmd_size,U8 idata *cmd_buf,U8 reply_size,U8 idata *reply)
{
	// It is always a good idea to check for cts prior to sending a command to the part.
	// Write the command to the part
	si4844_i2c_write_buf(cmd_size, cmd_buf);
	wait_cts();
	if(reply_size) 
	{// If the calling function would like to have results then read them.
		si4844_i2c_read_buf(reply_size, reply);
	}
}

void FM_Receiver(void)
{
	//Power up in Analog Mode
	//CONFIG CMD 0xE2
	//CONFIG_CMD 0x12
	//CONFIG_CMD 
}
