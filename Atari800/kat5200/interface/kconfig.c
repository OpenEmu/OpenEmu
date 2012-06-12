/******************************************************************************
*
* FILENAME: kconfig.c
*
* DESCRIPTION:  This reads from the config file kat5200.xml
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     06/18/05  bberlin      Creation
* 0.2     01/23/06  bberlin      Change to XML format
* 0.2     03/09/06  bberlin      Added functions for saving, deleting, and 
*                                  finding configuration item for the GUI
* 0.2.2   03/15/06  bberlin      Added deadzone parameter to xml configuration
* 0.2.3   03/17/06  bberlin      Added sound freq and sound samples to config
* 0.2.3   03/19/06  bberlin      Changed 'config_save' to add parameters not
*                                  found (i.e. new parameters)
* 0.3.0   04/01/06  bberlin      Added sensitivity parameter to xml configuration
* 0.3.0   04/05/06  bberlin      Added simulate_analog parameter to xml 
*                                  configuration
* 0.3.0   04/07/06  bberlin      Fix 'config_save_cart_info' not saving in
*                                  alphabetical order
* 0.3.0   04/08/06  bberlin      Added 'config_update' function to handle
*                                  updates to cart list and config file
* 0.4.0   05/21/06  bberlin      Added ui_keys parameter to configuration
* 0.4.0   05/21/06  bberlin      Added Hangly Man and Pac-Man Plus to
*                                  'config_update'
* 0.4.0   06/04/06  bberlin      Added hires_artifacts parameter to configuration
******************************************************************************/
#include <dirent.h>
#ifdef WIN32
#include <direct.h>
#else
#include <glob.h>
#endif
#include "kconfig.h"
#include "util.h"
#include "db_if.h"
#include "input.h"
#include "video.h"
#include "sound.h"
#include "../kat5200.h"

extern t_atari_input g_input;
extern t_atari_video g_video;
extern t_atari_sound g_sound;

static t_config config;
static char g_configname[1024] = "Default";

int config_pre_check_arguments ( int argc, char *argv[] );
int config_check_arguments ( int argc, char *argv[] );

t_config * config_get_ptr ( void ) {

	return &config;

} /* end config_get_ptr */

/******************************************************************************
**  Function   :  config_init
**                            
**  Objective  :  This function initialize the emulator configuration
**                  (We need util_init called before this)
**
**  Parameters :  NONE 
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int config_init (  int argc, char *argv[] ) {

	int state_loading = 0;

	config_set_internal_defaults ( );
	config_pre_check_arguments ( argc, argv );
	config_load ( );
	config_get_exit_status ();
	state_loading = config_check_arguments ( argc, argv );

	return state_loading;
}

/******************************************************************************
**  Function   :  config_set_internal_defaults
**                            
**  Objective  :  This function sets up program defaults that can be overided
**                by the database file and the command line
**
**  Parameters :  NONE 
**                                                
**  return     :  NONE
**      
******************************************************************************/
void config_set_internal_defaults ( void ) {

	/*
	 * Files/Directories
	 */
	strcpy ( config.os_file_5200, "5200.bin" );
	strcpy ( config.os_file_800, "" );
	strcpy ( config.os_file_xl, "" );
	strcpy ( config.os_file_basic, "" );
	strcpy ( config.rom_file, "" );
	strcpy ( config.font_file, "default.bmp" );
	strcpy ( config.state, "" );
	strcpy ( config.default_input_profile, "Default" );
	strcpy ( config.default_800input_profile, "Default800" );
	strcpy ( config.default_video_profile, "Default" );
	strcpy ( config.default_sound_profile, "Default" );
	strcpy ( config.default_ui_profile, "Default" );
	config.throttle = 100;
	config.machine_type = MACHINE_TYPE_5200;
	config.ram_size[MACHINE_TYPE_5200] = 16;
	config.ram_size[MACHINE_TYPE_800] = 48;
	config.ram_size[MACHINE_TYPE_XL] = 64;
	config.system_type = NTSC;
	config.exit_status = 1;
	config.sio_patch = 1;
	config.basic_enable = 0;
	config.first_use = 0;
	config.start_with_gui = 0;
	config.start_gui_with_launcher = 0;
	config.launcher_show_all = 0;
	config.launcher_tabbed_view = 0;
	config.launcher_group_view = 1;

} /* end config_set_internal_defaults */

