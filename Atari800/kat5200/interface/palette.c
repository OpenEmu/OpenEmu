/******************************************************************************
*
* FILENAME: palette.c
*
* DESCRIPTION:  This handles creation, saving, and recalling of color palettes
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.2     01/03/06  bberlin      Creation
* 0.4.0   06/12/06  bberlin      Change phase shift, saturation, and phase
*                                  multiplier of internal default palette
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "db_if.h"
#include "util.h"
#include "logger.h"
#include <stdio.h>

#define CLIP_VAR(x) \
  if (x > 0xff) \
    x = 0xff; \
  if (x < 0) \
    x = 0

static char msg[1100];

static int recall_flag;

/******************************************************************************
**  Function   :  save_palette
**                            
**  Objective  :  This function saves the pallete to the database
**
**  Parameters :  name      - name of profile to store pallete to 
**                pal       - array of int values to store
**                                                
**  return     :  Error code
**      
******************************************************************************/
int save_palette ( char *name, int *pal ) {

	int i, status;
	char statement[3000];
	char temp[20];

	util_search_and_replace ( name, "'", "''" );

	sprintf ( statement, "INSERT OR REPLACE INTO Palette VALUES ( '%s',NULL", name );
	for ( i = 0; i < 256; ++i ) {
		sprintf ( temp, ", %d", pal[i] );
		strcat ( statement, temp );
	}
	strcat ( statement, " )" );

	status = db_if_exec_sql ( statement, 0, 0 );

	return status;

} /* end save_palette */

/******************************************************************************
**  Function   :  recall_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  pal       - pointer to integer array for values
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                -1 if failure
******************************************************************************/ 
int recall_callback ( void *pal, int argc, char **argv, char **azColName ) {

	int i;
	int *p_pal = pal;

	if ( argc < 258 )
		return -1;

	recall_flag = 0;

	/*
	 * First column is name, we don't care about that
	 */
	for ( i = 2; i < argc; ++i ) {
		if ( argv[i] ) p_pal[i-2] = atoi ( argv[i] );
	}

	return 0;

} /* end recall_callback */

/******************************************************************************
**  Function   :  recall_palette
**                            
**  Objective  :  This function loads the palette from the database
**
**  Parameters :  name      - name of palette in database to recal
**                pal       - pointer to array to store values in
**                                                
**  return     :  Error code
**      
******************************************************************************/
int recall_palette ( const char *name, int *pal ) {

	char statement[257];
	int status;

	recall_flag = 1;

	sprintf ( statement, "SELECT * FROM Palette WHERE Name='%s'", name );
	status = db_if_exec_sql ( statement, recall_callback, pal );

	return recall_flag;

} /* end recall_palette */

void make_default_palette (int *pal) {

	int red[256], green[256], blue[256];
	int i, j;
	const double colf = 0.090;
	int colshift = 54;
	const double redf = 0.30;
	const double greenf = 0.59;
	const double bluef = 0.11;
	int r, g, b;
	int white = 0xf0;
	double angle;
	int y, r1, g1, b1;

  for (i = 0; i < 0x10; i++)
  {
    if (i == 0)
    {
      r = g = b = 0;
    }
    else
    {
      angle = 3.14159 * ((double) i * (5.0 / 34.0) - (double) colshift * 0.01);
      r = (int)((colf / redf) * cos(angle) * (white));
      g = (int)((colf / greenf) * cos(angle + 3.14159 * (2.0 / 3)) * (white));
      b = (int)((colf / bluef) * cos(angle + 3.14159 * (4.0 / 3)) * (white));
    }
    for (j = 0; j < 0x10; j++)
    {

      y = (white * j) / 0xf;
      r1 = y + r;
      g1 = y + g;
      b1 = y + b;
      CLIP_VAR(r1);
      CLIP_VAR(g1);
      CLIP_VAR(b1);
      red[i * 16 + j] = r1;
      green[i * 16 + j] = g1;
      blue[i * 16 + j] = b1;
    }
  }
  for (i = 0; i < 0x100; i++)
  {
    pal[i] = (red[i] << 16) + (green[i] << 8) + (blue[i] << 0);
  }
} /* end make_default_palette */

/******************************************************************************
**  Function   :  convert_palette
**                            
**  Objective  :  This function converts the file format from atari800 to db
**
**  Parameters :  old_file  - name of file containing atari800 binary array
**                new_name  - name of palette to store
**                                                
**  return     :  Error code
**      
******************************************************************************/
int convert_palette ( char *old_file, char *new_name ) {

	int pal[256];
	int i,j,c;
	FILE *fp;

	/*
	 * Open the atari800 palette and get values
	 */
	if ((fp = fopen(old_file,"rb")) == NULL) {
		sprintf ( msg, "Cound not open Atari800 Pallete file: %s", old_file );
		logger_log_message ( LOG_ERROR, msg, "" );
		return -1;
	}
	for (i = 0; i < 256; ++i ) {
		pal[i] = 0;
		for (j = 16; j >= 0; j -= 8) {
			c = fgetc(fp);
			if (c == EOF) {
				fclose(fp);
				return -1;
			}
			pal[i] |= c << j;
		}
	}
	fclose(fp);

	/*
	 * Now save to the new file as kat5200 palette
	 */
	if ( save_palette ( new_name, pal ) )
		return -1;

	return 0;

} /* end convert_palette */

/******************************************************************************
**  Function   :  save_a800_pal
**                            
**  Objective  :  This function saves the palette as an Atari800 style file
**
**  Parameters :  file  - name of file to save Atari 800 palette to
**                pal   - palette to save
**                                                
**  return     :  Error code
**      
******************************************************************************/
int save_a800_pal ( char *file, int *pal ) {

	int i,j;
	FILE *fp;

	/*
	 * Open the atari800 palette and get values
	 */
	if ((fp = fopen(file,"wb")) == NULL) {
		sprintf ( msg, "Cound not open Atari800 Pallete file: %s", file );
		logger_log_message ( LOG_ERROR, msg, "" );
		return -1;
	}
	for (i = 0; i < 256; ++i ) {
		for (j = 16; j >= 0; j -= 8) {
			fputc(pal[i]>>j, fp);
		}
	}
	fclose(fp);

	return 0;

} /* end save_a800_pal */

/******************************************************************************
**  Function   :  convert_kat_palette
**                            
**  Objective  :  This function converts the file format from xml to atari800
**
**  Parameters :  old_name  - name of palette containing kat5200 palette
**                new_file  - name of file to store value to
**                                                
**  return     :  Error code
**      
******************************************************************************/
int convert_kat_palette ( char *old_name, char *new_file ) {

	int pal[256];

	/*
	 * Now save to the new file as kat5200 palette
	 */
	if ( recall_palette ( old_name, pal ) )
		return -1;

	if ( save_a800_pal ( new_file, pal ) )
		return -1;

	return 0;

} /* end convert_kat_palette */
