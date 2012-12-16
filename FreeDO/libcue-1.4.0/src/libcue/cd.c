/*
 * Copyright (c) 2004, 2005, 2006, 2007, Svend Sorensen
 * Copyright (c) 2009, 2010 Jochen Keil
 * For license terms, see the file COPYING in this distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cd.h"

typedef struct Data Data;
struct Data {
	int type;			/* DataType */
	char *name;			/* data source name */
	long start;			/* start time for data */
	long length;			/* length of data */
};

struct Track {
	Data zero_pre;			/* pre-gap generated with zero data */
	Data file;			/* track data file */
	Data zero_post;			/* post-gap generated with zero data */
	int mode;			/* track mode */
	int sub_mode;			/* sub-channel mode */
	int flags;			/* flags */
	char *isrc;			/* IRSC Code (5.22.4) 12 bytes */
	Cdtext *cdtext;			/* CD-TEXT */
	Rem* rem;
	int nindex;			/* number of indexes */
	long index[MAXINDEX];		/* indexes (in frames) (5.29.2.5)
					 * relative to start of track
					 * index[0] should always be zero */
};

struct Cd {
	int mode;			/* disc mode */
	char *catalog;			/* Media Catalog Number (5.22.3) */
	char *cdtextfile;		/* Filename of CDText File */
	Cdtext *cdtext;			/* CD-TEXT */
	Rem* rem;
	int ntrack;			/* number of tracks in album */
	Track *track[MAXTRACK];		/* array of tracks */
};

Cd *cd_init(void)
{
	Cd *cd = NULL;
	cd = malloc(sizeof(Cd));

	if(NULL == cd) {
		fprintf(stderr, "unable to create cd\n");
	} else {
		cd->mode = MODE_CD_DA;
		cd->catalog = NULL;
		cd->cdtextfile = NULL;
		cd->cdtext = cdtext_init();
		cd->rem = rem_new();
		cd->ntrack = 0;
	}

	return cd;
}

void track_delete(struct Track* track)
{
	if (track != NULL)
	{
		cdtext_delete(track_get_cdtext(track));

		rem_free(track_get_rem(track));

		free(track->isrc);

		free(track->zero_pre.name);

		free(track->zero_post.name);

		free(track->file.name);

		free(track);
	}
}

void cd_delete(struct Cd* cd)
{
	int i = 0;

	if (cd != NULL)
	{
		free(cd->catalog);

		free(cd->cdtextfile);

		for (i = 0; i < cd->ntrack; i++)
			track_delete(cd->track[i]);

		cdtext_delete(cd_get_cdtext(cd));

		rem_free(cd_get_rem(cd));

		free(cd);
	}
}

Track *track_init(void)
{
	Track *track = NULL;
	track = malloc(sizeof(Track));

	if (NULL == track) {
		fprintf(stderr, "unable to create track\n");
	} else {
		track->zero_pre.type = DATA_ZERO;
		track->zero_pre.name = NULL;
		track->zero_pre.start = 0;
		track->zero_pre.length = 0;

		track->file.type = DATA_AUDIO;
		track->file.name = NULL;
		track->file.start = 0;
		track->file.length = 0;

		track->zero_post.type = DATA_ZERO;
		track->zero_post.name = NULL;
		track->zero_post.start = 0;
		track->zero_post.length = 0;

		track->mode = MODE_AUDIO;
		track->sub_mode = SUB_MODE_RW;
		track->flags = FLAG_NONE;
		track->isrc = NULL;
		track->cdtext = cdtext_init();
		track->rem = rem_new();
		track->nindex = 0;
	}

	return track;
}

/*
 * cd structure functions
 */
void cd_set_mode(Cd *cd, int mode)
{
	cd->mode = mode;
}

int cd_get_mode(Cd *cd)
{
	return cd->mode;
}

void cd_set_catalog(Cd *cd, char *catalog)
{
	if (cd->catalog)
		free(cd->catalog);

	cd->catalog = strdup(catalog);
}

void cd_set_cdtextfile(Cd *cd, char *cdtextfile)
{
	if (cd->cdtextfile)
		free(cd->cdtextfile);

	cd->cdtextfile = strdup(cdtextfile);
}

char *cd_get_cdtextfile(Cd *cd)
{
	return cd->cdtextfile;
}

Cdtext *cd_get_cdtext(Cd *cd)
{
	if (cd != NULL)
		return cd->cdtext;
	else
		return NULL;
}

Rem*
cd_get_rem(	Cd* cd)
{
	if (cd != NULL)
		return cd->rem;
	else
		return NULL;
}

