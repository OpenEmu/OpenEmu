
#ifndef _CONFIG_H_
#define _CONFIG_H_

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
	uint8 psgBoostNoise;
	int32 psg_preamp;
	int32 fm_preamp;
	uint8 filter;
	float lg;
	float mg;
	float hg;
	uint8 region_detect;
	uint8 force_dtack;
	uint8 addr_error;
	uint8 bios_enabled;
	int16 xshift;
	int16 yshift;
	int16 xscale;
	int16 yscale;
	uint8 tv_mode;
	uint8 aspect;
	uint8 overscan;
	uint8 render;
	uint8 ntsc;
	uint8 bilinear;
	uint8 gun_cursor[2];
	uint8 invert_mouse;
	uint16 pad_keymap[4][MAX_KEYS];
	uint32 wpad_keymap[4*3][MAX_KEYS];
	t_input_config input[MAX_INPUTS];
	int8 sram_auto;
	int8 state_auto;
	int8 bg_color;
	int16 screen_w;
	uint8 ask_confirm;
	float bgm_volume;
	float sfx_volume;
} t_config;

/* Global variables */
extern t_config config;

#endif /* _CONFIG_H_ */

