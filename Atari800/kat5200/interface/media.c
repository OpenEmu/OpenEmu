/******************************************************************************
*
* FILENAME: media.c
*
* DESCRIPTION:  This contains sound handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/01/07  bberlin      Creation, break out from config
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "media.h"
#include "db_if.h"
#include "logger.h"
#include "util.h"
#include "kconfig.h"
#include "md5.h"
#include "sha1.h"

int media_open_file ( char *filename, unsigned char *buffer );

static t_media g_media;

static int carts_found;
static int total_carts;

extern t_cart cart;

/* 
 * Blantantly copied from atari++
 */
const unsigned char binload_bootstrap[] = {
  0x00,  // bootflag
  0x03,  // # of sectors to boot
  0x00,  // lo boot address
  0x07,  // hi boot address
  0x99,  // lo run address
  0x07,  // hi run address ***
  // bootstrap code runs here
  0x20,0x07,0x08,                     // disk init
  0xa9,0x00,
  0x8d,0xe0,0x02,0x8d,0xe1,0x02,      // clear run vector
  0x8d,0xe2,0x02,0x8d,0xe3,0x02,      // clear init vector
  0x8d,0x7d,0x09,                     // set sector, hi
  0x8d,0x7f,0x09,0x85,0x49,           // init sector buffer to point to sector #4
  0xa9,0x04,0x8d,0x7e,0x09,           // set sector, lo
  //
  0x20,0xe8,0x07,                     // get byte
  0xc9,0xff,                          // must be 0xff for binary load
  0xd0,0x05,                          // branch on error
  0x20,0xe8,0x07,                     // get byte
  0xc9,0xff,                          // another 0xff must follow here
  0xd0,0x62,                          // branch on error to SEC:RTS
  //
  0x20,0xe8,0x07,                     // get start address, lo
  0x85,0x43,0x8d,0xe0,0x02,
  0x20,0xe8,0x07,                     // get start address, hi
  0x85,0x44,0x8d,0xe1,0x02,
  0x20,0xe8,0x07,                     // get end address, lo
  0x85,0x45,
  0x20,0xe8,0x07,                     // get end address, hi
  0x85,0x46,
  // loading loop starts here.
  0x20,0xe8,0x07,                     // get next byte
  0xa0,0x00,                          // reset Y
  0x91,0x43,                          // store result
  0xe6,0x43,                          // increment low
  0xd0,0x02,                          // carry over into high?
  0xe6,0x44,                          // if so, do it now.
  0xa5,0x45,0xc5,0x43,                // compare low-addresses
  0xa5,0x46,0xe5,0x44,                // compare high-addresses.
  0xb0,0xe9,                          // as LONG as this is greater or equal, repeat
  0xad,0xe2,0x02,0x0d,0xe3,0x02,      // check whether the init address has been set
  0xf0,0x0f,                          // if not, avoid the init method
  // run thru init vector
  0xa5,0x49,0x48,                     // keep sector offset
  0x20,0xfb,0x07,                     // jump into the init vector
  // for compatibility, wait for a vblank
  0x20,0x1f,0x08,0x20,0x1f,0x08,
  0x68,0x85,0x49,                     // restore sector offset
  // init vector done
  0x20,0xd4,0x07,                     // get next byte
  0xf0,0x0f,                          // on EOF, exit from boot process
  0x85,0x43,                          // next start, lo
  0x20,0xe8,0x07,                     // next start, hi
  0x85,0x44,
  0x25,0x43,0xc9,0xff,                // is this 0xff,0xff?
  0xf0,0xee,                          // if so, repeat
  0xd0,0xb4,                          // jump back to the loading part
  0xf0,0x09,                          // boot process end
  // here: exit on error: BOOT ERROR
  0x68,0x68,0x68,0x68,                // drop off return address
  0x38,0x60,        
  // here: run/init jumps
  0x6c,0xe2,0x02,                     // jump on init
  0x6c,0xe0,0x02,                     // jump on start
  // here: load new sector.
  0xad,0x7e,0x09,
  0x8d,0x0a,0x03,                     // set sector lo
  0xad,0x7d,0x09,                     // get sector hi (no need to mask, only one file)
  0x8d,0x0b,0x03,                     // install it
  0x0d,0x0a,0x03,0xf0,0x26,           // return with Z flag set on EOF.
  0xa9,0x31,0x8d,0x00,0x03,           // set disk drive
  0xa9,0x01,0x8d,0x01,0x03,           // set unit
  0xa9,0x52,0x8d,0x02,0x03,           // set CMD=read
  0xa9,0x00,0x8d,0x04,0x03,0x85,0x47, // set buffer address, lo
  0x85,0x49,                          // clear offset within sector
  0xa9,0x09,0x8d,0x05,0x03,0x85,0x48, // set buffer address, hi: page 9
  0x20,0x53,0xe4,                     // call SIO
  0x30,0xbf,                          // jump to boot error generation on error.
  0xa0,0x01,                          // clear Z flag
  0x60,                               //
  // here: get byte, return eq on EOF.
  0xa4,0x49,0xcc,0x7f,0x09,           // compare against the length of the sector
  0x90,0x08,                          // skip over if not end.
  0x20,0x9c,0x07,                     // reload sector
  0xd0,0x01,0x60,                     // skip return on not EOF
  0xa0,0x00,                          // reset Y
  0xb1,0x47,                          // read value
  0xe6,0x49,                          // read next value: increment, clear Z flag
  0x60,
  // here: get byte, error on EOF
  0xa4,0x49,0xcc,0x7f,0x09,           // compare against the length of the sector
  0x90,0x07,                          // skip over if not end.
  0x20,0x9c,0x07,                     // reload sector
  0xf0,0x9e,                          // branch to error on EOF.
  0xa0,0x00,                          // reset Y
  0xb1,0x47,                          // read value
  0xe6,0x49,                          // read next value: increment, clear Z flag
  0x60,
  //
  0x20,0x96,0x07,                     // call the init vector
  0xa9,0x00,                          // clear
  0x8d,0xe2,0x02,0x8d,0xe3,0x02,      // the init vector
  0x60,
  //
  0xa9,0x01,0x85,0x09,0xa9,0x00,
  0x8d,0x44,0x02,
  0xa9,0x77,0x85,0x0a,0x85,0x0c,
  0xa9,0xe4,0x85,0x0b,0x85,0x0d,
  0x4c,0x50,0xe4,                     // disk init
  // 
  // VBI waiter
  0xad,0x0b,0xd4,0xc9,0x70,0x90,0xf9,
  0xad,0x0b,0xd4,0xc9,0x20,0xb0,0xf9,
  0x60
};

