/******************************************************************************
*
* FILENAME: util.c
*
* DESCRIPTION:  This contains utility functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.2     03/09/06  bberlin      Changed display message functions to write to
*                                  log file instead of console
*                                Added functions for handling filenames and
*                                  directories
*                                Fixed 'util_get_crc' to exit on file error
*                                Integrated use of GUI into debugger
* 0.3.0   03/27/06  bberlin      Changed 'debug_print_value' to call pokey read
*                                  register function instead of using values
* 0.6.2   06/26/09  bberlin      Updated util_init to use Windows user profile
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
//#ifdef WIN32
//#include <direct.h>
//#include <shlobj.h>
//#define getcwd _getcwd 
//#else
#include <glob.h>
#include <unistd.h>
//#endif
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "util.h"
#include "md5.h"
#include "sha1.h"
#include "../kat5200.h"

static char g_program_path[1024];

/******************************************************************************
**  Function   :  util_init                                            
**                                                                    
**  Objective  :  This function initializes utility stuff               
**                                                   
**  Parameters :  p_path - program calling string ( argv[0] )
**                 
**  return     :  NONE
**
******************************************************************************/ 
void util_init ( char *p_path ) {

	char new_directory[1024];
	int error;
	FILE *fp;

	/*
	 * Figure out the program config path
	 *   If not Windows, check for users home directory
	 */

#ifndef WIN32

	strcpy ( g_program_path, "" );
	glob_t glob_result;

	glob ( "~", GLOB_TILDE, NULL, &glob_result );
	sprintf ( new_directory, "%s/.kat5200/kat5200.db3", *(glob_result.gl_pathv) );
	fp = fopen ( new_directory, "r" );
	if ( fp ) {
		sprintf ( g_program_path, "%s/.kat5200", *(glob_result.gl_pathv) );
		fclose ( fp );
	}

	/*
	 * If there is an /etc then use that and try to create home
	 * directory for files
	 */
	else {
		fp = fopen ( "/etc/kat5200/kat5200.db3", "r" );
		if ( fp ) {
			fclose ( fp );
			strcpy ( g_program_path, "/etc/kat5200" );
			sprintf ( new_directory, "%s/.kat5200", *(glob_result.gl_pathv) );

			if ( !(error=mkdir(new_directory, S_IREAD | S_IWRITE | S_IEXEC)) ) {

				sprintf ( g_program_path, "%s/.kat5200", *(glob_result.gl_pathv) );

				sprintf ( new_directory, "%s/.kat5200/kat5200.db3", *(glob_result.gl_pathv) );
				util_file_copy ( "/etc/kat5200/kat5200.db3", new_directory );

				sprintf ( new_directory, "%s/.kat5200/default.bmp", *(glob_result.gl_pathv) );
				util_file_copy ( "/etc/kat5200/default.bmp", new_directory );

				sprintf ( new_directory, "%s/.kat5200/kat5200.bmp", *(glob_result.gl_pathv) );
				util_file_copy ( "/etc/kat5200/kat5200.bmp", new_directory );
			}
		}

	} /* else if not in home directory, create from /etc */
#else
	char userAppDir[1024];
	char appFile[1024];
	char appVersion[10];
	
	strcpy ( g_program_path, "" );
	strcpy ( appVersion, VERSION );
	*( strrchr(appVersion, '.') ) = '\0';

	/*
	 *   If Windows, check for users roaming profile
	 */
	SHGetFolderPathA( NULL, CSIDL_APPDATA, NULL, 0, userAppDir );
	strcat ( userAppDir, "\\kat5200\\" );
	strcat ( userAppDir, appVersion );

	sprintf ( new_directory, "%s\\kat5200.db3", userAppDir );

	fp = fopen ( new_directory, "r" );
	if ( fp ) {
		strcpy ( g_program_path, userAppDir );
		fclose ( fp );
	}

	else {
		util_strip_file_to_directory ( p_path );
		sprintf ( new_directory, "%s\\kat5200.db3", p_path );
		fp = fopen ( new_directory, "r" );
		if ( fp ) {
			fclose ( fp );
			strcpy ( g_program_path, p_path );

			if ( !(error=mkdir(userAppDir)) ) {

				strcpy ( g_program_path, userAppDir );

				sprintf ( new_directory, "%s\\kat5200.db3", userAppDir );
				sprintf ( appFile, "%s\\kat5200.db3", p_path );
				CopyFileA (appFile, new_directory, 0 );

				sprintf ( new_directory, "%s\\default.bmp", userAppDir );
				sprintf ( appFile, "%s\\default.bmp", p_path );
				CopyFileA (appFile, new_directory, 0 );

				sprintf ( new_directory, "%s\\kat5200.bmp", userAppDir );
				sprintf ( appFile, "%s\\kat5200.bmp", p_path );
				CopyFileA (appFile, new_directory, 0 );
			}
		}

	} /* else if not in user's profile, create from application directory */

#endif

	return;

} /* end util_init */

