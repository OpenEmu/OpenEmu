#ifndef COLOURS_EXTERNAL_H_
#define COLOURS_EXTERNAL_H_

#include <stdio.h> /* for FILENAME_MAX */

/* Contains an externally-loaded palette and its configuration */
typedef struct COLOURS_EXTERNAL_t {
	char filename[FILENAME_MAX]; /* Name of the palette file */
	int loaded; /* Ext. palette can be turned on/off by user */
	int adjust; /* Should the brightness/contrast/etc. be applied also to ext. palette? */
	unsigned char palette[768]; /* Raw RGB data for 256 colours */
} COLOURS_EXTERNAL_t;

/* Read a palette COLOURS from file; mark is as loaded. Return TRUE on
   success or FALSE on error. */
int COLOURS_EXTERNAL_Read(COLOURS_EXTERNAL_t *colours);
/* Unload external palette COLOURS. Old file name is not erased, so the
   same palette may be later re-loaded conveniently. */
void COLOURS_EXTERNAL_Remove(COLOURS_EXTERNAL_t *colours);
/* Read external palette COLOURS, from file named FILENAME. Same as
   COLOURS_EXTERNAL_READ, but with file name overridden by parameter. */
int COLOURS_EXTERNAL_ReadFilename(COLOURS_EXTERNAL_t *colours, char *filename);

#endif /* COLOURS_EXTERNAL_H_ */
