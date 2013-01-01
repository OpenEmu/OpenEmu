/*
 * Copyright (c) 2004, 2005, 2006, 2007, Svend Sorensen
 * Copyright (c) 2009, 2010 Jochen Keil
 * For license terms, see the file COPYING in this distribution.
 */

/* references: MMC-3 draft revsion - 10g */

#ifndef CDTEXT_H
#define CDTEXT_H

#include <stdio.h>

/* cdtext pack type indicators */
enum Pti {
	PTI_TITLE,	/* title of album or track titles */
	PTI_PERFORMER,	/* name(s) of the performer(s) */
	PTI_SONGWRITER,	/* name(s) of the songwriter(s) */
	PTI_COMPOSER,	/* name(s) of the composer(s) */
	PTI_ARRANGER,	/* name(s) of the arranger(s) */
	PTI_MESSAGE,	/* message(s) from the content provider and/or artist */
	PTI_DISC_ID,	/* (binary) disc identification information */
	PTI_GENRE,	/* (binary) genre identification and genre information */
	PTI_TOC_INFO1,	/* (binary) table of contents information */
	PTI_TOC_INFO2,	/* (binary) second table of contents information */
	PTI_RESERVED1,	/* reserved */
	PTI_RESERVED2,	/* reserved */
	PTI_RESERVED3,	/* reserved */
	PTI_RESERVED4,	/* reserved for content provider only */
	PTI_UPC_ISRC,	/* UPC/EAN code of the album and ISRC code of each track */
	PTI_SIZE_INFO,	/* (binary) size information of the block */
	PTI_END		/* terminating PTI (for stepping through PTIs) */
};

enum PtiFormat {
	FORMAT_CHAR,		/* single or double byte character string */
	FORMAT_BINARY		/* binary data */
};

typedef struct Cdtext Cdtext;

/* return a pointer to a new Cdtext */
Cdtext *cdtext_init(void);

/* release a Cdtext */
void cdtext_delete(Cdtext *cdtext);

/* returns non-zero if there are no CD-TEXT fields set, zero otherwise */
int cdtext_is_empty(Cdtext *cdtext);

/* set CD-TEXT field to value for PTI pti */
void cdtext_set(int pti, char *value, Cdtext *cdtext);

/* returns pointer to CD-TEXT value for PTI pti */
char *cdtext_get(int pti, Cdtext *cdtext);

/*
 * returns appropriate string for PTI pti
 * if istrack is zero, UPC/EAN string will be returned for PTI_UPC_ISRC
 * othwise ISRC string will be returned
 */
const char *cdtext_get_key(int pti, int istrack);

/*
 * dump all cdtext info
 * in human readable format (for debugging)
 */
void cdtext_dump(Cdtext *cdtext, int istrack);

#endif