/******************************************************************************
**  Function   :  util_strip_file_to_directory                                       
**                                                                    
**  Objective  :  This function is used to strip a filename down to it's
**                directory name
**                                                   
**  Parameters :  filename  - pointer to the filename
**                 
**  return     :  0
**  
******************************************************************************/ 
int util_strip_file_to_directory ( char * filename ) {

	struct stat file_status;
	char *temp_ptr;

	if ( !stat ( filename, &file_status ) ) {
		if ( !(file_status.st_mode & S_IFDIR) ) {
			if ( (temp_ptr=strrchr(filename,'/')) )
				*temp_ptr = '\0';
			else if ( (temp_ptr=strrchr(filename,'\\')) )
				*temp_ptr = '\0';
			else
				*filename = '\0';
		}
	}

	/*
	 * If file doesn't exist assume we just strip
	 */
	else {
		if ( (temp_ptr=strrchr(filename,'/')) )
			*temp_ptr = '\0';
		else if ( (temp_ptr=strrchr(filename,'\\')) )
			*temp_ptr = '\0';
		else
			*filename = '\0';
	}

	return 0;

} /* end util_strip_file_to_directory */

/******************************************************************************
**  Function   :  util_strip_to_file                                       
**                                                                    
**  Objective  :  This function is used to strip an absolute pathname down to
**                it's filename
**                                                   
**  Parameters :  filename  - pointer to the filename
**                 
**  return     :  0
**
******************************************************************************/ 
int util_strip_to_file ( char * filename ) {

	struct stat file_status;
	char *temp_ptr;

	if ( !stat ( filename, &file_status ) ) {
		if ( (temp_ptr=strrchr(filename,'\\')) )
			strcpy ( filename, temp_ptr+1 );
		if ( (temp_ptr=strrchr(filename,'/')) )
			strcpy ( filename, temp_ptr+1 );
	}

	return 0;

} /* end util_strip_to_file */

/******************************************************************************
**  Function   :  util_set_file_from_relative_dir                                       
**                                                                    
**  Objective  :  This function takes an absolute filename and checks if it is
**                relative to the input directory and strips the directory off
**                if so
**                                                   
**  Parameters :  filename  - pointer to the filename to strip
**                directory - directory to check for relative path
**                 
**  return     :  0
**
******************************************************************************/ 
int util_set_file_from_relative_dir ( char *filename, char *directory ) {

	char temp[1100];

	if ( !strlen(filename) || !strlen(directory) )
		return 0;

	if ( strstr(filename, directory) ) {
		strcpy ( temp, filename+strlen(directory)+1 );
		strcpy ( filename, temp );
	}

	return 0;

} /* end util_set_file_from_relative_dir */

