
#include "shared.h"

t_config config;


void set_config_defaults(void)
{
    int i;
    
	/* sound options */
	config.psg_preamp     = 150;
	config.fm_preamp      = 100;
	config.hq_fm          = 1;
	config.psgBoostNoise  = 0;
	config.filter         = 1;
	config.lp_range       = 60;
	config.low_freq       = 200;
	config.high_freq      = 8000;
	config.lg             = 1.0;
	config.mg             = 1.0;
	config.hg             = 1.0;
	config.dac_bits       = 14;
    config.ym2413         = 2;
	
	/* system options */
	config.region_detect  = 0;
	config.force_dtack    = 0;
	config.addr_error     = 1;
	//config.tmss           = 0;
	config.lock_on        = 0;
	//config.romtype        = 0;
	config.hot_swap       = 0;
    config.bios           = 0;
    config.system         = 0;
    config.master_clock   = 0;
    config.vdp_mode       = 0;
	
	/* video options */
	config.xshift   = 0;
	config.yshift   = 0;
	config.xscale   = 0;
	config.yscale   = 0;
	config.aspect   = 1;
	config.overscan = 1;
    config.gg_extra = 0;
	config.render = 0;
	config.ntsc     = 0;
	config.bilinear = 0;
	
	/* controllers options */
	input.system[0]       = SYSTEM_MD_GAMEPAD;
	input.system[1]       = SYSTEM_MD_GAMEPAD;
	config.gun_cursor[0]  = 1;
	config.gun_cursor[1]  = 1;
	config.invert_mouse   = 0;
    for (i=0;i<MAX_INPUTS;i++)
    {
        config.input[i].padtype = DEVICE_PAD6B;
    }
}
