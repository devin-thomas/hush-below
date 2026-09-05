#include <gba.h>
#include <stddef.h>
#include "audio.h"
#include "sim.h"
#include "video.h"
HbSim hb_state;
const unsigned int hb_debug_offsets[] __attribute__((used))={offsetof(HbSim,screen),offsetof(HbSim,dive),offsetof(HbSim,collected_mask),offsetof(HbSim,collected_total),offsetof(HbSim,failed),offsetof(HbSim,ping_radius),offsetof(HbSim,oxygen),offsetof(HbSim,x),offsetof(HbSim,y),offsetof(HbSim,collisions),offsetof(HbSim,pings)};
int main(void){uint16_t prior=0;irqInit();irqEnable(IRQ_VBLANK);hb_init(&hb_state);hb_video_init();hb_audio_init();while(1){uint8_t c=hb_state.collected_total,h=hb_state.collisions;VBlankIntrWait();scanKeys();uint16_t held=keysHeld(),pressed=held&~prior;prior=held;hb_update(&hb_state,held,pressed);if(hb_state.ping_radius==1)hb_audio_ping();if(hb_state.collected_total!=c)hb_audio_collect();if(hb_state.collisions!=h)hb_audio_hit();hb_video_render(&hb_state);}}
