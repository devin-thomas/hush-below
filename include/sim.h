#ifndef HUSH_BELOW_SIM_H
#define HUSH_BELOW_SIM_H
#include <stdint.h>
#define HB_DIVE_COUNT 3
#define HB_SPECIMENS 3
#define HB_ENEMIES 6
enum HbScreen { HB_TITLE, HB_BRIEF, HB_PLAY, HB_PAUSE, HB_LOG, HB_REPORT, HB_ENDING, HB_HELP };
enum HbKeys { HB_A=1, HB_B=2, HB_SELECT=4, HB_START=8, HB_RIGHT=16, HB_LEFT=32, HB_UP=64, HB_DOWN=128 };
typedef struct { int16_t x,y; int16_t target_x,target_y; uint8_t alert; } HbEnemy;
typedef struct {
 uint32_t signature; uint8_t screen,dive,menu,free_dive,collected_mask;
 uint8_t collected_total,failed,ping_radius,ping_cooldown,facing;
 uint8_t collisions,pings; uint16_t oxygen,frame; int16_t x,y,ping_x,ping_y;
 HbEnemy enemies[HB_ENEMIES];
} HbSim;
extern const uint8_t hb_specimen_x[HB_DIVE_COUNT][HB_SPECIMENS];
extern const uint8_t hb_specimen_y[HB_DIVE_COUNT][HB_SPECIMENS];
void hb_init(HbSim *s); void hb_update(HbSim *s,uint16_t held,uint16_t pressed);
int hb_wall(int dive,int tx,int ty); int hb_near(int x,int y,int tx,int ty,int range);
#endif
