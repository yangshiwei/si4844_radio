/*************************************************************************
 * this file include the functions of write or read command
 * void atdd_power_up(U8 band_select_mode,U8 band_index)
 * void atdd_power_down(void)
 * void atdd_set_property(u16 property_id,u16 property_value);
 * u16 atdd_get_property(u16 property_id);
 * void atdd_wait_cts(void);
 * void atdd_command(U8 cmd_size,U8 idata *cmd_buf,U8 replay_size,U8 idata *reply);
 * ***************************************************************************/
#include "si4844_clockradio.h"
#include "si4844_i2c.h"
/*****************************************************************************/
#define FM      0
#define AM      1
#define SW      2

#define ATDD_POWER_DOWN 0x11

#define ATDD_POWER_UP   0xE1
#define ATDD_XOSCEN     0x80
#define ATDD_XOWAIT     0x40
//1--> external 32768Hz crystal;0--> RCLK
#define XOSCEN          1
//0--> wait 600ms for crystal oscillator setup, for normal crystal
//1--> wait 850ms for crystal oscillator setup, for worse crystal which the setup time is long
#define XOWAIT_LONG     0

#define ATDD_GET_STATUS 0xE0

#define ATDD_AUDIO_MODE 0xE2
#define ADJPT_STEO      0x10
#define ADJPT_ATTN      0x08
#define FM_MONO         0x04

#define HOSTRST         0x40
#define HOSTPWRUP       0x20
#define INFORDY         0x10
#define STATION         0x08
#define STEREO          0x04
#define TUNERPRI        0x02
#define HOSTBAND        0x01

//define the state machine constant
#define	SM_RADIO_READY 0x80
#define	SM_RADIO_RESET 0x81
#define	SM_RADIO_POWERUP 0x82

/*********************************************************************
 * this function initial the tuner after powerup command
 * *******************************************************************/
#define FM_BAND_TOP     10900
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
#define FM_FORCE_MONO 0
//1:de-emphasis 50us
//2:de-emphasis 75us
#define FM_DE_EMPHASIS 2
#define FM_BLEND_MONO  8
#define FM_BLEND_STEREO 49
#define FM_SEPARATION 0x9F
/*******************************************************************
 * define some global variable
 * ***************************************************************/

U8 state_machine;
U8 band_index;
U8 band_mode;
U8 flag_tuner_irq;
U8 flag_station;
U8 flag_stereo;
U8 freq_bcd[2];
const U8 code max_vol_list[9] = {59,60,61,62,63,62,61,60,59};
U8 max_vol;
bit flag_host_band;
bit flag_tuner_pri;
U8 fm_bass_treble;
U8 am_bass_treble;
U8 vol;
/*****************************************************************************/
void isr_irq() interrupt 0
{//this isr set a flag, but get atdd status in main loop
        if(state_machine & SM_RADIO_READY) {
                flag_tuner_irq = 1;
        }
}
/*****************************************************************************/
/**********************************************************************************
 * this function enter radio mode or exit radio
 * *******************************************************************************/
void switch_power()
{
        if(state_machine & SM_RADIO_READY) {
                state_machine = SM_POWER_OFF;
                i2c_reset_disable();
        } else {
                state_machine = SM_RADIO_RESET;
                i2c_reset_enable();
        }
}
/**********************************************************************************
 * this function switch band
 * *******************************************************************************/
void adjust_band(void)
{
        if(state_machine & SM_RADIO_READY) {
                band_index++;
                if(band_index > 40) {
                       band_index = 0;
                } 
                i2c_reset();
                state_machine = SM_RADIO_RESET;        
        }        
}
/**********************************************************************************
 * this function adjust volume
 * *******************************************************************************/
void adjust_volume(U8 direction)
{
        if(state_machine & SM_RADIO_READY) {
                if (direction == 0) {
                        if(volume) {
                                volume--;
                        }
                } else {
                        if(volume < 63) {
                                volume++;
                        }
                }
                si48xx_set_volume(volume);
                state_machine = SM_RADIO_VOLUME;     
        }
}
/***************************************************************
 * this function adjust bass treble
 * ************************************************************/
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
/************************************************************************
 * this function parse the atdd status and process the tuner requirement
 * **********************************************************************/
void parse_atdd_status()
{
        U8 idata atdd_status[4];     
		if( flag_tuner_irq ) {
			flag_tuner_irq = 0;
                atdd_get_status(atdd_status);
                // STATUS.4 1--> BAND INDEX AND FREQUENCY INFO IS READY
                if(atdd_status[0] & INFORDY) {
                        flag_host_band = atdd_status[0] & HOSTBAND;
                        flag_tuner_pri = atdd_status[0] & TUNERPRI;
                        //check who detect the band,Host mcu or tuner itself
                        //atdd_status[0].1 0--> tuner; 1--> host mcu
                        if(!flag_host_band) {
                                // if tuner detect the band, the current will be report to 
                                // host through att_get_status command,and local at response 1
                                // host need to memorize the band index reported by tuner and send
                                // it back to tuner later 
                                band_index = atdd_status[1] & 0x3F;
                                band_mode = atdd_status[1] >> 6;
                        }
                        //after the power status and the info ready again,enter ready status
                        if(state_machine == SM_RADIO_POWERUP ) {
                                state_machine = SM_RADIO_READY;
                        }
                        //extract the frequency and indicator information 
                        if(state_machine == SM_RADIO_READY) {
                                //after enter the radio ready state,start to display frequency and indicators
                                flag_station = 0;
                                if(atdd_status[0] & STATION) {
                                        flag_station = 1;
                                }
                                flag_stereo = 0;
                                if(atdd_status[0] & STEREO) {
                                        flag_stereo = 1;
                                }
                                freq_bcd[0] = atdd_status[1];
                                freq_bcd[0] = atdd_status[2];
                        }
                } else {
                    band_index = 0;
                }
                // status.6 1--> need reset the tune
                if(atdd_status[0] & HOSTRST) {
                        i2c_reset();
                        state_machine = SM_RADIO_RESET;
                }
                // STATUS.5 1--> need power up command
                if(atdd_status[0] & HOSTPWRUP) {
                        // powerup and config the systom .
                        config_tune();
                        state_machine = SM_RADIO_POWERUP;
                }
        }
}