int cart_header_type[50] = {
	0,
	CART_PC_8K,
	CART_PC_16K,
	CART_OSS,
	CART_32K,
	CART_PC_32K,
	CART_16K_TWO_CHIP,
	CART_40K,
	CART_WILL,
	CART_EXP, CART_DIAMOND, CART_SDX,
	CART_XEGS, CART_XEGS, CART_XEGS,
	CART_OSS2,
	CART_16K_ONE_CHIP,
	CART_ATRAX,
	CART_PC_40K,
	CART_8K,
	CART_4K,
	CART_PC_RIGHT,
	CART_WILL,
	CART_XEGS, CART_XEGS, CART_XEGS,
	CART_MEGA, CART_MEGA, CART_MEGA, CART_MEGA, CART_MEGA, CART_MEGA, CART_MEGA,
	CART_SWXEGS, CART_SWXEGS, CART_SWXEGS, CART_SWXEGS, CART_SWXEGS, CART_SWXEGS,
	CART_PHOENIX, CART_BLIZZARD, 
	CART_ATMAX, CART_ATMAX
};

/******************************************************************************
**  Function   :  media_get_ptr
**                            
**  Objective  :  This function returns the address of the media struct
**
**  Parameters :  NONE
**                                                
**  return     :  address of g_media
**      
******************************************************************************/
t_media *media_get_ptr ( void ) {

	return &g_media;

} /* end media_get_ptr */

/******************************************************************************
**  Function   :  media_settings_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  media     - pointer to media struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int media_settings_callback ( void *media, int argc, char **argv, char **azColName ) {

	int i;
	t_media *p_media = media;

	if ( argc < 7 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE DATABASE!
	 */
	i = -1;
	if ( argv[++i] ) strcpy ( p_media->title, argv[i] );
	if ( argv[++i] ) strcpy ( p_media->filename, argv[i] );
	if ( argv[++i] ) p_media->image_size = atoi ( argv[i] );
	if ( argv[++i] ) p_media->crc = strtoul ( argv[i], NULL, 10 );
	if ( argv[++i] ) strncpy ( p_media->md5, argv[i], 40 );
	if ( argv[++i] ) strncpy ( p_media->sha1, argv[i], 50 );
	if ( argv[++i] ) p_media->machine_type = atoi ( argv[i] );
	if ( argv[++i] ) p_media->mapping = atoi ( argv[i] );
	if ( argv[++i] ) p_media->media_type = atoi ( argv[i] );
	if ( argv[++i] ) strcpy ( p_media->input_profile, argv[i] );
	if ( argv[++i] ) strcpy ( p_media->video_profile, argv[i] );
	if ( argv[++i] ) strcpy ( p_media->sound_profile, argv[i] );
	if ( argv[++i] ) p_media->write_protect = atoi ( argv[i] );
	if ( argv[++i] ) p_media->basic_enable = atoi ( argv[i] );
	if ( argv[++i] ) p_media->sio_patch_enable = atoi ( argv[i] );
	if ( argv[++i] ) p_media->favorite = atoi ( argv[i] );
	if ( argv[++i] ) p_media->ram_size = atoi ( argv[i] );
	if ( argv[++i] ) p_media->flags = atoi ( argv[i] );

	return 0;

} /* end media_settings_callback */