/******************************************************************************
**  Function   :  util_set_file_from_program_dir                                       
**                                                                    
**  Objective  :  This function takes an absolute filename and checks if it is
**                relative to the program directory and strips the 
**                directory off if so
**                                                   
**  Parameters :  filename  - pointer to the filename to strip
**                 
**  return     :  0
**
******************************************************************************/ 
int util_set_file_from_program_dir ( char *filename ) {

	if ( !strlen(filename) )
		return 0;

	util_set_file_from_relative_dir ( filename, g_program_path );

	return 0;

} /* end util_set_file_from_program_dir */

/******************************************************************************
**  Function   :  util_set_file_to_program_dir                                       
**                                                                    
**  Objective  :  This function checks for a file being relative and if we
**                are not in the program directory it creates an absolute
**                path with the program directory.
**                                                   
**  Parameters :  filename  - pointer to the filename to strip
**                 
**  return     :  0
**
******************************************************************************/ 
int util_set_file_to_program_dir ( char *filename ) {

	char *directory;
	char *path;
	int i = -1;

	if ( !strlen(filename) )
		return 0;

	if ( (filename[0] == '/') || (filename[1] == ':') )
		return 0;

	directory = malloc ( sizeof(char)*1100 );
	getcwd ( directory, 1100 );

	path = malloc ( sizeof(char)*1100 );
	strcpy ( path, g_program_path );

#ifdef WIN32
	while ( path[++i] ) path[i] = tolower(g_program_path[i]);
	i = -1;
	while ( directory[++i] ) directory[i] = tolower(directory[i]);
#endif

	if ( strstr(directory,path) ){
		free(directory);
		free(path);
		return 0;
	}

	util_make_path ( g_program_path, filename, path );
	strcpy ( filename, path );

	free(directory);
	free(path);

	return 0;

} /* end util_set_file_to_program_dir */

/******************************************************************************
**  Function   :  util_set_file_from_current_dir                                       
**                                                                    
**  Objective  :  This function takes an absolute filename and checks if it is
**                relative to the current working directory and strips the 
**                directory off if so
**                                                   
**  Parameters :  filename  - pointer to the filename to strip
**                 
**  return     :  0
**
******************************************************************************/ 
int util_set_file_from_current_dir ( char *filename ) {

	char *directory;

	directory = malloc ( sizeof(char)*1100 );
	getcwd ( directory, 1100 );

	util_set_file_from_relative_dir ( filename, directory );

	free ( directory );

	return 0;

} /* end util_set_file_from_current_dir */

/******************************************************************************
**  Function   :  util_set_file_to_current_dir                                       
**                                                                    
**  Objective  :  This function checks for a file being relative and appends
**                to current working directory if so to make an absolute name
**                                                   
**  Parameters :  filename  - pointer to the filename to setup
**                 
**  return     :  0
**
******************************************************************************/ 
int util_set_file_to_current_dir ( char *filename ) {

	char *directory;
	char *path;

	if ( !strlen(filename) )
		return 0;

	if ( (filename[0] == '/') || (filename[1] == ':') )
		return 0;

	directory = malloc ( sizeof(char)*1100 );
	getcwd ( directory, 1100 );

	path = malloc ( sizeof(char)*1100 );

	util_make_path ( directory, filename, path );
	strcpy ( filename, path );

	free(directory);
	free(path);

	return 0;

} /* end util_set_file_to_program_dir */

/******************************************************************************
**  Function   :  util_get_dir_seperator_character                                       
**                                                                    
**  Objective  :  This function gets the directory seperator character based
**                on the current working directory (absolute) and returns it
**                if so
**                                                   
**  Parameters :  NONE
**                 
**  return     :  seperator character
** 
******************************************************************************/ 
char util_get_dir_seperator_character ( void ) {

	char *filename;
	char tmp;

	filename = malloc ( sizeof(char)*1100 );

	getcwd ( filename, 1000 );

	if ( strrchr(filename,'\\') )
		tmp = '\\';
	else
		tmp = '/';

	free ( filename );

	return tmp;

} /* end util_get_dir_seperator_character */

