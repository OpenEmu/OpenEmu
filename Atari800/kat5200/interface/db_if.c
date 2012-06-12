/******************************************************************************
*
* FILENAME: db_if.c
*
* DESCRIPTION:  This interfaces with the sqlite database
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   11/16/07  bberlin      Creation
******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include "logger.h"
#include "kconfig.h"
#include "util.h"
#include "db_if.h"

static sqlite3 *db = 0;
static int status = 0;
static char log_msg[4096];
static struct generic_node *p_node_static;

/******************************************************************************
**  Function   :  db_if_init
**                            
**  Objective  :  This function opens the database for use
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int db_if_init ( void ) {

	char title[500];

	strcpy ( title, "kat5200.db3" );
	util_set_file_to_program_dir ( title );	

	return db_if_open ( title );
}

/******************************************************************************
**  Function   :  db_if_open
**                            
**  Objective  :  This function opens the database for use
**
**  Parameters :  file      - filename to open
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int db_if_open ( char *file ) {

	status = sqlite3_open ( file, &db );
	if ( status ) { 
		strcpy ( log_msg, file );
		status = sqlite3_open ( util_create_program_path(log_msg, file), &db );
		if ( status ) { 
			sprintf ( log_msg, "db_if_open: Error w/ DB File \"%s\"", file );
			logger_log_message ( LOG_ERROR, log_msg, "" );
			return -1; 
		}
	}

	return 0;

} /* end db_if_open */

/******************************************************************************
**  Function   :  db_if_close
**                            
**  Objective  :  This function closes the connection to the database
**
**  Parameters :  NONE
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int db_if_close ( void ) {

	sqlite3_close ( db );

	return 0;

} /* end db_if_close */

/******************************************************************************
**  Function   :  db_if_process_file
**                            
**  Objective  :  This function processes the SQL file.  Basically copied
**                from shell.c from sqlite3.
**
**  Parameters :  file - name of file to process
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int db_if_process_file ( const char *file ) {

	FILE *fp = 0;
	char *zLine = 0;
	char *zSql = 0;
	char *zErrMsg = 0;
	int errCnt = 0;
	int nLine;
	int eol, n, rc;
	int nSql = 0;
	int lineno = 0;
	int startline = 0;

	fp = fopen(file, "rb");

	if ( !fp ) {
		sprintf ( log_msg, "db_if_process_file: Unable to open file: \"%s\"", file );
		logger_log_message ( LOG_ERROR, log_msg, "" );
		return -1;
	}

	sprintf ( log_msg, "db_if_process_file: Opened SQL file: \"%s\"", file );
	logger_log_message ( LOG_INFO, log_msg, "" );

	while( errCnt==0 ) {
		free(zLine);

		/*
		 * Get a Line
		 */
		nLine = 100;
		zLine = malloc( nLine );
		if( zLine==0 ) return -1;
		n = 0;
		eol = 0;
		while( !eol ){
		  if( n+100>nLine ){
		    nLine = nLine*2 + 100;
		    zLine = realloc(zLine, nLine);
		    if( zLine==0 ) break;
		  }
		  if( fgets(&zLine[n], nLine - n, fp)==0 ){
		    if( n==0 ){
		      free(zLine);
			  zLine = 0;
		      break;
		    }
		    zLine[n] = 0;
		    eol = 1;
		    break;
		  }
		  while( zLine[n] ){ n++; }
		  if( n>0 && zLine[n-1]=='\n' ){
		    n--;
		    zLine[n] = 0;
		    eol = 1;
		  }
		}

		if ( zLine )
			zLine = realloc( zLine, n+1 );

		/*
		 * Test For EOF
		 */
	    if( zLine==0 ){
			break; 
		}

		lineno++;

		/*
		 * Test For empty or whitespace line
		 */
		if( (zSql==0 || zSql[0]==0) && util_all_whitespace(zLine) ) continue;

		/*
		 * Don't do meta commands for now
		 */
		if( zLine && zLine[0]=='.' && nSql==0 ){
			/*
			rc = do_meta_command(zLine, p);
			if( rc==2 ){
			break;
			}else if( rc ){
			errCnt++;
			}
			*/
			continue;
		}

		if( util_is_command_terminator(zLine) ){
			memcpy(zLine,";",2);
		}

		if( zSql==0 ){
			int i;
			for(i=0; zLine[i] && isspace((unsigned char)zLine[i]); i++){}
			if( zLine[i]!=0 ){
				nSql = strlen(zLine);
				zSql = malloc( nSql+1 );
				if( zSql==0 ){
					fprintf(stderr, "out of memory\n");
					exit(1);
				}
				memcpy(zSql, zLine, nSql+1);
				startline = lineno;
			}
		}
		else{
			int len = strlen(zLine);
			zSql = realloc( zSql, nSql + len + 2 );
			if( zSql==0 ){
				fprintf(stderr,"%s: out of memory!\n", "db_process_file");
				exit(1);
			}
			zSql[nSql++] = '\n';
			memcpy(&zSql[nSql], zLine, len+1);
			nSql += len;
		}

		/*
		 * If this is the end of a SQL command, execute
		 */
		if( zSql && util_ends_with_semicolon(zSql, nSql) && sqlite3_complete(zSql) ){
			rc = sqlite3_exec(db, zSql, NULL, NULL, &zErrMsg);
			if( rc || zErrMsg ){
				sprintf ( log_msg, "db_if_process_file: SQL error near line %d", startline );
				logger_log_message ( LOG_ERROR, log_msg, "" );
				if( zErrMsg!=0 ){
					sqlite3_free(zErrMsg);
					zErrMsg = 0;
				}
				else{
					sprintf ( log_msg, "db_if_process_file: %s", sqlite3_errmsg(db) );
					logger_log_message ( LOG_ERROR, log_msg, "" );
				}
				errCnt++;
			}
			free(zSql);
			zSql = 0;
			nSql = 0;
		}

	} /* end while processing file */

	if( zSql ){
		if( !util_all_whitespace(zSql) ) {
			sprintf ( log_msg, "db_if_process_file: Incomplete SQL" );
			logger_log_message ( LOG_ERROR, log_msg, "" );
		}
		free(zSql);
	}
	free(zLine);

	sprintf ( log_msg, "db_if_process_file: Processing SQL file complete - Errors : %d", errCnt );
	logger_log_message ( LOG_INFO, log_msg, "" );

	return errCnt;

} /* end db_if_process_file */