/******************************************************************************
**  Function   :  media_get_settings
**                                                                    
**  Objective  :  This function loads the configuration from the cart db
**                                                   
**  Parameters :  media - struct containing the CRC to search on
**                 
**  return     :  0 if success
**                -1 if failure
******************************************************************************/ 
int media_get_settings ( t_media *media ) {
	
	char statement[257];

	/*
	 * Setup image to default values
	 */
	*(media->title) = '\0';
	*(media->filename) = '\0';
	*(media->md5) = '\0';
	*(media->sha1) = '\0';
	media->mapping = 0;
	media->media_type = 0;
	*(media->input_profile) = '\0';
	*(media->video_profile) = '\0';
	*(media->sound_profile) = '\0';
	media->image_size = 0;
	media->write_protect = 1;
	media->favorite = 0;
	media->ram_size = 0;
	media->flags = 0;
	media->basic_enable = 2;
	media->sio_patch_enable = 2;
	media->machine_type = MACHINE_TYPE_MIN;

	sprintf ( statement, "SELECT * FROM Media WHERE CRC=%lu", media->crc );
	db_if_exec_sql ( statement, media_settings_callback, media );

	if ( strlen(media->title) )
		return 0;
	else
		return -1;

} /* end media_get_settings */

/******************************************************************************
**  Function   :  media_save_settings                                            
**                                                                    
**  Objective  :  This function saves the image settings to the file
**                                                   
**  Parameters :  p_media - media structure containing info to save
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int media_save_settings ( t_media *p_media ) {

	int status,i;
	char statement[1024];
	char *p_tmp[5];

	sprintf ( statement, "INSERT OR REPLACE INTO Media VALUES ( '%s', '%s', %d, %lu, '%s', '%s', %d, %d, %d, '%s', '%s', '%s', %d, %d, %d, %d, %d, %d )",
	            p_tmp[0]=util_replace_and_copy(p_media->title, "'", "''" ),
	            p_tmp[1]=util_replace_and_copy(p_media->filename, "'", "''" ),
	            p_media->image_size, p_media->crc, p_media->md5, p_media->sha1, 
				p_media->machine_type, p_media->mapping, p_media->media_type,
	            p_tmp[2]=util_replace_and_copy(p_media->input_profile, "'", "''" ),
	            p_tmp[3]=util_replace_and_copy(p_media->video_profile, "'", "''" ),
	            p_tmp[4]=util_replace_and_copy(p_media->sound_profile, "'", "''" ),
	            p_media->write_protect, p_media->basic_enable, 
				p_media->sio_patch_enable, p_media->favorite,
	            p_media->ram_size, p_media->flags );
	status = db_if_exec_sql ( statement, 0, 0 );

	for ( i = 0; i < 5; ++i )
		free ( p_tmp[i] );

	return status;

} /* end media_save_settings */

/******************************************************************************
**  Function   :  media_delete_crc                                            
**                                                                    
**  Objective  :  This function deletes the entry in the db based on CRC
**                                                   
**  Parameters :  crc  - crc of the cart entry to be deleted
**                 
**  return     :  0 
**
******************************************************************************/ 
int media_delete_crc ( unsigned long crc ) {

	char statement[257];

	sprintf ( statement, "DELETE FROM Media WHERE CRC=%lu", crc );
	db_if_exec_sql ( statement, 0, 0 );

	return 0;

} /* end media_delete_crc */

/******************************************************************************
**  Function   :  media_clear_filenames       
**                            
**  Objective  :  This function deletes the filenames for each entry in the db
**
**  Parameters :  machine_type - which machine to delete filenames for
**                                                
**  return     :  0 for success, otherwise failure
**      
******************************************************************************/
int media_clear_filenames ( e_machine_type machine_type ) {

	int status;
	char statement[257];

	if ( machine_type == MACHINE_TYPE_5200 )
		sprintf ( statement, "UPDATE Media SET Filename = NULL WHERE MachineID = %d", machine_type );
	else
		sprintf ( statement, "UPDATE Media SET Filename = NULL WHERE MachineID <> %d", MACHINE_TYPE_5200 );
	status = db_if_exec_sql ( statement, 0, 0 );

	return status;

} /* media_clear_filenames */

/******************************************************************************
**  Function   :  media_create_cart_from_file
**                                                                    
**  Objective  :  This function sets up the cart struct from a file
**                                                   
**  Parameters :  file  - file to create from
**                media - media to output
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int media_create_cart_from_file ( char *file, t_media *media ) {

	unsigned char tmp_ary[21];
	unsigned char tmp_arys[21];
	char temp_title[1024];
	char *tmp_string = 0;
	t_config *p_config = config_get_ptr();
	struct stat file_status;
	int i;

	stat ( file, &file_status );

	if ( util_get_all_hash ( file, &media->crc, tmp_ary, tmp_arys ) )
		return -1;

	for ( i = 0; i < 16; ++i )
		sprintf ( media->md5+(i*2), "%02x", tmp_ary[i] );
	for ( i = 0; i < 20; ++i )
		sprintf ( media->sha1+(i*2), "%02x", tmp_arys[i] );

	strcpy ( media->filename, file );
	util_set_file_from_current_dir (media->filename);
	strcpy ( temp_title, media->filename );
	util_strip_to_file ( temp_title );
	strcpy ( media->title, temp_title );
	tmp_string = strrchr ( media->title, '.' );
	if ( tmp_string && (tmp_string > (media->title + strlen(media->title)-5)) ) 
		*tmp_string = '\0';

	media->mapping = CART_MIN;
	media->media_type = CART_MIN;
	strcpy ( media->input_profile, "" );
	strcpy ( media->video_profile, "" );
	strcpy ( media->sound_profile, "" );
	media->image_size = file_status.st_size;
	media->write_protect = 1;
	media->machine_type = p_config->machine_type;
	media->ram_size = 0;
	media->flags = 0;
	media->basic_enable = 2;
	media->sio_patch_enable = 2;

	return 0;

} /* media_create_cart_from_file */

