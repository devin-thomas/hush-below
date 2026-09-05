#include <gba.h>
#include "audio.h"
static void tone(unsigned short p,unsigned short v){REG_SOUND1CNT_L=0x0032;REG_SOUND1CNT_H=(2<<6)|(2<<8)|(1<<11)|(v<<12);REG_SOUND1CNT_X=0x8000|(p&0x7ff);}
void hb_audio_init(void){REG_SOUNDCNT_X=0x80;REG_SOUNDCNT_L=SND1_R_ENABLE|SND1_L_ENABLE|SND4_R_ENABLE|SND4_L_ENABLE|0x77;REG_SOUNDCNT_H=2;}
void hb_audio_ping(void){tone(1500,12);}void hb_audio_collect(void){tone(1940,15);}void hb_audio_hit(void){REG_SOUND4CNT_L=(3<<8)|(10<<12);REG_SOUND4CNT_H=0xC04F;}