/******************************************************************************
**  Function   :  config_load_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  config    - pointer to config struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int config_load_callback ( void *config, int argc, char **argv, char **azColName ) {

	int i;
	t_config *p_config = config;

	if ( argc < 25 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE DATABASE!
	 */
	i = -1;
	if ( argv[++i] ) strcpy ( p_config->name, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->description, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->rom_file, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->os_file_5200, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->os_file_800, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->os_file_xl, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->os_file_basic, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->font_file, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->state, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->default_input_profile, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->default_800input_profile, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->default_video_profile, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->default_sound_profile, argv[i] );
	if ( argv[++i] ) strcpy ( p_config->default_ui_profile, argv[i] );
	if ( argv[++i] ) p_config->machine_type = atoi ( argv[i] );
	if ( argv[++i] ) p_config->system_type = atoi ( argv[i] );
	if ( argv[++i] ) p_config->throttle = atoi ( argv[i] );
	if ( argv[++i] ) p_config->ram_size[MACHINE_TYPE_5200] = atoi ( argv[i] );
	if ( argv[++i] ) p_config->ram_size[MACHINE_TYPE_800] = atoi ( argv[i] );
	if ( argv[++i] ) p_config->ram_size[MACHINE_TYPE_XL] = atoi ( argv[i] );
	if ( argv[++i] ) p_config->sio_patch = atoi ( argv[i] );
	if ( argv[++i] ) p_config->basic_enable = atoi ( argv[i] );
	if ( argv[++i] ) p_config->start_with_gui = atoi ( argv[i] );
	if ( argv[++i] ) p_config->start_gui_with_launcher = atoi ( argv[i] );
	if ( argv[++i] ) p_config->launcher_show_all = atoi ( argv[i] );
	if ( argv[++i] ) p_config->launcher_tabbed_view = atoi ( argv[i] );
	if ( argv[++i] ) p_config->launcher_group_view = atoi ( argv[i] );
	if ( argv[++i] ) p_config->exit_status = atoi ( argv[i] );
	if ( argv[++i] ) p_config->first_use = atoi ( argv[i] );

	return 0;

} /* end config_load_callback */

/******************************************************************************
**  Function   :  config_load
**                            
**  Objective  :  This function gets configuration from the database file
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int config_load ( void ) {

	char statement[257];
	int status;

	/*
	 * Create the SQL query statement
	 */
	sprintf ( statement, "SELECT * FROM Config WHERE Name='%s'", g_configname );
	status = db_if_exec_sql ( statement, config_load_callback, &config );

	return 0;

} /* end config_load */