/******************************************************************************
**  Function   :  media_scan_for_bios
**                            
**  Objective  :  This function scans the input directory for bios files
**
**  Parameters :  directory - directory to search
**                scan_subs - 1 to scan sub-directories, 0 if not
**                                                
**  return     :  0 for success, failure otherwise
**      
******************************************************************************/
int media_scan_for_bios ( char *directory, int scan_subs ) {

	struct stat file_status;
	char *temp_ptr = 0;
	char new_file[1200];
	DIR *pdir;
	struct dirent *entry;
	unsigned long media_crc=0;

	/*
	 * First check start to make sure its not a file
	 *       If it is, strip to directory           
	 */
	if ( !stat ( directory, &file_status ) ) {
		if ( !(file_status.st_mode & S_IFDIR) ) {
			if ( (temp_ptr=strrchr(directory,'\\')) )
				*temp_ptr = '\0';
			if ( (temp_ptr=strrchr(directory,'/')) )
				*temp_ptr = '\0';
		}
	}

	/*
	 * Open directory and start searching
	 */
	pdir = opendir ( directory );
	if ( !pdir ) {
		sprintf ( new_file, "media_scan_for_bios: Could not open directory \"%s\"", directory );
		logger_log_message ( LOG_ERROR, new_file, "" );
		return -1;
	}

	while ( (entry=readdir(pdir)) ) {

		if ( !strcmp(entry->d_name,".") || !strcmp(entry->d_name, "..") )
			continue;

		/*
		 * Check for file being a directory
		 */
		util_make_path ( directory, entry->d_name, new_file );
		stat ( new_file, &file_status );

		if ( file_status.st_mode & S_IFDIR ) {
			if ( scan_subs )
				media_scan_for_bios ( new_file, scan_subs );
			continue;
		}

		/*
		 * Get CRC and check for BIOS for current file
		 */
		media_crc = util_get_crc ( new_file );
		media_bios_check ( 0, media_crc, new_file );

	} /* end while searching files */

	closedir ( pdir );

	return 0;

} /* media_scan_for_bios */

/******************************************************************************
**  Function   :  media_scan
**                            
**  Objective  :  This function scans the input directory for cart files that
**                have matching checksums with entries in the cart db
**
**  Parameters :  type      - machine type to search for 
**                directory - directory to search
**                scan_subs - 1 to scan sub-directories, 0 if not
**                bios_check - 1 to set bios files if found, 0 if not
**                                                
**  return     :  0 for success, failure otherwise
**      
******************************************************************************/
int media_scan ( e_machine_type type,  char *directory, int scan_subs, int bios_check ) {

	struct stat file_status;
	char *temp_ptr = 0;
	char new_file[1200];
	DIR *pdir;
	struct dirent *entry;
	int i;
	unsigned char tmp_ary[21];
	t_media tmp_media;

	/*
	 * First check start to make sure its not a file
	 *       If it is, strip to directory           
	 */
	if ( !stat ( directory, &file_status ) ) {
		if ( !(file_status.st_mode & S_IFDIR) ) {
			if ( (temp_ptr=strrchr(directory,'\\')) )
				*temp_ptr = '\0';
			if ( (temp_ptr=strrchr(directory,'/')) )
				*temp_ptr = '\0';
		}
	}

	/*
	 * Open directory and start searching
	 */
	pdir = opendir ( directory );
	if ( !pdir ) {
		sprintf ( new_file, "media_scan: Could not open directory \"%s\"", directory );
		logger_log_message ( LOG_ERROR, new_file, "" );
		return -1;
	}

	while ( (entry=readdir(pdir)) ) {

		if ( !strcmp(entry->d_name,".") || !strcmp(entry->d_name, "..") )
			continue;

		/*
		 * Check for file being a directory
		 */
		util_make_path ( directory, entry->d_name, new_file );
		stat ( new_file, &file_status );

		if ( file_status.st_mode & S_IFDIR ) {
			if ( scan_subs )
				media_scan ( type,  new_file, scan_subs, bios_check );
			continue;
		}

		/*
		 * Get CRC of current file
		 */
		tmp_media.crc = util_get_crc ( new_file );

		if ( bios_check )
			media_bios_check ( 0, tmp_media.crc, new_file );

		/*
		 * Find the entry for this CRC and update if found
		 */
		media_get_settings ( &tmp_media );

		if ( strlen(tmp_media.title) ) {

			if ( tmp_media.machine_type != MACHINE_TYPE_5200 &&
			     type == MACHINE_TYPE_5200 )
				continue;

			carts_found++;
			strcpy ( tmp_media.filename, new_file );

			if ( !tmp_media.image_size )
				tmp_media.image_size = file_status.st_size;

			if ( !strlen(tmp_media.md5) ) {
				md5_file ( new_file, tmp_ary );
				for ( i = 0; i < 16; ++i )
					sprintf ( tmp_media.md5+(i*2), "%02x", tmp_ary[i] );
			}

			if ( !strlen(tmp_media.sha1) ) {
				sha1_file ( new_file, tmp_ary );
				for ( i = 0; i < 20; ++i )
					sprintf ( tmp_media.sha1+(i*2), "%02x", tmp_ary[i] );
			}
			
			media_save_settings ( &tmp_media );
		}
		
	} /* end while searching files */

	closedir ( pdir );

	return 0;

} /* media_scan */

