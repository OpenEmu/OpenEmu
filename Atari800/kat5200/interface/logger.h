/******************************************************************************
*
* FILENAME: logger.h
*
* DESCRIPTION:  This contains function declarations for logging functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/10/07  bberlin      Creation
******************************************************************************/
#ifndef logger_h
#define logger_h

typedef enum {
	LOG_INFO,
	LOG_ERROR,
} e_logger_type;

int logger_init ( void );
void logger_exit ( void );
void logger_log_message ( e_logger_type type, char *msg, char *source );
char * logger_get_buffer ( void );

#endif