/******************************************************************************
**  Function   :  config_save
**                            
**  Objective  :  This function save configuration to the database
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int config_save ( void ) {

	char statement[2048];
	char *p_tmp[14];
	int status;
	int i;

	sprintf ( statement, "INSERT OR REPLACE INTO Config VALUES ( '%s', '%s', '%s',\
	                         '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',\
	                         '%s', '%s', %d, %d, %d, %d,\
	                         %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )",
				p_tmp[0]=util_replace_and_copy(config.name, "'", "''" ),
				p_tmp[1]=util_replace_and_copy(config.description, "'", "''" ),
				p_tmp[2]=util_replace_and_copy(config.rom_file, "'", "''" ),
				p_tmp[3]=util_replace_and_copy(config.os_file_5200, "'", "''" ),
				p_tmp[4]=util_replace_and_copy(config.os_file_800, "'", "''" ),
				p_tmp[5]=util_replace_and_copy(config.os_file_xl, "'", "''" ),
				p_tmp[6]=util_replace_and_copy(config.os_file_basic, "'", "''" ),
				p_tmp[7]=util_replace_and_copy(config.font_file, "'", "''" ),
				p_tmp[8]=util_replace_and_copy(config.state, "'", "''" ),
				p_tmp[9]=util_replace_and_copy(config.default_input_profile, "'", "''" ),
				p_tmp[10]=util_replace_and_copy(config.default_800input_profile, "'", "''" ),
				p_tmp[11]=util_replace_and_copy(config.default_video_profile, "'", "''" ),
				p_tmp[12]=util_replace_and_copy(config.default_sound_profile, "'", "''" ),
				p_tmp[13]=util_replace_and_copy(config.default_ui_profile, "'", "''" ),
	            config.machine_type, config.system_type, config.throttle,
	            config.ram_size[MACHINE_TYPE_5200], 
	            config.ram_size[MACHINE_TYPE_800], config.ram_size[MACHINE_TYPE_XL],
	            config.sio_patch, config.basic_enable, config.start_with_gui,
	            config.start_gui_with_launcher, config.launcher_show_all, 
	            config.launcher_tabbed_view, config.launcher_group_view, 
				config.exit_status, config.first_use
	);
	status = db_if_exec_sql ( statement, 0, 0 );

	for ( i = 0; i < 14; ++i )
		free (p_tmp[i]);

	return status;

} /* end config_save */

/******************************************************************************
**  Function   :  config_get_exit_status
**                            
**  Objective  :  This function gets the exit status from the config file and
**                resets it to 1
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int config_get_exit_status ( void ) {

	int status;
	char statement[257];

	sprintf ( statement, "SELECT ExitStatus FROM Config WHERE Name='%s'",
	               config.name );
	status = db_if_exec_sql ( statement, db_if_id_callback, &config.exit_status );

	sprintf ( statement, "UPDATE Config SET ExitStatus=1 WHERE Name='%s'",
	               config.name );
	status = db_if_exec_sql ( statement, 0, 0 );

	return 0;

} /* end config_get_exit_status */

/******************************************************************************
**  Function   :  config_get_first_use
**                            
**  Objective  :  This function checks for the programs first use, then sets
**                it to 0
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int config_get_first_use ( void ) {

	int tmp_value = 0;
	int status;
	char statement[257];

	sprintf ( statement, "SELECT FirstUse FROM Config WHERE Name='%s'",
	               config.name );
	status = db_if_exec_sql ( statement, db_if_id_callback, &tmp_value );

	if ( tmp_value ) {
		sprintf ( statement, "UPDATE Config SET FirstUse=0 WHERE Name='%s'",
		               config.name );
		status = db_if_exec_sql ( statement, 0, 0 );
		config.first_use = 0;
	}

	return tmp_value;

} /* end config_get_first_use */

/******************************************************************************
**  Function   :  config_set_exit_status
**                            
**  Objective  :  This function sets the exit status to 0 at exit of program
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int config_set_exit_status ( void ) {

	char statement[257];
	int status;

	config.exit_status = 0;

	sprintf ( statement, "UPDATE Config SET ExitStatus=0 WHERE Name='%s'",
	               config.name );
	status = db_if_exec_sql ( statement, 0, 0 );

	return 0;

} /* end config_set_exit_status */

/******************************************************************************
**  Function   :  config_get_os_file
**                            
**  Objective  :  This function return the os file based on current machine
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
char * config_get_os_file ( void ) {

	if ( config.machine_type == MACHINE_TYPE_5200 )
		return config.os_file_5200;
	else if ( config.machine_type == MACHINE_TYPE_800 )
		return config.os_file_800;
	else
		return config.os_file_xl;

} /* end config_get_os_file */