/******************************************************************************
**  Function   :  media_count_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  media     - pointer to media struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int media_count_callback ( void *media, int argc, char **argv, char **azColName ) {

	if ( argc < 1 )
		return -1;

	total_carts++;

	return 0;

} /* end media_count_callback */

/******************************************************************************
**  Function   :  media_scan_directory
**                            
**  Objective  :  This function scans the input directory for cart files that
**                have matching checksums with entries in the cart db
**
**  Parameters :  type       - machine type to search for 
**                directory  - directory to search
**                scan_subs  - 1 to scan sub-directories, 0 if not
**                bios_check - 1 to set bios files if found, 0 if not
**                                                
**  return     :  0 for success, failure otherwise
**      
******************************************************************************/
int media_scan_directory ( e_machine_type type,  char *directory, 
                              int scan_subs, int bios_check ) {

	char msg[200];

	carts_found = 0;
	total_carts = 0;

	media_bios_check ( 1, 0, "" );

	db_if_exec_sql ( "BEGIN", 0, 0 );

	media_scan ( type,  directory, scan_subs, bios_check );

	sprintf ( msg, "SELECT CRC FROM Media WHERE MachineID=%d", type );
	db_if_exec_sql ( msg, media_count_callback, &total_carts );

	db_if_exec_sql ( "END", 0, 0 );

	if ( bios_check )
		config_save();

	sprintf ( msg, "media_scan_directory: Found %d out of %d entries", 
	               carts_found, total_carts );
	logger_log_message ( LOG_INFO, msg, "" );

	return 0;

} /* media_scan_directory */

/******************************************************************************
**  Function   :  media_add_from_directory
**                            
**  Objective  :  This function scans the input directory for cart files that
**                have matching checksums with entries in the cart db
**
**  Parameters :  directory - directory to search
**                scan_subs - 1 to scan sub-directories, 0 if not
**                                                
**  return     :  0 for success, failure otherwise
**      
******************************************************************************/
int media_add_from_directory ( char *directory, int scan_subs ) {

	struct stat file_status;
	char *temp_ptr = 0;
	char new_file[1200];
	int  cart_found;
	DIR *pdir;
	struct dirent *entry;
	unsigned long media_crc=0;
	t_media tmp_cart;

	/*
	 * First check start to make sure its not a file
	 *       If it is, strip to directory           
	 */
	if ( !stat ( directory, &file_status ) ) {
		if ( !(file_status.st_mode & S_IFDIR) ) {
			if ( (temp_ptr=strrchr(directory,'\\')) )
				*temp_ptr = '\0';
			if ( (temp_ptr=strrchr(directory,'/')) )
				*temp_ptr = '\0';
		}
	}

	/*
	 * Open directory and start searching
	 */
	pdir = opendir ( directory );
	if ( !pdir ) {
		sprintf ( new_file, "media_scan: Could not open directory \"%s\"", directory );
		logger_log_message ( LOG_ERROR, new_file, "" );
		return -1;
	}

	while ( (entry=readdir(pdir)) ) {

		if ( !strcmp(entry->d_name,".") || !strcmp(entry->d_name, "..") )
			continue;

		/*
		 * Check for file being a directory
		 */
		util_make_path ( directory, entry->d_name, new_file );
		stat ( new_file, &file_status );

		if ( file_status.st_mode & S_IFDIR ) {
			if ( scan_subs )
				media_add_from_directory ( new_file, scan_subs );
			continue;
		}

		/*
		 * Get CRC of current file
		 */
		media_crc = util_get_crc ( new_file );
		cart_found = 0;

		media_get_settings ( &tmp_cart );

		if ( strlen(tmp_cart.title) )
			cart_found = 1;

		/*
		 * If cart not found, add it
		 */
		if ( !cart_found ) {
			carts_found++;
			media_create_cart_from_file ( new_file, &tmp_cart );
			media_save_settings ( &tmp_cart );
		}

	} /* end while searching files */

	closedir ( pdir );

	return 0;

} /* media_add_from_directory */

/******************************************************************************
**  Function   :  media_create_from_directory
**                            
**  Objective  :  This function scans the input directory for cart files that
**                have matching checksums with entries in the cart db
**
**  Parameters :  type      - file type to search for 
**                directory - directory to search
**                scan_subs - 1 to scan sub-directories, 0 if not
**                overwrite - overwrite existing data in file 
**                                                
**  return     :  0 for success, failure otherwise
**      
******************************************************************************/
int media_create_from_directory ( char *filename,  char *directory, 
                                     int scan_subs, int overwrite ) {

	char msg[1100];

	/*
	 * If overwrite, clear out the current media
	 */
	if ( overwrite ) {
		strcpy ( msg, "DELETE * FROM Media" );
		db_if_exec_sql ( msg, NULL, NULL );
	}

	/*
	 * Now scan the directory and sub-directories
	 */
	carts_found = 0;
	media_add_from_directory ( directory, scan_subs );

	/*
	 * Save the doc to file
	 */
	if ( overwrite )
		sprintf ( msg, "media_create_from_directory: Created CartDB File \"%s\"", filename );
	else
		sprintf ( msg, "media_create_from_directory: Updated CartDB File \"%s\"", filename );
	logger_log_message ( LOG_INFO, msg, "" );
	sprintf ( msg, "media_create_from_directory: Added %d entries", carts_found );
	logger_log_message ( LOG_INFO, msg, "" );

	return 0;

} /* media_create_from_directory */

