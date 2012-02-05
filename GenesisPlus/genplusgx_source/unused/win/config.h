
#ifndef _CONFIG_H_
#define _CONFIG_H_

/****************************************************************************
 * Config Option 
 *
 ****************************************************************************/
typedef struct 
{
  uint8 padtype;
} t_input_config;

typedef struct 
{
  uint8 hq_fm;
  uint8 filter;
  uint8 psgBoostNoise;
  uint8 dac_bits;
  int16 psg_preamp;
  int16 fm_preamp;
  int16 lp_range;
  int16 low_freq;
  int16 high_freq;
  int16 lg;
  int16 mg;
  int16 hg;
  float rolloff;
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
  uint8 ntsc;
  uint8 render;
  t_input_config input[MAX_INPUTS];
} t_config;

/* Global variables */
extern t_config config;
extern void set_config_defaults(void);

#endif /* _CONFIG_H_ */