/******************************************************************************
**  Function   :  config_check_arguments
**                            
**  Objective  :  This function checks command line arguments skipping config
**                file argument
**
**  Parameters :  argc - number of arguments
**                argv - argument list
**                                                
**  return     :                                  
**      
******************************************************************************/
int config_check_arguments ( int argc, char *argv[] ) {

	int i;
	char tmp_file[1000];
	int status = STATE_LOAD_NONE;

	for ( i = 1; i < argc; ++i ) {
		if ( strstr(argv[i], "-debug") )
			console_set_debug_mode (1);
		else if ( strstr(argv[i],"-fullscreen") ) {
			if ( ++i < argc )
				g_video.fullscreen = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-width") ) {
			if ( ++i < argc )
				g_video.width = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-height") ) {
			if ( ++i < argc )
				g_video.height = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-zoom") ) {
			if ( ++i < argc )
				g_video.zoom = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-widescreen") ) {
			if ( ++i < argc )
				g_video.widescreen = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-mouse_speed") ) {
			if ( ++i < argc ) {
				g_input.mouse_speed = atoi(argv[i]);
				if ( g_input.mouse_speed > 5 )
					g_input.mouse_speed = 5;
				if ( g_input.mouse_speed < 1 )
					g_input.mouse_speed = 1;
			}
		}
		else if ( strstr(argv[i],"-sound") ) {
			if ( ++i < argc )
				g_sound.on = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-throttle") ) {
			if ( ++i < argc )
				config.throttle = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-start_with_gui") ) {
			if ( ++i < argc )
				config.start_with_gui = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-start_gui_with_launcher") ) {
			if ( ++i < argc )
				config.start_gui_with_launcher = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-system_type") ) {
			if ( ++i < argc ) {
				if ( strstr(argv[i], "PAL") )
					config.system_type = PAL;
				else
					config.system_type = NTSC;
			}
		}
		else if ( strstr(argv[i],"-sio_patch") ) {
			if ( ++i < argc )
				config.sio_patch = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-basic_enable") ) {
			if ( ++i < argc )
				config.basic_enable = atoi(argv[i]);
		}
		else if ( strstr(argv[i],"-machine_type") ) {
			if ( ++i < argc ) {
				if ( strstr(argv[i], "800XL") || strstr(argv[i], "800XE") )
					config.machine_type = MACHINE_TYPE_XL;
				else if ( strstr(argv[i], "800") )
					config.machine_type = MACHINE_TYPE_800;
				else
					config.machine_type = MACHINE_TYPE_5200;
			}
		}
		else if ( strstr(argv[i],"-state") ) {
			if ( ++i < argc ) {
				strcpy ( config.state, argv[i] );
				status = STATE_LOAD_FILE;
			}
		}
		else if ( strstr(argv[i],"-quick_state") ) {
			if ( ++i < argc ) {
				status = atoi(argv[i]);
			}
		}
		else if ( strstr(argv[i],"-config") ) {
			++i;
		}

		/*
		 * Check for a rom file or and SQL file
		 */
		else {
			if ( strstr(argv[i],":") || (argv[i][0]=='\\') || (argv[i][0]=='/') )
				strcpy ( tmp_file, argv[i] );
			else {
				strcpy ( tmp_file, argv[i] );
				util_set_file_to_current_dir ( tmp_file );
			}

			if ( strstr(tmp_file, ".sql") || strstr(tmp_file, ".SQL") ) {
				db_if_process_file ( tmp_file );
			}
			else {
				strcpy ( config.rom_file, tmp_file );

				/*
				 * If rom input on command line, we don't care about exit status
				 */
				config.exit_status = 0;
			}
		}
	}

	return status;

} /* config_check_arguments */

/******************************************************************************
**  Function   :  config_pre_check_arguments
**                            
**  Objective  :  This function checks command line arguments for alternate
**                config profile
**
**  Parameters :  argc - number of arguments
**                argv - argument list
**                                                
**  return     : 0                                
**      
******************************************************************************/
int config_pre_check_arguments ( int argc, char *argv[] ) {

	int i;

	for ( i = 1; i < argc; ++i ) {
		if ( strstr(argv[i],"-config") ) {
			if ( ++i < argc )
				strcpy ( g_configname, argv[i] );
		}
	}

	return 0;

} /* config_pre_check_arguments */