/******************************************************************************
**  Function   :  media_bios_check                                            
**                                                                    
**  Objective  :  This function checks input CRC matching a BIOS and setting
**                configuration accordingly
**                                                   
**  Parameters :  reset - start search over if 1, continue if 0
**                crc   - CRC to check
**                filename - file of rom
**                 
**  return     :  0 
**
******************************************************************************/ 
int media_bios_check ( int reset, unsigned long crc, char *filename ) {

	int i;
	static int bios_found[12];
	t_config *p_config = config_get_ptr();

	if ( reset ) {
		for ( i = 0; i < 12; ++i )
			bios_found[i] = 0;
	}

	switch ( crc ) {
		case 0x4248d3e3: /* 5200 ROM */
			strcpy ( p_config->os_file_5200, filename );
			bios_found[0] = 1;
			break;
		case 0x72b3fed4: /* OS Rev A PAL */
			if ( !bios_found[2] && !bios_found[3] )
				strcpy ( p_config->os_file_800, filename );
			bios_found[1] = 1;
			break;
		case 0x3e28a1fe: /* OS Rev B PAL */
			if ( !bios_found[3] )
				strcpy ( p_config->os_file_800, filename );
			bios_found[2] = 1;
			break;
		case 0xe86d61d:  /* OS Rev B NTSC */
			strcpy ( p_config->os_file_800, filename );
			bios_found[3] = 1;
			break;
		case 0x643bcc98:  /* OS Rev 1 600XL */
			if ( !bios_found[6]  && !bios_found[7] && !bios_found[8] )
				strcpy ( p_config->os_file_xl, filename );
			bios_found[4] = 1;
			break;
		case 0xc5c11546:  /* OS Rev 10 1200XL */
			if ( !bios_found[4] && !bios_found[6] && 
			     !bios_found[7] && !bios_found[8] )
				strcpy ( p_config->os_file_xl, filename );
			bios_found[5] = 1;
			break;
		case 0x1f9cd270:  /* OS Rev 2 800XL */
			strcpy ( p_config->os_file_xl, filename );
			bios_found[6] = 1;
			break;
		case 0x29f133f7:  /* OS Rev 3 800XE-65XE-130XE */
			if ( !bios_found[6] )
				strcpy ( p_config->os_file_xl, filename );
			bios_found[7] = 1;
			break;
		case 0x1eaf4002:  /* OS Rev 4 XEGS */
			if ( !bios_found[6]  && !bios_found[7] )
				strcpy ( p_config->os_file_xl, filename );
			bios_found[8] = 1;
			break;
		case 0x4bec4de2: /* BASIC Rev A 800 */
			if ( !bios_found[10] && !bios_found[11] )
				strcpy ( p_config->os_file_basic, filename );
			bios_found[9] = 1;
			break;
		case 0xf0202fb3: /* BASIC Rev B XL */
			if ( !bios_found[11] )
				strcpy ( p_config->os_file_basic, filename );
			bios_found[10] = 1;
			break;
		case 0x7d684184: /* BASIC Rev C XL-XE */
			strcpy ( p_config->os_file_basic, filename );
			bios_found[11] = 1;
			break;
		default:
			break;
	}

	return 0;

} /* media_bios_check */

/******************************************************************************
**  Function   :  media_open_basic                                            
**                                                                    
**  Objective  :  This function opens up the atari basic cart file
**                                                   
**  Parameters :  filename     - file of the rom
**                enable_basic - whether or not to enable basic
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int media_open_basic ( char *filename, int enable_basic ) {

	int bytes;
	unsigned char *buffer;

	buffer = malloc(sizeof(char)*32768);

	/*
	 * We need to make sure it is clear if not enabling BASIC
	 *   Put 0xfe since the 800 OS will inc bffc before checking for non-zero
	 */
	if ( !enable_basic ) {
		memset ( buffer, 0xfefefefe, 32768 );
		memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
		memory_write_basic_bytes ( 0xa000, buffer, 0x2000 );
		free ( buffer );
		return 0;
	}


	if ( (bytes = media_open_file ( filename, buffer )) < 0 ) {
		free ( buffer );
		return ERR_BASIC_LOAD;
	}

	if ( console_get_machine_type() == MACHINE_TYPE_XL )
		memory_write_basic_bytes ( 0xa000, buffer, 0x2000 );
	else
		cart_open ( filename, buffer, bytes, util_get_crc(filename), CART_PC_8K, 0 );

	free ( buffer );

	return 0;

} /* media_open_basic */

