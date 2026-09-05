#include "sim.h"
#include <string.h>
#define HB_OXYGEN_MAX 4500
const uint8_t hb_specimen_x[HB_DIVE_COUNT][HB_SPECIMENS]={{25,14,4},{24,5,18},{6,24,14}};
const uint8_t hb_specimen_y[HB_DIVE_COUNT][HB_SPECIMENS]={{4,16,12},{15,5,4},{16,6,14}};
static int ab(int v){return v<0?-v:v;}
int hb_near(int x,int y,int tx,int ty,int range){return ab(x-tx)<=range&&ab(y-ty)<=range;}
int hb_wall(int d,int x,int y){
 if(x<1||x>28||y<3||y>18)return 1;
 if(x==8&&y!=6&&y!=15)return 1;
 if(x==20&&y!=5+d&&y!=14)return 1;
 if(y==10&&x!=4+d*2&&x!=14&&x!=25-d)return 1;
 if(d>0&&x>=12&&x<=14&&y>=4&&y<=7)return 1;
 if(d>1&&x>=23&&x<=26&&y>=11&&y<=13)return 1;
 return 0;
}
static void enemies(HbSim*s){
 static const uint8_t ex[HB_ENEMIES]={6,12,24,17,27,4};
 static const uint8_t ey[HB_ENEMIES]={5,15,6,17,14,12};
 for(int i=0;i<HB_ENEMIES;i++){s->enemies[i].x=ex[i]*8;s->enemies[i].y=ey[i]*8;s->enemies[i].alert=0;}
}
static void begin(HbSim*s){s->x=3*8;s->y=4*8;s->oxygen=HB_OXYGEN_MAX;s->collected_mask=0;s->failed=0;s->ping_radius=0;s->ping_cooldown=0;s->collisions=0;s->pings=0;enemies(s);}
void hb_init(HbSim*s){memset(s,0,sizeof(*s));s->signature=0x48534842;s->screen=HB_TITLE;s->x=24;s->y=32;}
static void start(HbSim*s,int free){uint8_t menu=s->menu;hb_init(s);s->menu=menu;s->free_dive=free;s->screen=HB_BRIEF;}
static void ping(HbSim*s){s->ping_radius=1;s->ping_cooldown=75;s->ping_x=s->x;s->ping_y=s->y;s->pings++;for(int i=0;i<HB_ENEMIES;i++){s->enemies[i].target_x=s->x;s->enemies[i].target_y=s->y;s->enemies[i].alert=150;}}
static void move_player(HbSim*s,uint16_t held){
 int dx=0,dy=0,sp=(held&HB_B)?1:2;if(held&HB_LEFT){dx=-sp;s->facing=1;}if(held&HB_RIGHT){dx=sp;s->facing=2;}if(held&HB_UP){dy=-sp;s->facing=3;}if(held&HB_DOWN){dy=sp;s->facing=0;}
 int nx=s->x+dx,ny=s->y+dy;if(!hb_wall(s->dive,nx/8,s->y/8))s->x=nx;if(!hb_wall(s->dive,s->x/8,ny/8))s->y=ny;
}
static void move_enemies(HbSim*s){
 for(int i=0;i<HB_ENEMIES;i++){HbEnemy*e=&s->enemies[i];if(e->alert){e->alert--;if((s->frame+i)%3==0){int nx=e->x+(e->x<e->target_x?1:e->x>e->target_x?-1:0);int ny=e->y+(e->y<e->target_y?1:e->y>e->target_y?-1:0);if(!hb_wall(s->dive,nx/8,e->y/8))e->x=nx;if(!hb_wall(s->dive,e->x/8,ny/8))e->y=ny;}}
  if(hb_near(s->x,s->y,e->x,e->y,7)){s->collisions++;s->oxygen=s->oxygen>240?s->oxygen-240:0;e->x=(3+i*4)*8;e->y=(5+(i*3)%12)*8;e->alert=0;}}
}
static void collect(HbSim*s){for(int i=0;i<HB_SPECIMENS;i++)if(!(s->collected_mask&(1<<i))&&hb_near(s->x,s->y,hb_specimen_x[s->dive][i]*8,hb_specimen_y[s->dive][i]*8,8)){s->collected_mask|=1<<i;s->collected_total++;}}
void hb_update(HbSim*s,uint16_t held,uint16_t pressed){
 s->frame++;if(s->ping_cooldown)s->ping_cooldown--;if(s->ping_radius){s->ping_radius+=2;if(s->ping_radius>88)s->ping_radius=0;}
 switch(s->screen){
 case HB_TITLE:if((pressed&HB_UP)&&s->menu)s->menu--;if((pressed&HB_DOWN)&&s->menu<2)s->menu++;if(pressed&(HB_A|HB_START)){if(s->menu==0)start(s,0);else if(s->menu==1)start(s,1);else s->screen=HB_HELP;}break;
 case HB_HELP:if(pressed&(HB_A|HB_B|HB_START))s->screen=HB_TITLE;break;
 case HB_BRIEF:if(pressed&(HB_A|HB_START)){begin(s);s->screen=HB_PLAY;}break;
 case HB_PLAY:if(pressed&HB_START){s->screen=HB_PAUSE;break;}if(pressed&HB_SELECT){s->screen=HB_LOG;break;}if((pressed&HB_A)&&!s->ping_cooldown)ping(s);move_player(s,held);move_enemies(s);collect(s);if(!s->free_dive&&s->oxygen)s->oxygen--;if(s->oxygen==0){s->failed=1;s->screen=HB_REPORT;}else if(s->collected_mask==7&&hb_near(s->x,s->y,24,32,9))s->screen=HB_REPORT;break;
 case HB_PAUSE:if(pressed&(HB_START|HB_B))s->screen=HB_PLAY;if(pressed&HB_SELECT)hb_init(s);break;
 case HB_LOG:if(pressed&(HB_SELECT|HB_A|HB_B))s->screen=HB_PLAY;break;
 case HB_REPORT:if(pressed&(HB_A|HB_START)){if(s->failed){s->screen=HB_BRIEF;}else if(s->free_dive){begin(s);s->screen=HB_PLAY;}else if(s->dive+1>=HB_DIVE_COUNT)s->screen=HB_ENDING;else{s->dive++;s->screen=HB_BRIEF;}}break;
 case HB_ENDING:if(pressed&(HB_A|HB_START))hb_init(s);break;default:hb_init(s);break;}
}
