#ifndef _OSD_H
#define _OSD_H

#ifdef _MSC_VER
#include <stdio.h>
typedef unsigned char bool;
#define strncasecmp _strnicmp
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUTS 8
#define MAX_KEYS 8
#define MAXPATHLEN 1024

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932385
#endif

//#define ALIGN_SND 0xfffffff8  /* 32 bytes aligned sound buffers (8 samples alignment) */

typedef struct 
{
    int8 device;
    uint8 port;
    uint8 padtype;
} t_input_config;

typedef struct 
{
    char version[16];
    uint8 hq_fm;
    uint8 filter;
    uint8 psgBoostNoise;
    uint8 dac_bits;
    uint8 ym2413;
    int16 psg_preamp;
    int16 fm_preamp;
    int16 lp_range;
    int16 low_freq;
    int16 high_freq;
    int16 lg;
    int16 mg;
    int16 hg;
    float rolloff;
    uint8 system;
    uint8 region_detect;
    uint8 master_clock;
    uint8 vdp_mode;
    uint8 force_dtack;
    uint8 addr_error;
    uint8 tmss;
    uint8 bios;
    uint8 lock_on;
    uint8 hot_swap;
    uint8 invert_mouse;
    uint8 gun_cursor[2];
    uint8 overscan;
    uint8 ntsc;
    uint8 vsync;
    uint8 render;
    uint8 tv_mode;
    uint8 bilinear;
    uint8 aspect;
    int16 xshift;
    int16 yshift;
    int16 xscale;
    int16 yscale;
    t_input_config input[MAX_INPUTS];
    uint16 pad_keymap[4][MAX_KEYS];
    uint8 autoload;
    uint8 autocheat;
    uint8 s_auto;
    uint8 s_default;
    uint8 s_device;
    uint8 l_device;
    uint8 bg_overlay;
    int16 screen_w;
    float bgm_volume;
    float sfx_volume;
    char lastdir[4][2][MAXPATHLEN];
} t_config;

/* Global data */
t_config config;

extern char GG_ROM[256];
extern char AR_ROM[256];
extern char SK_ROM[256];
extern char SK_UPMEM[256];
extern char GG_BIOS[256];
extern char CD_BIOS_EU[256];
extern char CD_BIOS_US[256];
extern char CD_BIOS_JP[256];
extern char MS_BIOS_US[256];
extern char MS_BIOS_EU[256];
extern char MS_BIOS_JP[256];

extern int16 soundbuffer[2048 * 2]; //3068 1920 ?
//#define SOUND_SAMPLES_SIZE  2048
//n = SOUND_SAMPLES_SIZE * 2 * sizeof(short) * 11;
//sdl_sound.buffer = (char*)malloc(n);

//static short soundframe[SOUND_SAMPLES_SIZE];
//int size = audio_update(soundframe) * 2;

#define VERSION "Genesis Plus GX 1.7.0 (libretro)"

void osd_input_update(void);
int load_archive(char *filename, unsigned char *buffer, int maxsize, char *extension);

#endif /* _OSD_H */