/******************************************************************************
**  Function   :  media_exe_alloc
**                                                                    
**  Objective  :  This function rearranges the exe bin and adds the bootstrap
**                  code before mounting as a floppy disk
**                                                   
**  Parameters :  buffer     - original exe buffer
**                bytes      - number of bytes in original buffer
**                bin_buffer - output buffer to fill up
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int media_exe_alloc ( unsigned char *buffer, int bytes, unsigned char *bin_buffer ) {

	int i,j;
	int start, end, byte_pos, sector;
	unsigned char *buf_pos;

	memcpy ( bin_buffer, binload_bootstrap, sizeof(binload_bootstrap));

	/*
	 * Start storing stuff into our buffer
	 */
	buf_pos = bin_buffer + (128 * 3);
	byte_pos = i = 0;
	sector = 4;
	while ( byte_pos < bytes ) {

		/*
		 * So first find our start and end positions
		 */
		j = 0;
		while ( ((byte_pos + j + 1) < bytes) && 
		          buffer[byte_pos + j] == 0xff && 
		          buffer[byte_pos + j + 1] == 0xff ) {
			j += 2;
		}

		if ( (byte_pos + j + 3) < bytes ) {
			start = buffer[byte_pos + j] | (buffer[byte_pos + j + 1] << 8);
			end = buffer[byte_pos + j + 2] | (buffer[byte_pos + j + 3] << 8);
		}
		else {
			start = end = 0;
		}

		/*
		 * If something's not right, truncate
		 */
		if ( start > end  || start == 0 || end == 0 ) {
			if ( i > 0 ) {
				buf_pos[125] = 0;
				buf_pos[126] = 0;
				buf_pos[127] = i;
			}
			else if ( byte_pos > 125 ) {
				buf_pos -= 125;
				buf_pos[125] = 0;
				buf_pos[126] = 0;
				buf_pos[127] = 125;
			}

			break;
		}

		/*
		 * Now start storing the sector until we get 125 or start > end
		 */
		start -= (4+j);
		while ( start <= end ) {

			while ( i < 125 ) {
				buf_pos[i] = buffer[byte_pos];
				byte_pos++;
				i++;
				start++;
				if ( start > end )
					break;
				if ( byte_pos >= bytes )
					break;
			}

			if ( i == 125 ) {
				sector++;
				buf_pos[125] = (sector) >> 8;
				buf_pos[126] = (sector) & 0xff;
				buf_pos[127] = 125;
				buf_pos += 128;
				i = 0;
			}

		} /* end while start position is less than end position */

	} /* end while buffer bytes are processed */

	/*
	 * Check for left over data without a sector end
	 */
	if ( i > 0 ) {
		buf_pos[125] = 0;
		buf_pos[126] = 0;
		buf_pos[127] = i;
	}

	bytes = sector * 128;

	return bytes;

} /* media_exe_alloc */

