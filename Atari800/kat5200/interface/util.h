/******************************************************************************
*
* FILENAME: util.h
*
* DESCRIPTION:  This contains function declarations for utility functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.2     03/09/06  bberlin      Added functions declarations for filename and
*                                  directory handling functions
******************************************************************************/
#ifndef util_h
#define util_h

/*
 * Error Codes
 */
#define ERR_ROM_LOAD -1
#define ERR_BASIC_LOAD -2
#define ERR_BIOS_LOAD -3
#define ERR_PROFILE_LOAD -4

void util_init ( char *p_path );
int util_strip_file_to_directory ( char * filename );
int util_strip_to_file ( char * filename );
int util_delete_delimeters ( char *string );
int util_set_file_from_relative_dir ( char *filename, char *directory );
int util_set_file_from_program_dir ( char *filename );
int util_set_file_from_current_dir ( char *filename );
int util_set_file_to_program_dir ( char *filename );
int util_set_file_to_current_dir ( char *filename );
char util_get_dir_seperator_character ( void );
int util_all_whitespace(const char *z);
int util_is_command_terminator(const char *zLine);
int util_ends_with_semicolon(const char *z, int N);
int util_make_path ( char *dir, const char *file, char *path );
int util_file_copy ( char *from, char *to );
char * util_create_program_path ( char *path, char *filename );
int util_search_and_replace ( char *string, char *find, char *replace );
char * util_replace_and_copy ( char *string1, char *find, char *replace );
unsigned long util_get_crc ( char *file );
unsigned long util_get_all_hash ( char *file, unsigned long *crc, 
                                              unsigned char md5[16], 
                                              unsigned char sha1[20] );
int util_delay ( double seconds );

#endif
