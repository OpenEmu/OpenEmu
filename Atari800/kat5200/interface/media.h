/******************************************************************************
*
* FILENAME: media.h
*
* DESCRIPTION:  This contains function declarations for image handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/01/06  bberlin      Creation, break out from config
******************************************************************************/
#ifndef media_h
#define media_h

#include "../core/console.h"
#include "../core/cart.h"
#include "../core/sio.h"
#include "../core/memory.h"
#include "kconfig.h"

typedef struct {
	char title[257];
	char filename[1100];
	int  image_size;
	char input_profile[257];
	char video_profile[257];
	char sound_profile[257];
	e_cart_type mapping;
	e_cart_type media_type;
	e_machine_type machine_type;
	int  favorite;
	int  ram_size;
	int  write_protect;
	int  basic_enable;
	int  sio_patch_enable;
	unsigned long crc;
	char md5[40];
	char sha1[50];
	int flags;
} t_media;

/*
 * Function Declarations
 */
int media_init ( void );
int media_get_settings ( t_media *cart );
int media_save_settings ( t_media *cart );
int media_delete_crc ( unsigned long crc );
t_media *media_get_ptr ( void );

int media_clear_filenames ( e_machine_type type );
int media_create_cart_from_file ( char *file, t_media *cart );
int media_scan_for_bios ( char *directory, int scan_subs );
int media_scan_directory ( e_machine_type type,  char *directory, int scan_subs, int bios_check );
int media_create_from_directory ( char *filename,  char *directory, 
                                     int scan_subs, int overwrite );
int media_bios_check ( int reset, unsigned long crc, char *filename );
int media_open_rom ( char *filename, int media_type, int slot, int launch );

#endif