/******************************************************************************
**  Function   :  db_if_exec_sql
**                            
**  Objective  :  This function executes an SQL statement.  This is for INSERTs
**                and UPDATEs as there is no callback.
**
**  Parameters :  statement - string containing the SQL statement
**                callback  - pointer to function to call for query results
**                data      - pointer to data to pass to callback
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int db_if_exec_sql ( const char *statement, int (*callback)(void*,int,char**,char**), void *data ) {

	char *zErrMsg = 0;

	status = sqlite3_exec ( db, statement, callback, data, &zErrMsg );

	if ( status != SQLITE_OK ) {
		sprintf ( log_msg, "db_if_exec_sql: Statement: \"%s\"", statement );
		logger_log_message ( LOG_ERROR, log_msg, "" );
		sprintf ( log_msg, "db_if_exec_sql: Error: \"%s\"", zErrMsg );
		logger_log_message ( LOG_ERROR, log_msg, "" );
	}

	sqlite3_free ( zErrMsg );

	return status;

} /* end db_if_exec_sql */

/******************************************************************************
**  Function   :  db_if_exec_single_blob
**                            
**  Objective  :  This function executes an SQL statement for a single BLOB
**
**  Parameters :  statement - string containing the SQL statement
**                blob      - binary data
**                length    - amount of binary data to store
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int db_if_exec_single_blob ( char *statement, void *blob, int length ) {

	sqlite3_stmt *stmt;
	int status;

	status = sqlite3_prepare_v2 ( db, statement, strlen(statement), &stmt, 0 );
	status = sqlite3_bind_blob( stmt, 1, blob, length, SQLITE_STATIC );
	status = sqlite3_step ( stmt );
	status = sqlite3_finalize( stmt );

	return status;

} /* end db_if_exec_single_blob */

/******************************************************************************
**  Function   :  db_if_query_single_blob
**                            
**  Objective  :  This function executes an SQL query for a single BLOB
**
**  Parameters :  statement - string containing the SQL statement
**                blob      - pointer to store binary data
**                length    - amount of binary data to store
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/
int db_if_query_single_blob ( char *statement, void *blob, int length ) {

	sqlite3_stmt *stmt;
	int status, r_length, max;
	const void *result;

	status = sqlite3_prepare_v2 ( db, statement, strlen(statement), &stmt, 0 );

	if ( status )
		return status;

	status = sqlite3_step ( stmt );

	result = sqlite3_column_blob( stmt, 0 );
	r_length = sqlite3_column_bytes( stmt, 0 );
	
	if ( r_length > length )
		max = length;
	else
		max = r_length;

	memcpy ( blob, result, max );

	status = sqlite3_finalize( stmt );

	return status;

} /* end db_if_exec_single_blob */

