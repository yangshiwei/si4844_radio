#ifndef __SI4844_CLOCKRADIO_H__
#define __SI4844_CLOCKRADIO_H__
/*****************************************************************************/
#include "compiler_defs.h"
#include "C8051F340_defs.h"
#include "si4844_i2c.h"
/*****************************************************************************/
void wait_cts(void);
void write_command(u8 cmd_size,u8 idata *cmd_buf,u8 replay_size,u8 idata *reply);
void atdd_power_up(u8 xoscen,u8 xowait,u8 band_index,u16 band_bottom,u16 band_top,u8 chspc);
void atdd_power_down(void);

void atdd_get_status(u8 idata * rspbuf);
void config_tune(void);
void config_fm();
void config_am();
void config_sw();
void atdd_audio_mode(u8 adj_st,u8 adj_attn,u8 force_mono,u8 audio_mode);
void atdd_set_volume(u8 volume);
u8 atdd_get_actual_vol();
void atdd_set_bass_treble(u8 tone);
void atdd_set_deemphasis(u8 deemphasis);
void atdd_set_fm_softmute_rate(u8 rate);
void atdd_set_fm_softmute_slope(u8 slope);
void atdd_set_fm_softmute_max_attenuation(u8 max_attn);
void atdd_set_fm_softmute_snr(u8 snr);
void atdd_set_am_softmute_rate(u8 rate);
void atdd_set_am_softmute_slope(u8 slope);
void atdd_set_am_softmute_max_attenuation(u8 max_attn);
void atdd_set_am_softmute_snr(u8 snr);
void atdd_set_stereo_separation(u8 separation);
void atdd_set_rssi_blend_mono(u8 blend_mono);
void atdd_set_rssi_blend_stereo(u8 blend_stereo);
void set_property(u16 property_id,u16 property_value);
u16 get_property(u16 property_id);
void atdd_get_version();
void parse_atdd_status();

#endif
