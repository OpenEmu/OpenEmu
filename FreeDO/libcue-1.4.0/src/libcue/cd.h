/*
 * Copyright (c) 2004, 2005, 2006, 2007, Svend Sorensen
 * Copyright (c) 2009, 2010 Jochen Keil
 * For license terms, see the file COPYING in this distribution.
 */

/* references: MMC-3 draft revsion - 10g */

#ifndef CD_H
#define CD_H

#include "cdtext.h"
#include "rem.h"

#define MAXTRACK	99	/* Red Book track limit */
#define MAXINDEX	99	/* Red Book index limit */

/*
 * disc modes
 * DATA FORM OF MAIN DATA (5.29.2.8)
 */
enum DiscMode {
	MODE_CD_DA,		/* CD-DA */
	MODE_CD_ROM,		/* CD-ROM mode 1 */
	MODE_CD_ROM_XA		/* CD-ROM XA and CD-I */
};

/*
 * track modes
 * 5.29.2.8 DATA FORM OF MAIN DATA
 * Table 350 - Data Block Type Codes
 */
enum TrackMode {
	MODE_AUDIO,		/* 2352 byte block length */
	MODE_MODE1,		/* 2048 byte block length */
	MODE_MODE1_RAW,		/* 2352 byte block length */
	MODE_MODE2,		/* 2336 byte block length */
	MODE_MODE2_FORM1,	/* 2048 byte block length */
	MODE_MODE2_FORM2,	/* 2324 byte block length */
	MODE_MODE2_FORM_MIX,	/* 2332 byte block length */
	MODE_MODE2_RAW		/* 2352 byte block length */
};

/*
 * sub-channel mode
 * 5.29.2.13 Data Form of Sub-channel
 * NOTE: not sure if this applies to cue files
 */
enum TrackSubMode {
	SUB_MODE_RW,		/* RAW Data */
	SUB_MODE_RW_RAW		/* PACK DATA (written R-W */
};

/*
 * track flags
 * Q Sub-channel Control Field (4.2.3.3, 5.29.2.2)
 */
enum TrackFlag {
	FLAG_NONE		= 0x00,	/* no flags set */
	FLAG_PRE_EMPHASIS	= 0x01,	/* audio recorded with pre-emphasis */
	FLAG_COPY_PERMITTED	= 0x02,	/* digital copy permitted */
	FLAG_DATA		= 0x04,	/* data track */
	FLAG_FOUR_CHANNEL	= 0x08,	/* 4 audio channels */
	FLAG_SCMS		= 0x10,	/* SCMS (not Q Sub-ch.) (5.29.2.7) */
	FLAG_ANY		= 0xff	/* any flags set */
};

enum DataType {
	DATA_AUDIO,
	DATA_DATA,
	DATA_FIFO,
	DATA_ZERO
};

/* ADTs */
typedef struct Cd Cd;
typedef struct Track Track;


/* return pointer to CD structure */
Cd *cd_init(void);

Track *track_init(void);

void track_delete(struct Track* track);

void cd_delete(struct Cd* cd);

void cd_dump(Cd *cd);

/*
 * Cd functions
 */

void cd_set_mode(Cd *cd, int mode);
int cd_get_mode(Cd *cd);

void cd_set_catalog(Cd *cd, char *catalog);
char *cd_get_catalog(Cd *cd);

void cd_set_cdtextfile(Cd *cd, char *cdtextfile);
char *cd_get_cdtextfile(Cd *cd);

/*
 * return pointer to cd's Cdtext
 */
Cdtext *cd_get_cdtext(Cd *cd);

Rem* cd_get_rem(Cd* cd);

/*
 * add a new track to cd, increment number of tracks
 * and return pointer to new track
 */
Track *cd_add_track(Cd *cd);

/*
 * return number of tracks in cd
 */
int cd_get_ntrack(Cd *cd);

Track *cd_get_track(Cd *cd, int i);

/*
 * Track functions
 */

/* filename of data file */
void track_set_filename(Track *track, char *filename);
char *track_get_filename(Track *track);

/* track start is starting position in data file */
void track_set_start(Track *track, long start);
long track_get_start(Track *track);

/* track length is length of data file to use */
void track_set_length(Track *track, long length);
long track_get_length(Track *track);

/* see enum TrackMode */
void track_set_mode(Track *track, int mode);
int track_get_mode(Track *track);

/* see enum TrackSubMode */
void track_set_sub_mode(Track *track, int sub_mode);
int track_get_sub_mode(Track *track);

/* see enum TrackFlag */
void track_set_flag(Track *track, int flag);
void track_clear_flag(Track *track, int flag);
int track_is_set_flag(Track *track, int flag);

/* zero data pregap */
void track_set_zero_pre(Track *track, long length);
long track_get_zero_pre(Track *track);

/* zero data postgap */
void track_set_zero_post(Track *track, long length);
long track_get_zero_post(Track *track);

void track_set_isrc(Track *track, char *isrc);
char *track_get_isrc(Track *track);

Cdtext *track_get_cdtext(Track *track);

Rem* track_get_rem(Track* track);

void track_add_index(Track *track, long index);
int track_get_nindex(Track *track);
long track_get_index(Track *track, int i);
#endif
