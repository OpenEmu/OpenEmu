/******************************************************************************
*
* FILENAME: palette.h
*
* DESCRIPTION:  This contains function and struct declarations for color 
*               palette functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.2     01/06/06  bberlin      Creation
******************************************************************************/
#ifndef palette_h
#define palette_h

int save_palette ( char *name, int *pal );
int recall_palette ( const char *name, int *pal );
void make_default_palette (int *pal);
int convert_palette ( char *old_file, char *new_name );
int convert_kat_palette ( char *old_name, char *new_file );
int make_hi_res_artifacts ( int hue, int lum0, int lum1, int *art0, int *art1 );

#endif
