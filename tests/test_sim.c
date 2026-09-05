#include <assert.h>
#include <stdio.h>
#include "sim.h"
static void press(HbSim*s,uint16_t k){hb_update(s,k,k);hb_update(s,0,0);}
static void finish_dive(HbSim*s){for(int i=0;i<HB_SPECIMENS;i++){s->x=hb_specimen_x[s->dive][i]*8;s->y=hb_specimen_y[s->dive][i]*8;hb_update(s,0,0);}assert(s->collected_mask==7);s->x=24;s->y=32;hb_update(s,0,0);assert(s->screen==HB_REPORT);}
int main(void){HbSim s;hb_init(&s);assert(s.screen==HB_TITLE);press(&s,HB_A);assert(s.screen==HB_BRIEF);for(int d=0;d<HB_DIVE_COUNT;d++){press(&s,HB_A);assert(s.screen==HB_PLAY);press(&s,HB_A);assert(s.pings==1&&s.ping_radius>0);for(int i=0;i<HB_ENEMIES;i++)assert(s.enemies[i].alert>0);finish_dive(&s);press(&s,HB_A);assert(s.screen==(d==HB_DIVE_COUNT-1?HB_ENDING:HB_BRIEF));}assert(s.collected_total==9);hb_init(&s);assert(hb_wall(0,0,5));assert(!hb_wall(0,4,6));puts("simulation tests: PASS");return 0;}