/******************************************************************************
**  Function   :  media_open_rom                                            
**                                                                    
**  Objective  :  This function opens the image indicated by the file.  What
**                  type of media is determined as follows:
**                   1.  Check media_type input here.  If 0, then
**                   2.  Check database for type.  If AUTO, then
**                   3.  Check file for a header.  If NONE, then
**                   4.  Determine by file extension.  If unable, then
**                   5.  Treat as cartridge.
**                                                   
**  Parameters :  filename   - file to open
**                media_type - type of media (0 for auto)
**                slot       - desired floppy disk slot
**                launch     - "Launch" to image instead of just attach
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int media_open_rom ( char *filename, int media_type, int slot, int launch ) {

	int bytes;
	int i, status;
	char lowername[1100];
	unsigned char *buffer, *bin_buffer;
	unsigned long save_crc;
	e_cart_type tmp_mapping;
	t_config *p_config = config_get_ptr();

	buffer = malloc(sizeof(char)*131090);
	if ( (bytes = media_open_file ( filename, buffer )) < 0 ) {
		free ( buffer );
		return ERR_ROM_LOAD;
	}

	strcpy ( lowername, filename );
	i = -1;
	while ( lowername[++i] ) lowername[i] = tolower(lowername[i]);

	save_crc = g_media.crc;
	tmp_mapping = media_type;

	status = media_get_settings ( &g_media );
	strcpy ( g_media.filename, filename );

	/*
	 * Determine media type if input is AUTO
	 */
	if ( tmp_mapping == CART_MIN ) {

		/*
		 * If database indicates AUTO, try to see if there is a header
		 */
		if ( g_media.mapping == CART_MIN || status ) {
			if ( !strncmp(buffer, "CART", 4) ) {
				tmp_mapping = cart_header_type [(buffer[4] << 24) | 
				                     (buffer[5] << 16) | (buffer[6] << 8) | (buffer[7])];
				bytes -= 16;
				memcpy ( buffer, buffer + 16, bytes );
			}

			/*
			 * If no CART Header, check by filename and media_type
			 */
			else {
				if ( strstr(lowername, ".atr") || strstr(lowername,".atz") ||
				     strstr(lowername, ".xfd") || strstr(lowername,".xfz") || 
				     g_media.media_type == CART_FLOPPY )
					tmp_mapping = CART_FLOPPY;
				else if ( strstr(lowername, ".cas") || g_media.media_type == CART_CASSETTE )
					tmp_mapping = CART_CASSETTE;
				else if ( strstr(lowername, ".xex") || strstr(lowername, ".exe") || 
				          strstr(lowername, ".com") || g_media.media_type == CART_EXE )
					tmp_mapping = CART_EXE;
			}

		} /* end if database mapping is AUTO */

		else {
			tmp_mapping = g_media.mapping;
		}

	} /* end if input mapping is AUTO */

	/*
	 * See if we can set the machine type based on this CRC
	 *   else we will just use what's already set
	 */
	if ( !status ) {
		if ( g_media.machine_type == MACHINE_TYPE_5200 )
			p_config->machine_type = MACHINE_TYPE_5200;
		else {
			if ( g_media.machine_type == MACHINE_TYPE_MIN ) {
				if ( p_config->machine_type == MACHINE_TYPE_5200 )
					p_config->machine_type = MACHINE_TYPE_XL;
			}
			else
				p_config->machine_type = g_media.machine_type;
		}
	}

	config_save();
	console_set_machine_type ( p_config->machine_type );
	console_get_ptr()->cart->loaded = 1;
	console_get_ptr()->basic_enable = 1;
	console_get_ptr()->cart->size = 0x2000;

	if ( g_media.basic_enable == 2 )
		console_get_ptr()->basic_enable = p_config->basic_enable;
	else
		console_get_ptr()->basic_enable = g_media.basic_enable;

	if ( p_config->machine_type == MACHINE_TYPE_XL || !console_get_ptr()->basic_enable ) {
		console_get_ptr()->cart->loaded = 0;
	}

	switch ( tmp_mapping ) {
		case CART_FLOPPY:
			if ( media_open_basic ( p_config->os_file_basic, console_get_ptr()->basic_enable ) )
				return ERR_BASIC_LOAD;

			sio_mount_image ( slot, SIO_TYPE_FLOPPY, filename, 1, buffer, bytes, launch );
			g_media.crc = save_crc;
			g_media.mapping = tmp_mapping;
			break;
		case CART_CASSETTE:
			if ( media_open_basic ( p_config->os_file_basic, console_get_ptr()->basic_enable ) )
				return ERR_BASIC_LOAD;
	
			sio_mount_image ( 0, SIO_TYPE_CASSETTE, filename, 1, buffer, bytes, launch );
			g_media.crc = save_crc;
			g_media.mapping = tmp_mapping;
			break;
		case CART_EXE:
			if ( media_open_basic ( p_config->os_file_basic, console_get_ptr()->basic_enable ) )
				return ERR_BASIC_LOAD;

			bin_buffer = malloc(sizeof(char)*131084);
	
			bytes = media_exe_alloc ( buffer, bytes, bin_buffer );

			sio_mount_image ( 0, SIO_TYPE_FLOPPY, filename, 1, bin_buffer, bytes, launch );

			free ( bin_buffer );
			g_media.mapping = tmp_mapping;
			break;
		default:
			if ( launch )
				sio_unmount_all ( );
			if ( g_media.mapping != CART_MIN )
				tmp_mapping = g_media.mapping;
			cart_open ( filename, buffer, bytes, g_media.crc, tmp_mapping, launch );
			if ( g_media.mapping == CART_PC_RIGHT ) {
				if ( media_open_basic ( p_config->os_file_basic, console_get_ptr()->basic_enable ) )
					return ERR_BASIC_LOAD;
			}
			g_media.mapping = cart.mapping;
			console_get_ptr()->cart->loaded = 1;
			console_get_ptr()->cart->size = bytes;
			break;
	}

	/*
	 * Check for 5200 Bounty Bob 3 piece rom
	 */
	if ( g_media.crc == 0x74e6a611 ||
	     g_media.crc == 0xefa4915d ||
	     g_media.crc == 0x02931055 ) {

		g_media.crc = 0x74e6a611;
		media_get_settings ( &g_media );
		media_open_file ( g_media.filename, buffer );
		cart_open ( g_media.filename, buffer, bytes, g_media.crc, g_media.mapping, launch );

		g_media.crc = 0xefa4915d;
		media_get_settings ( &g_media );
		media_open_file ( g_media.filename, buffer );
		cart_open (  g_media.filename, buffer, bytes, g_media.crc, g_media.mapping, launch );

		g_media.crc = 0x02931055;
		media_get_settings ( &g_media );
		media_open_file ( g_media.filename, buffer );
		cart_open ( g_media.filename, buffer, bytes, g_media.crc, g_media.mapping, launch );

		g_media.mapping = cart.mapping;
	}

	/*
	 * Close and get out
	 */
	free ( buffer );
	
	return 0;

} /* end media_open_rom */

/******************************************************************************
**  Function   :  media_open_file                                            
**                                                                    
**  Objective  :  This function opens the file and puts contents in buffer
**                                                   
**  Parameters :  filename - file to open
**                buffer   - where to store the contents of the file
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int media_open_file ( char *filename, unsigned char *buffer ) {

	gzFile zip_fp;
	int bytes;
	char msg[1100];

	zip_fp = gzopen ( filename, "rb" );

	if ( zip_fp ) {
		bytes = gzread ( zip_fp, buffer, 131088 );
		sprintf ( msg, "media_open_file: Opened Rom File \"%s\"", filename );
		logger_log_message ( LOG_INFO, msg, "" );
	}
	else {
		sprintf ( msg, "media_open_file: Unable to Open Rom File \"%s\"", filename );
		logger_log_message ( LOG_ERROR, msg, "" );
		return -1; 
	}

	gzclose (zip_fp);

	g_media.crc = crc32(0L,buffer,bytes);

	return bytes;

} /* end media_open_file */

