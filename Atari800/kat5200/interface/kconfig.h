/******************************************************************************
*
* FILENAME: kconfig.h
*
* DESCRIPTION:  This contains function and struct declartions for configuration 
*               functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     06/18/05  bberlin      Creation
* 0.2     03/09/06  bberlin      Added more configuration items and functions
* 0.2.2   03/15/06  bberlin      Added deadzone parameter to t_atari_stick
* 0.2.3   03/17/06  bberlin      Added sound_freq and sound_samples parameters
*                                  to t_config.
* 0.3.0   04/01/06  bberlin      Added sensitivity parameter to t_atari_stick
* 0.3.0   04/05/06  bberlin      Added simulate_analog parameter to 
*                                  t_atari_stick
* 0.4.0   05/21/06  bberlin      Added ui_keys parameter to configuration
* 0.4.0   06/04/06  bberlin      Added hires_artifacts parameter to configuration
******************************************************************************/
#ifndef config_h
#define config_h

#include "../core/console.h"
#include "input.h"

#define STATE_LOAD_NONE 0
#define STATE_LOAD_FILE 10

struct generic_node {
	char name[1024];
	unsigned long  value;
	unsigned long  data;
	struct generic_node *next;
};

typedef struct {
	char name[257];
	char description[257];
	char os_file_800[1024];
	char os_file_5200[1024];
	char os_file_xl[1024];
	char os_file_basic[1024];
	char font_file[1024];
	char state[257];
	char rom_file[1100];
	char default_input_profile[257];
	char default_800input_profile[257];
	char default_video_profile[257];
	char default_sound_profile[257];
	char default_ui_profile[257];
	int throttle;
	e_machine_type machine_type;
	int ram_size[4];
	int system_type;
	int sio_patch;
	int basic_enable;
	int exit_status;
	int first_use;
	int start_with_gui;
	int start_gui_with_launcher;
	int launcher_show_all;
	int launcher_tabbed_view;
	int launcher_group_view;
} t_config;

t_config * config_get_ptr ( void );
int config_init (  int argc, char *argv[] );
void config_set_internal_defaults ( void );
int config_update ( void );
int config_load ( );
int config_save ( );
int config_get_exit_status ( void );
int config_set_exit_status ( void );
int config_get_first_use ( void );
char * config_get_os_file ( void );
int config_delete_group ( t_config *p_config, char *node_name );

#endif