/******************************************************************************
**  Function   :  util_delete_delimeters                                       
**                                                                    
**  Objective  :  This function deletes the single or double quotes from the 
**                string
**                                                   
**  Parameters :  NONE
**                 
**  return     :  seperator character
** 
******************************************************************************/ 
int util_delete_delimeters ( char *string ) {

	char *tmp = 0;

	if ( string[0] == '"' || string[0] == '\'' ) {
		strcpy ( string, string+1 );
	}

	if ( (tmp=strstr(string, "\"")) || (tmp=strstr(string, "'")) )
		*tmp = '\0';

	return 0;

} /* end util_delete_delimeters */

/******************************************************************************
**  Function   :  util_all_whitespace
**                                                                    
**  Objective  :  This function tests a string for being all whitespace char
**                Basically copied from sqlite3 shell.c
**                                                   
**  Parameters :  z  - string to test
**                 
**  return     :  1 if all whitespace, 0 otherwise
** 
******************************************************************************/ 
int util_all_whitespace(const char *z){
  for(; *z; z++){
    if( isspace(*(unsigned char*)z) ) continue;
    if( *z=='/' && z[1]=='*' ){
      z += 2;
      while( *z && (*z!='*' || z[1]!='/') ){ z++; }
      if( *z==0 ) return 0;
      z++;
      continue;
    }
    if( *z=='-' && z[1]=='-' ){
      z += 2;
      while( *z && *z!='\n' ){ z++; }
      if( *z==0 ) return 1;
      continue;
    }
    return 0;
  }
  return 1;
}

int util_is_command_terminator(const char *zLine){
  while( isspace(*(unsigned char*)zLine) ){ zLine++; };
  if( zLine[0]=='/' && util_all_whitespace(&zLine[1]) ) return 1;  /* Oracle */
  if( tolower(zLine[0])=='g' && tolower(zLine[1])=='o'
         && util_all_whitespace(&zLine[2]) ){
    return 1;  /* SQL Server */
  }
  return 0;
}

int util_ends_with_semicolon(const char *z, int N){
  while( N>0 && isspace((unsigned char)z[N-1]) ){ N--; }
  return N>0 && z[N-1]==';';
}

/******************************************************************************
**  Function   :  util_search_and_replace
**                                                                    
**  Objective  :  This function replaces sub-string with replace string
**                                                   
**  Parameters :  string  - string to change
**                find    - string to find
**                replace - replacement string
**                 
**  return     :  1 if replacements made, 0 otherwise
** 
******************************************************************************/ 
int util_search_and_replace ( char *string, char *find, char *replace ) {

	char *original;
	char *place;
	char *find_loc;
	int status = 0;

	original = malloc ( strlen(string) + 1 );

	strcpy ( original, string );

	place = original;
	find_loc = strstr ( place, find );
	while ( *place != '\0' ) {
		if ( place == find_loc ) {
			status = 1;
			strncpy ( string, replace, strlen(replace) );
			place += strlen(find);
			string += strlen(replace);
			find_loc = strstr ( place, find );
		}
		else {
			*string = *place;
			++string;
			++place;
		}
	}
	*string = '\0';

	free ( original );

	return status;

} /* end util_search_and_replace */

/******************************************************************************
**  Function   :  util_replace_and_copy
**                                                                    
**  Objective  :  This function replaces sub-string with replace string. It
**                 allocates memory that must be freed by the caller
**                                                   
**  Parameters :  string1 - string to change
**                string2 - string to copy to
**                find    - string to find
**                replace - replacement string
**                 
**  return     :  new string
** 
******************************************************************************/ 
char * util_replace_and_copy ( char *string1, char *find, char *replace ) {

	char *original;
	char *original2;
	char *place;
	char *string2;
	char *find_loc;
	int status = 0;

	original = malloc ( strlen(string1) + 1 );
	string2 = malloc ( strlen(string1) + strlen(replace) + 1 );
	original2 = string2;

	strcpy ( string2, string1 );
	strcpy ( original, string2 );

	place = original;
	find_loc = strstr ( place, find );
	while ( *place != '\0' ) {
		if ( place == find_loc ) {
			status = 1;
			strncpy ( string2, replace, strlen(replace) );
			place += strlen(find);
			string2 += strlen(replace);
			find_loc = strstr ( place, find );
		}
		else {
			*string2 = *place;
			++string2;
			++place;
		}
	}
	*string2 = '\0';

	free ( original );

	return original2;

} /* end util_replace_and_copy */