void config_tune(void)
{
        if( band_mode == FM) {
                atdd_power_up(XOSCEN,XOWAIT_LONG,band_index,FM_BAND_BOTTOM,FM_BAND_TOP,FM_BAND_SPACING);
                // the fist power up after reset,wait 100ms
                // for the crystal stable
                if(state_machine == SM_RADIO_RESET) {
                        wait_ms(100);
                }
                config_fm();
        }
        if(band_mode == AM) {
                atdd_power_up(XOSCEN,XOWAIT_LONG,band_index,AM_BAND_BOTTOM,AM_BAND_TOP,AM_BAND_SPACING);
                // the fist power up after reset,wait 100ms
                // for the crystal stable
                if(state_machine == SM_RADIO_RESET) {
                        wait_ms(100);
                }
                config_am();
        }
        if(band_mode == SW) {
                atdd_power_up(XOSCEN,XOWAIT_LONG,band_index,SW_BAND_BOTTOM,SW_BAND_TOP,SW_BAND_SPACING);
                // the fist power up after reset,wait 100ms
                // for the crystal stable
                if(state_machine == SM_RADIO_RESET) {
                        wait_ms(100);
                }
                config_sw();
        }
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
        atdd_set_bass_treble(fm_bass_treble);
#endif
#if  FM_AUDIO_MODE != 1
        atdd_set_volume(vol);
#endif    
}
/***************************************************
 * config am band,initial volume and bass/treble
 * *************************************************/
void config_am()
{
        atdd_set_bass_treble(am_bass_treble);
        atdd_set_volume(vol);
}
/***************************************************
 * config sw band,initial volume ,bass/treble and tuning preference
 * a
 * *************************************************/
#define SW_ADJPT_ATTENUATION_DISABLE 0
void config_sw()
{
        atdd_set_bass_treble(am_bass_treble);
        atdd_set_volume(vol);
        atdd_audio_mode(0,SW_ADJPT_ATTENUATION_DISABLE,0,0);
}
//-----------------------------------------------------------------------------
// Take the tuner out of powerdown mode.
//-----------------------------------------------------------------------------
void atdd_power_up(U8 xoscen,U8 xowait,U8 band_index,u16 band_bottom,u16 band_top,U8 chspc)
{
	U8 idata cmd[7];   
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

        write_command(len, cmd, 0, NULL);
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
        if(band_mode == FM) {
                vol = atdd_get_actual_vol();
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
void set_property(u16 property_id,u16 property_value)
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
#ifdef  enable_get_property_command
u16 get_property(u16 property_id)
{
        U8 idata cmd[4];
        U8 idata rsp[4];
        cmd[0] = 0x13;
        cmd[1] = 0;
        cmd[2] = (U8)(property_id >> 8);
        cmd[3] = (U8)(property_id);
        write_command(4, cmd,4, rsp);
        return (u16)(rsp[2]<<8+rsp[3]);
}
#endif
/*******************************************************************************
 * this function get the chip version from the tuner chip
 * *****************************************************************************/
#ifdef  enable_get_version_command
void atdd_get_version()
{
        U8 idata cmd[1];
        U8 idata rsp[9];
        cmd[0] = 0x10;
        write_command(1, cmd, 9, rsp);
}
#endif
/*******************************************************************************
 * take the tuner into the power down mode
 * *****************************************************************************/
#ifdef  enable_power_down_command
void atdd_power_down(void)
{
        U8 idata cmd;
        cmd = ATDD_POWER_DOWN;
        write_command(1,&cmd,0,NULL);
}
#endif
//-----------------------------------------------------------------------------
// Command that will wait for CTS before returning
//-----------------------------------------------------------------------------
void wait_cts(void)
{
        U8 idata status;
        U8 i=10;
        do {
                wait_us(100);
                i2c_read_buf(1,&status); 
        } while (!(status & 0x80) && i--);
}
void write_command(U8 cmd_size,U8 idata *cmd_buf,U8 reply_size,U8 idata *reply)
{
        // It is always a good idea to check for cts prior to sending a command to the part.
        // Write the command to the part
        i2c_write_buf(cmd_size, cmd_buf);
        wait_cts();
        // If the calling function would like to have results then read them.
        if(reply_size) {
                i2c_read_buf(reply_size, reply);
        }
}