/******************************************************************************
**  Function   :  db_if_id_callback                                            
**                                                                    
**  Objective  :  This function is a generic callback for getting a single int
**                                                   
**  Parameters :  id        - pointer to integer to store info into
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int db_if_id_callback ( void *id, int argc, char **argv, char **azColName ) {

	int *p_id = id;

	if ( argc < 1 )
		return -1;

	if ( argv[0] ) *p_id = atoi ( argv[0] );

	return 0;

} /* end db_if_id_callback */

/******************************************************************************
**  Function   :  db_if_update_callback                                            
**                                                                    
**  Objective  :  This function creates the SET columns for the UPDATE stmt
**                                                   
**  Parameters :  values    - string with the comma seperated values
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int db_if_update_callback ( void *values, int argc, char **argv, char **azColName ) {

	char *p_values = values;
	char string[1024];
	char temp[257];
	char *single_value;
	int i;

	if ( argc < 2 )
		return -1;

	single_value = strtok ( p_values, "," );
	sprintf ( string, "SET %s = %s", azColName[1], single_value );

	for ( i = 2; i < argc; ++i ) {
		single_value = strtok ( NULL, "," );
		sprintf ( temp, ", %s =%s", azColName[i], single_value );
		strcat ( string, temp );
	}

	sprintf ( temp, " WHERE %s = %s", azColName[0], argv[0] );
	strcat ( string, temp );

	strcpy ( (char *)values, string );

	return 0;

} /* end db_if_update_callback */

/******************************************************************************
**  Function   :  db_if_insert_or_update                                            
**                                                                    
**  Objective  :  This function checks the id sql for and id and applies an
**                 INSERT or UPDATE based on the result.  ID must be the first
**                 column of the table.
**                                                   
**  Parameters :  check_sql - the SQL statement to find the ID
**                values    - values to enter in table (without ID)
**                table     - name of table containing data
**                result    - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int db_if_insert_or_update ( char *check_sql, char *values, char *table, 
                               int *result ) {

	int status;
	int id = 0;
	char statement[1024];

	status = db_if_exec_sql ( check_sql, db_if_id_callback, &id );

	/*
	 * if ID is true, then we only want to update (entry exists)
	 */
	if ( id ) {
		sprintf ( statement, "SELECT * FROM %s WHERE ROWID = %d", table, id );
		status = db_if_exec_sql ( statement, db_if_update_callback, values );

		sprintf ( statement, "UPDATE %s %s;", table, values );
		status = db_if_exec_sql ( statement, 0, 0 );
	}

	/*
	 * Else INSERT it in the table
	 */
	else {
		sprintf ( statement, "INSERT INTO %s VALUES ( NULL, %s )", table, values );
		status = db_if_exec_sql ( statement, 0, 0 );

		sprintf ( statement, "SELECT max(ROWID) FROM %s", table );
		status = db_if_exec_sql ( statement, db_if_id_callback, &id );
	}

	*result = id;

	return 0;

} /* end db_if_insert_or_update */

/******************************************************************************
**  Function   :  db_if_profile_name_callback                                            
**                                                                    
**  Objective  :  This function is a generic callback for getting a single int
**                                                   
**  Parameters :  nodeptr   - pointer to integer to store info into
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int db_if_profile_name_callback ( void *nodeptr, int argc, char **argv, char **azColName ) {

	struct generic_node **p_nodeptr = nodeptr;
	struct generic_node *p_node = *p_nodeptr;

	if ( !p_node_static ) {
		*p_nodeptr = malloc ( sizeof(struct generic_node) );
		p_node_static = *p_nodeptr;
	}

	else {
		p_node_static->next = malloc ( sizeof(struct generic_node) );
		p_node_static = p_node_static->next;
	}

	strcpy ( p_node_static->name, argv[0] );

	if ( argc > 1 )
		p_node_static->value = strtoul ( argv[1], NULL, 10 );

	p_node_static->next = 0;

	return 0;

} /* end db_if_profile_name_callback */

/******************************************************************************
**  Function   :  db_if_get_profile_names1
**                                                                    
**  Objective  :  This function gets names based on the input SQL.  Assumes
**                 name will be first column
**                                                   
**  Parameters :  sql       - the SQL statement to find the ID
**                nodeptr   - address of pointer to node 
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int db_if_get_profile_names ( const char *sql, struct generic_node **nodeptr ) {

	*nodeptr = 0;

	p_node_static = 0;

	status = db_if_exec_sql ( sql, db_if_profile_name_callback, nodeptr );

	return 0;

} /* end db_if_get_profile_names */

