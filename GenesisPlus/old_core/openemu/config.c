
#include "shared.h"

t_config config;


void set_config_defaults(void)
{
	/* sound options */
	config.psg_preamp     = 150;
	config.fm_preamp      = 100;
	config.hq_fm          = 1;
	config.psgBoostNoise  = 0;
	config.filter         = 1;
	config.lg             = 1.0;
	config.mg             = 1.0;
	config.hg             = 1.0;
	
	/* system options */
	config.region_detect  = 0;
	config.force_dtack    = 0;
	config.addr_error     = 1;
	config.bios_enabled   = 0;
	
	/* video options */
	config.xshift   = 0;
	config.yshift   = 0;
	config.xscale   = 0;
	config.yscale   = 0;
	config.aspect   = 1;
	config.overscan = 1;
	config.render = 0;
	config.ntsc     = 0;
	config.bilinear = 1;
	
	/* controllers options */
	input.system[0]       = SYSTEM_GAMEPAD;
	input.system[1]       = SYSTEM_GAMEPAD;
	config.gun_cursor[0]  = 1;
	config.gun_cursor[1]  = 1;
	config.invert_mouse   = 0;
}