/******************************************************************************
**  Function   :  util_make_path
**                            
**  Objective  :  This function creates a path from a directory and filename
**
**  Parameters :  dir     - directory to make path from
**                file    - file to make path from
**                path    - return string containing path
**                                                
**  return     :  Error code
**      
******************************************************************************/
int util_make_path ( char *dir, const char *file, char *path ) {

	/*
	 * Create Path
	 */
	sprintf ( path, "%s%c%s", dir, util_get_dir_seperator_character(), file ); 

	return 0;

} /* end util_make_path */

/******************************************************************************
**  Function   :  util_file_copy
**                            
**  Objective  :  This function copy a file
**
**  Parameters :  from    - file to copy from
**                to      - file to create
**                                                
**  return     :  Error code
**      
******************************************************************************/
int util_file_copy ( char *from, char *to ) {

	int nbytes;
	int status = -1;
	int fd1 = open(from,(O_RDONLY));
	int fd2 = open(to,(O_WRONLY | O_CREAT),S_IWRITE | S_IREAD);
	static char buffer[512];

	if (fd1 >= 0 && fd2 >= 0) {
		status = 0;
		while ((nbytes = read(fd1,buffer,512)) > 0)
			if (write(fd2,buffer,nbytes) != nbytes) {
				/* Write error */
				status = -1;
				break;       /* Write error */
			}

		/* Was there a read error? */
		if (nbytes == -1)
		status = -1;
	}

	if (fd1 >= 0)
		close(fd1);
	if (fd2 >= 0)
		close(fd2);

   return status;

} /* end util_file_copy */

/******************************************************************************
**  Function   :  util_create_program_path
**
**  Objective  :  This function creates the path to program config files
** 
**  Parameters :  path     - address to store returned file path
**                filename - filename to append to program directory
**                                               
**  return     :  path
******************************************************************************/ 
char * util_create_program_path ( char *path, char *filename ) {

	if ( strlen(g_program_path) )
		util_make_path ( g_program_path, filename, path );
	else
		sprintf ( path, "%s", filename );

	return path;

} /* end util_create_program_path */

/******************************************************************************
**  Function   :  util_get_crc     
**
**  Objective  :  This function gets the crc32 of the input file
** 
**  Parameters :  file - filename to get crc for   
**                                               
**  return     :  crc32
******************************************************************************/ 
unsigned long util_get_crc ( char *file ) {

	gzFile *rom;
	int bytes_read = 0;
	unsigned char *buffer;
	unsigned long crc;

	/* First read rom into a buffer and calculate bytes */
	rom = gzopen ( file, "rb" );

	if ( !rom )
		return 0;

	buffer = malloc ( 132000 );

	bytes_read = gzread ( rom, buffer, 132000 );

	gzclose ( rom );

	/* Now calculate CRC from buffer                    */
	crc = crc32(0L, buffer, bytes_read);

	free ( buffer );

	return crc;

} /* end util_get_crc */