Track *cd_add_track(Cd *cd)
{
	if (MAXTRACK - 1 > cd->ntrack)
		cd->ntrack++;
	else
		fprintf(stderr, "too many tracks\n");

	/* this will reinit last track if there were too many */
	cd->track[cd->ntrack - 1] = track_init();

	return cd->track[cd->ntrack - 1];
}


int cd_get_ntrack(Cd *cd)
{
	return cd->ntrack;
}

Track *cd_get_track(Cd *cd, int i)
{
	if ((0 < i) && (i <= cd->ntrack) && (cd != NULL))
		return cd->track[i - 1];
	else
		return NULL;
}

/*
 * track structure functions
 */

void track_set_filename(Track *track, char *filename)
{
	if (track->file.name)
		free(track->file.name);

	track->file.name = strdup(filename);
}

char *track_get_filename(Track *track)
{
	return track->file.name;
}

void track_set_start(Track *track, long start)
{
	track->file.start = start;
}

long track_get_start(Track *track)
{
	return track->file.start;
}

void track_set_length(Track *track, long length)
{
	track->file.length = length;
}

long track_get_length(Track *track)
{
	return track->file.length;
}

void track_set_mode(Track *track, int mode)
{
	track->mode = mode;
}

int track_get_mode(Track *track)
{
	return track->mode;
}

void track_set_sub_mode(Track *track, int sub_mode)
{
	track->sub_mode = sub_mode;
}

int track_get_sub_mode(Track *track)
{
	return track->sub_mode;
}

void track_set_flag(Track *track, int flag)
{
	track->flags |= flag;
}

void track_clear_flag(Track *track, int flag)
{
	track->flags &= ~flag;
}

int track_is_set_flag(Track *track, int flag)
{
	return track->flags & flag;
}

void track_set_zero_pre(Track *track, long length)
{
	track->zero_pre.length = length;
}

long track_get_zero_pre(Track *track)
{
	return track->zero_pre.length;
}

void track_set_zero_post(Track *track, long length)
{
	track->zero_post.length = length;
}

long track_get_zero_post(Track *track)
{
	return track->zero_post.length;
}
void track_set_isrc(Track *track, char *isrc)
{
	if (track->isrc)
		free(track->isrc);

	track->isrc = strdup(isrc);
}

char *track_get_isrc(Track *track)
{
	return track->isrc;
}

Cdtext *track_get_cdtext(Track *track)
{
	if (track != NULL)
		return track->cdtext;
	else
		return NULL;
}

Rem*
track_get_rem(	Track* track)
{
	if (track != NULL)
		return track->rem;
	else
		return NULL;
}

void track_add_index(Track *track, long ind)
{
	if (MAXTRACK - 1 > track->nindex)
		track->nindex++;
	else
		fprintf(stderr, "too many indexes\n");

	/* this will overwrite last index if there were too many */
	track->index[track->nindex - 1] = ind;
}

int track_get_nindex(Track *track)
{
	return track->nindex;
}

long track_get_index(Track *track, int i)
{
	if ((0 <= i) && (i < track->nindex))
		return track->index[i];

	return -1;
}

/*
 * dump cd information
 */
static void cd_track_dump(Track *track)
{
	int i;

	printf("zero_pre: %ld\n", track->zero_pre.length);
	printf("filename: %s\n", track->file.name);
	printf("start: %ld\n", track->file.start);
	printf("length: %ld\n", track->file.length);
	printf("zero_post: %ld\n", track->zero_post.length);
	printf("mode: %d\n", track->mode);
	printf("sub_mode: %d\n", track->sub_mode);
	printf("flags: 0x%x\n", track->flags);
	printf("isrc: %s\n", track->isrc);
	printf("indexes: %d\n", track->nindex);

	for (i = 0; i < track->nindex; ++i)
		printf("index %d: %ld\n", i, track->index[i]);

	if (NULL != track->cdtext) {
		printf("cdtext:\n");
		cdtext_dump(track->cdtext, 1);
	}

	if (track->rem != NULL)
	{
		fprintf(stdout, "rem:\n");
		rem_dump(track->rem);
	}
}

void cd_dump(Cd *cd)
{
	int i;

	printf("Disc Info\n");
	printf("mode: %d\n", cd->mode);
	printf("catalog: %s\n", cd->catalog);
	printf("cdtextfile: %s\n", cd->cdtextfile);
	if (NULL != cd->cdtext) {
		printf("cdtext:\n");
		cdtext_dump(cd->cdtext, 0);
	}

	if (cd->rem != NULL)
	{
		fprintf(stdout, "rem:\n");
		rem_dump(cd->rem);
	}

	for (i = 0; i < cd->ntrack; ++i) {
		printf("Track %d Info\n", i + 1);
		cd_track_dump(cd->track[i]);
	}
}
