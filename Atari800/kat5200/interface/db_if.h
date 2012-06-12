/******************************************************************************
*
* FILENAME: db_if.h
*
* DESCRIPTION:  This contains function declartions for database functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   11/16/07  bberlin      Creation
******************************************************************************/
#ifndef db_if_h
#define db_if_h

#include "kconfig.h"

int db_if_init ( void );
int db_if_open ( char *file );
int db_if_close ( void );
int db_if_process_file ( const char *file );
int db_if_exec_sql ( const char *statement, int (*callback)(void*,int,char**,char**), void *data );
int db_if_exec_single_blob ( char *statement, void *blob, int length );
int db_if_query_single_blob ( char *statement, void *blob, int length );
int db_if_exec_single_blob ( char *statement, void *blob, int length );
int db_if_id_callback ( void *id, int argc, char **argv, char **azColName );
int db_if_insert_or_update ( char *check_sql, char *values, char *table, 
                               int *result );
int db_if_get_profile_names ( const char *sql, struct generic_node **nodeptr );
#endif