/******************************************************************************
**  Function   :  util_get_all_hash
**
**  Objective  :  This function gets all the different hashes for the input file
** 
**  Parameters :  file - filename to get crc for   
**                crc  - crc checksum to return
**                md5  - md5 hash to return
**                sha1 - sha1 hash to return
**                                               
**  return     :  error code
******************************************************************************/ 
unsigned long util_get_all_hash ( char *file, unsigned long *crc, 
                                              unsigned char md5[16], 
                                              unsigned char sha1[20] ) {

	gzFile *rom;
	int bytes_read = 0;
	md5_context ctx;
	sha1_context ctxs;
	unsigned char *buffer;

	*crc = 0;

	/*
	 * Read the file into a buffer
	 */
	rom = gzopen ( file, "rb" );

	if ( !rom )
		return -1;

	buffer = malloc ( 132000 );

	bytes_read = gzread ( rom, buffer, 132000 );

	gzclose ( rom );

	/* 
	 * Grab the CRC
	 */
	*crc = crc32(0L, buffer, bytes_read);

	/* 
	 * Now the md5
	 */
	md5_starts( &ctx );
	md5_update( &ctx, buffer, bytes_read );
	md5_finish( &ctx, md5 );

	/* 
	 * Now the sha1
	 */
	sha1_starts( &ctxs );
	sha1_update( &ctxs, buffer, bytes_read );
	sha1_finish( &ctxs, sha1 );

	free ( buffer );

	return 0;

} /* end util_get_all_hash */

/******************************************************************************
**  Function   :  util_save_font_file
**
**  Objective  :  This function saves the font file to a new .h file
** 
**  Parameters :  file - filename to grab data out of
**                                               
**  return     :  crc32
******************************************************************************/ 
unsigned long util_save_font_file ( char *file ) {

	FILE *rom;
	FILE *dot_h_file;
	int bytes_on_line = 0;
	int byte_was_read = 1;
	unsigned char this_byte;
	char temp_string[10];

	/* First read rom into a buffer and calculate bytes */
	rom = fopen ( file, "rb" );
	dot_h_file = fopen ( "default_font.h", "w" );

	if ( !rom )
		return 0;

	fputs ( "unsigned char default_font[]={\n", dot_h_file );

	while ( byte_was_read ) {
		byte_was_read = fread ( &this_byte, 1, 1, rom );

		if ( !byte_was_read )
			break;
		sprintf ( temp_string, "%d,", this_byte );
		fputs ( temp_string, dot_h_file );
		if ( ++bytes_on_line == 16 ) {
			bytes_on_line = 0;
			fputs ( "\n", dot_h_file );
		}
	}

	fputs ( "};\n", dot_h_file );
	fclose ( rom );
	fclose ( dot_h_file );

	return 0;

} /* end util_save_font_file */

/******************************************************************************
**  Function   :  util_save_header_file
**
**  Objective  :  This function takes data from a file, compresses it and save
**                to a new header file
** 
**  Parameters :  file        - filename to grab data out of
**                header_file - name of file to save to
**                var_name    - name of variable to save in header
**                                               
**  return     :  error code
******************************************************************************/ 
int util_save_header_file ( char *file, char *header_file, char *var_name ) {

	FILE *rom;
	FILE *dot_h_file;
	int bytes_on_line = 0;
	int i = 0;
	char temp_string[10];
	unsigned char *src_buffer;
	unsigned char *dest_buffer;
	long src_len;
	long dest_len;

	/*
	 * Open file to read in and header file to write
	 */
	rom = fopen ( file, "rb" );
	dot_h_file = fopen ( header_file, "w" );

	if ( !rom )
		return 0;

	fputs ( "unsigned char ", dot_h_file );
	fputs ( var_name, dot_h_file );
	fputs ( "[]={\n", dot_h_file );

	/*
	 * Compress data
	 */
	src_buffer = malloc(100000);
	dest_buffer = malloc(20000);
	src_len = fread ( src_buffer, 1, 100000, rom );
	compress (dest_buffer, &dest_len, src_buffer, src_len);

	while ( i < dest_len ) {
		sprintf ( temp_string, "%d,", dest_buffer[i] );
		fputs ( temp_string, dot_h_file );
		if ( ++bytes_on_line == 16 ) {
			bytes_on_line = 0;
			fputs ( "\n", dot_h_file );
		}
		++i;
	}

	fputs ( "};\n", dot_h_file );
	fclose ( rom );
	fclose ( dot_h_file );

	free ( src_buffer );
	free ( dest_buffer );

	return 0;

} /* end util_save_header_file */
