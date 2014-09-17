#ifndef __SI4844_CLOCKRADIO_H__
#define __SI4844_CLOCKRADIO_H__
/*****************************************************************************/
#include "compiler_defs.h"
#include "si4844_i2c.h"
/*****************************************************************************/
void wait_cts(void);
void write_command(U8 cmd_size,U8 idata *cmd_buf,U8 replay_size,U8 idata *reply);
void atdd_power_up(U8 xoscen,U8 xowait,U8 band_index,U16 band_bottom,U16 band_top,U8 chspc);
void atdd_power_down(void);
void atdd_get_status(U8 idata * rspbuf);
void config_tune(void);
void config_fm();
void config_am();
void config_sw();
void atdd_audio_mode(U8 adj_st,U8 adj_attn,U8 force_mono,U8 audio_mode);
void atdd_set_volume(U8 volume);
U8 atdd_get_actual_vol();
void atdd_set_bass_treble(U8 tone);
void atdd_set_deemphasis(U8 deemphasis);
void atdd_set_fm_softmute_rate(U8 rate);
void atdd_set_fm_softmute_slope(U8 slope);
void atdd_set_fm_softmute_max_attenuation(U8 max_attn);
void atdd_set_fm_softmute_snr(U8 snr);
void atdd_set_am_softmute_rate(U8 rate);
void atdd_set_am_softmute_slope(U8 slope);
void atdd_set_am_softmute_max_attenuation(U8 max_attn);
void atdd_set_am_softmute_snr(U8 snr);
void atdd_set_stereo_separation(U8 separation);
void atdd_set_rssi_blend_mono(U8 blend_mono);
void atdd_set_rssi_blend_stereo(U8 blend_stereo);
void set_property(U16 property_id,U16 property_value);
U16 get_property(U16 property_id);
void atdd_get_version();
void parse_atdd_status();

#endif
