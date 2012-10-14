
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define uint8  unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define int8  signed char
#define int16 signed short
#define int32 signed int
/****************************************************************************
 * Config Option 
 *
 ****************************************************************************/
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
	uint8 region_detect;
	uint8 force_dtack;
	uint8 addr_error;
	uint8 tmss;
	uint8 lock_on;
	uint8 hot_swap;
	uint8 romtype;
	uint8 invert_mouse;
	uint8 gun_cursor[2];
	uint8 overscan;
    uint8 gg_extra;
	uint8 ntsc;
	uint8 render;
	uint8 tv_mode;
	uint8 bilinear;
	uint8 aspect;
	int16 xshift;
	int16 yshift;
	int16 xscale;
	int16 yscale;
    uint8 system;
    uint8 bios;
    uint8 master_clock;
    uint8 vdp_mode;
#ifdef HW_RVL
	uint32 trap;
	float gamma;
#endif
	t_input_config input[MAX_INPUTS];
	uint16 pad_keymap[4][MAX_KEYS];
#ifdef HW_RVL
	uint32 wpad_keymap[4*3][MAX_KEYS];
#endif
	uint8 autoload;
	uint8 autocheat;
	uint8 s_auto;
	uint8 s_default;
	uint8 s_device;
	uint8 autocheats;
	int8 bg_type;
	int8 bg_overlay;
	int16 screen_w;
	float bgm_volume;
	float sfx_volume;
} t_config;

/* Global variables */
extern t_config config;

#endif /* _CONFIG_H_ */

