/******************************************************************************
*
* FILENAME: logger.c
*
* DESCRIPTION:  This contains logging functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/10/06  bberlin      Creation
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "logger.h"
#include "util.h"

static FILE *fp = 0;

static char g_messages[16384] = {0};
static char g_tmp_string[1024] = {0};
static char g_logfile[1024] = "kat5200.log";

char * logger_get_string ( e_logger_type type, char *msg, char *source );

/******************************************************************************
**  Function   :  logger_init                                            
**                                                                    
**  Objective  :  This function initializes the logging
**                                                   
**  Parameters :  NONE
**                 
**  return     :  0 if success, otherwise failure
** 
******************************************************************************/ 
int logger_init ( void ) {

	char *tmp_string;

	util_create_program_path ( g_logfile, "kat5200.log" );

	/*
	 * Create Initial String and store to buffer
	 */
	tmp_string = logger_get_string ( LOG_INFO, "Starting kat5200 Emulator...", "" );
	strcat ( g_messages, tmp_string );

	/* 
	 * Open LogFile and write
	 */
	fp = fopen ( g_logfile, "w" );

	if ( !fp )
		return -1;

	fwrite ( tmp_string, sizeof(char), strlen(tmp_string), fp );
	fflush ( fp );

	return 0;

} /* end logger_init */

/******************************************************************************
**  Function   :  logger_exit                                            
**                                                                    
**  Objective  :  This function exits the logging
**                                                   
**  Parameters :  NONE
**                 
**  return     :  NONE
******************************************************************************/ 
void logger_exit ( void ) {

	if ( fp )
		fclose ( fp );

} /* end logger_exit */

/******************************************************************************
**  Function   :  logger_log_message
**                                                                    
**  Objective  :  This function appends a message to the log
**                                                   
**  Parameters :  type   - type of message (error,info)
**                msg    - string containing message to display
**                source - where message came from
**                 
**  return     :  NONE
******************************************************************************/ 
void logger_log_message ( e_logger_type type, char *msg, char *source ) {

	char *tmp_string;

	/*
	 * Create string and store in global message buffer
	 */
	tmp_string = logger_get_string ( type, msg, source );
	strcat ( g_messages, tmp_string );

	/*
	 * Write to file
	 */
	if ( fp ) {
		fwrite ( tmp_string, sizeof(char), strlen(tmp_string), fp );
		fflush ( fp );
	}

} /* end logger_log_message */

/******************************************************************************
**  Function   :  logger_get_string
**                                                                    
**  Objective  :  This function returns a string with information in it
**                                                   
**  Parameters :  NONE
**                 
**  return     :  message buffer pointer
******************************************************************************/ 
char * logger_get_string ( e_logger_type type, char *msg, char *source ) {

	char type_string[257];
	char *time_s;
	time_t rawtime;
	struct tm *timeinfo;

	/*
	 * Get Date and Time
	 */
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	/*
	 * Create Type String
	 */
	switch ( type ) {
		case LOG_ERROR: strcpy ( type_string, "ERROR" ); break;
		case LOG_INFO: strcpy ( type_string, "INFO " ); break;
		default: strcpy ( type_string, "UNKNOWN" ); break;
	}

	time_s = asctime(timeinfo);
	time_s[strlen(time_s)-1] = '\0';

	sprintf ( g_tmp_string, "[%s][%s] %s", time_s, type_string, msg );

	/*
	 * Add Source if necessary
	 */
	if ( sizeof(source) ) {
		sprintf ( type_string, " [%s]", source );
		strcat ( g_tmp_string, source );
	}
	strcat ( g_tmp_string, "\n" );

	return g_tmp_string;

} /* end logger_get_string */

/******************************************************************************
**  Function   :  logger_get_buffer
**                                                                    
**  Objective  :  This function returns a ptr to the messages buffer
**                                                   
**  Parameters :  NONE
**                 
**  return     :  message buffer pointer
******************************************************************************/ 
char * logger_get_buffer ( void ) {

	return g_messages;

} /* end logger_get_buffer */

