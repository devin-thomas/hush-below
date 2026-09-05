#include <gba.h>
#include <stdio.h>
#include <string.h>
#include "video.h"
#include "bubbles.h"
#include "cave_tiles.h"
#include "creatures.h"
#include "font.h"
#include "icons.h"
#include "sonar_ring.h"
#include "specimens.h"
#include "submarine.h"
#include "wordmark.h"
#define BG_TILES ((volatile unsigned char*)0x06000000)
#define OBJ_TILES ((volatile unsigned char*)0x06010000)
#define BG_MAP ((volatile uint16_t*)0x0600F800)
#define BG_PAL ((volatile uint16_t*)0x05000000)
#define OBJ_PAL ((volatile uint16_t*)0x05000200)
#define HB_OAM ((volatile uint16_t*)0x07000000)
#define FONT_BASE 64
#define WORD_BASE 112
#define ICON_BASE 160
#define O_SUB 0
#define O_CREATURE 64
#define O_SPEC 96
#define O_SONAR 112
#define O_BUBBLE 176
static uint16_t map[1024] __attribute__((aligned(4))),oam[512] __attribute__((aligned(4)));
static const char glyphs[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$%:!-></ ";
static uint16_t dim(uint16_t c){return ((c&31)>>2)|((((c>>5)&31)>>2)<<5)|((((c>>10)&31)>>2)<<10);}
static void palettes(void){for(int i=0;i<16;i++){BG_PAL[i]=dim(cave_tilesPal[i]);BG_PAL[16+i]=cave_tilesPal[i];BG_PAL[32+i]=fontPal[i];for(int b=0;b<4;b++)OBJ_PAL[b*16+i]=cave_tilesPal[i];}}
static void tile(int x,int y,int t,int p){if(x>=0&&x<30&&y>=0&&y<20)map[y*32+x]=t|(p<<12);}
static void fill(int t){for(int i=0;i<1024;i++)map[i]=t;}
static int glyph(char c){const char*f=strchr(glyphs,c);return f?(int)(f-glyphs):47;}
static void text(int x,int y,const char*s){for(;*s&&x<30;s++,x++)if(*s!=' ')tile(x,y,FONT_BASE+glyph(*s),2);}
static void center(int y,const char*s){text((30-(int)strlen(s))/2,y,s);}
static void number(int x,int y,int n,int w){char b[12];snprintf(b,sizeof b,"%*d",w,n);text(x,y,b);}
static void copy_tile(int d,const unsigned int*s,int st){volatile unsigned int*o=(volatile unsigned int*)(OBJ_TILES+d*32);s+=st*8;for(int i=0;i<8;i++)o[i]=s[i];}
static void frames8(int d,const unsigned int*s,int n){for(int i=0;i<n;i++)copy_tile(d+i,s,i);}
static void frames16(int d,const unsigned int*s,int sw,int cols,int rows){int f=0;for(int y=0;y<rows;y++)for(int x=0;x<cols;x++,f++){int q=y*2*sw+x*2;copy_tile(d+f*4,s,q);copy_tile(d+f*4+1,s,q+1);copy_tile(d+f*4+2,s,q+sw);copy_tile(d+f*4+3,s,q+sw+1);}}
static void frames32(int d,const unsigned int*s,int sw,int cols){for(int f=0;f<cols;f++)for(int y=0;y<4;y++)for(int x=0;x<4;x++)copy_tile(d+f*16+y*4+x,s,f*4+x+y*sw);}
static void load(void){memcpy((void*)BG_TILES,cave_tilesTiles,cave_tilesTilesLen);memcpy((void*)(BG_TILES+FONT_BASE*32),fontTiles,fontTilesLen);memcpy((void*)(BG_TILES+WORD_BASE*32),wordmarkTiles,wordmarkTilesLen);memcpy((void*)(BG_TILES+ICON_BASE*32),iconsTiles,iconsTilesLen);frames16(O_SUB,submarineTiles,8,4,4);frames16(O_CREATURE,creaturesTiles,8,4,2);frames16(O_SPEC,specimensTiles,8,4,1);frames32(O_SONAR,sonar_ringTiles,16,4);frames8(O_BUBBLE,bubblesTiles,8);}
static void hide(void){for(int i=0;i<128;i++){oam[i*4]=160;oam[i*4+1]=oam[i*4+2]=oam[i*4+3]=0;}}
static void sprite(int i,int x,int y,int t,int pal,int size){oam[i*4]=y&255;oam[i*4+1]=(x&511)|(size<<14);oam[i*4+2]=t|(pal<<12);}
static void panel(void){fill(2);for(int x=0;x<30;x++){tile(x,0,3,1);tile(x,19,3,1);}for(int y=0;y<20;y++){tile(0,y,3,1);tile(29,y,3,1);}}
static void wordmark_at(int y){for(int r=0;r<4;r++)for(int x=0;x<12;x++)tile(9+x,y+r,WORD_BASE+r*12+x,2);}
static void title(const HbSim*s){panel();wordmark_at(2);center(7,"EVERY PING HAS A PRICE");center(10,s->menu==0?"> STORY DIVE":"  STORY DIVE");center(12,s->menu==1?"> FREE DIVE":"  FREE DIVE");center(14,s->menu==2?"> SONAR MANUAL":"  SONAR MANUAL");center(17,"A OR START");}
static void brief(const HbSim*s){static const char*n[]={"DIVE 1  THE SHELF","DIVE 2  THE WRECK","DIVE 3  THE ABYSS"};static const char*a[]={"RECOVER THREE SPECIMENS","FIND THREE SIGNAL SHARDS","WAKE THE LOST TRANSMITTER"};panel();center(3,s->free_dive?"FREE DIVE":n[s->dive]);center(7,a[s->dive]);center(10,"PING REVEALS THE CAVE");center(12,"PING ALSO WAKES HUNTERS");center(16,"D PAD MOVE  A PING  B BRAKE");center(18,"PRESS A TO DESCEND");}
static int revealed(const HbSim*s,int x,int y){int dx=x-s->ping_x,dy=y-s->ping_y;int r=s->ping_radius;return r&&dx*dx+dy*dy<=r*r;}
static void play(const HbSim*s){int sp=0;fill(1);for(int y=2;y<20;y++)for(int x=0;x<30;x++){int wall=hb_wall(s->dive,x,y);int t=(wall?16:0)+((x*3+y*5+s->dive*7)&15);int p=(revealed(s,x*8+4,y*8+4)||hb_near(s->x,s->y,x*8,y*8,17))?1:0;tile(x,y,t,p);}for(int x=0;x<30;x++){tile(x,0,1,0);tile(x,1,2,0);}text(0,0,"O2");number(3,0,s->oxygen/60,2);text(7,0,"S");number(9,0,__builtin_popcount(s->collected_mask),1);text(10,0,"/3");text(14,0,s->ping_cooldown?"PING CHARGE":"PING READY");text(0,1,s->collected_mask==7?"RETURN TO BUOY":"A PING  B BRAKE  SELECT LOG");sprite(sp++,16,24,O_SPEC,2,1);
 for(int i=0;i<HB_SPECIMENS;i++)if(!(s->collected_mask&(1<<i))&&revealed(s,hb_specimen_x[s->dive][i]*8,hb_specimen_y[s->dive][i]*8))sprite(sp++,hb_specimen_x[s->dive][i]*8-8,hb_specimen_y[s->dive][i]*8-8,O_SPEC+i*4,2,1);
 for(int i=0;i<HB_ENEMIES;i++){const HbEnemy*e=&s->enemies[i];if(revealed(s,e->x,e->y)||hb_near(s->x,s->y,e->x,e->y,18))sprite(sp++,e->x-8,e->y-8,O_CREATURE+((i&3)+(e->alert?4:0))*4,1,1);}
 sprite(sp++,s->x-8,s->y-8,O_SUB+(s->facing*4+((s->frame>>3)&1))*4,0,1);if(s->ping_radius){int f=s->ping_radius/23;if(f>3)f=3;sprite(sp++,s->x-16,s->y-16,O_SONAR+f*16,3,2);}if((s->frame&7)==0)sprite(sp++,s->x-12,s->y,O_BUBBLE+((s->frame>>3)&7),3,0);
}
static void pause_screen(void){panel();center(6,"ENGINES SILENT");center(10,"START OR B  RESUME");center(13,"SELECT  ABORT DIVE");}
static void log_screen(const HbSim*s){panel();center(2,"DIVE LOG");text(5,6,"SPECIMENS");number(18,6,__builtin_popcount(s->collected_mask),1);text(5,8,"SONAR PINGS");number(18,8,s->pings,2);text(5,10,"COLLISIONS");number(18,10,s->collisions,2);text(5,12,"OXYGEN");number(18,12,s->oxygen/60,2);center(17,"SELECT TO RETURN");}
static void report(const HbSim*s){panel();center(3,s->failed?"DIVE LOST":"DIVE COMPLETE");center(6,s->failed?"THE DARK KEPT ITS SECRET":"THE SEA ANSWERED");text(6,10,"SPEC");number(14,10,__builtin_popcount(s->collected_mask),1);text(17,10,"PING");number(24,10,s->pings,2);text(6,12,"HITS");number(14,12,s->collisions,2);text(17,12,"O2");number(24,12,s->oxygen/60,2);center(17,s->failed?"A TO RETRY":"A FOR NEXT DIVE");}
static void ending(const HbSim*s){panel();wordmark_at(2);center(7,"THE TRANSMITTER WOKE");if(!s->collisions&&s->pings<=6){center(10,"IT LEARNED YOUR SILENCE");center(12,"AND WHISPERED BACK");}else{center(10,"SOMETHING FAR BELOW");center(12,"PINGED BACK");}center(16,"THE ABYSS IS LISTENING");center(18,"A TO SURFACE");}
static void help(void){panel();center(2,"SONAR MANUAL");text(4,6,"D PAD  THRUST");text(4,8,"A      SONAR PING");text(4,10,"B      KILL ENGINES");text(4,12,"SELECT DIVE LOG");center(15,"LIGHT ATTRACTS HUNTERS");center(18,"A TO RETURN");}
void hb_video_init(void){REG_DISPCNT=0x80;memset((void*)BG_TILES,0,0x10000);memset((void*)OBJ_TILES,0,0x8000);load();palettes();hide();fill(1);dmaCopy(map,(void*)BG_MAP,sizeof map);dmaCopy(oam,(void*)HB_OAM,sizeof oam);REG_BG0CNT=2|(31<<8);REG_DISPCNT=MODE_0|BG0_ON|OBJ_ON|OBJ_1D_MAP;}
void hb_video_render(const HbSim*s){hide();switch(s->screen){case HB_TITLE:title(s);break;case HB_BRIEF:brief(s);break;case HB_PLAY:play(s);break;case HB_PAUSE:pause_screen();break;case HB_LOG:log_screen(s);break;case HB_REPORT:report(s);break;case HB_ENDING:ending(s);break;case HB_HELP:help();break;default:title(s);}dmaCopy(map,(void*)BG_MAP,sizeof map);dmaCopy(oam,(void*)HB_OAM,sizeof oam);}
