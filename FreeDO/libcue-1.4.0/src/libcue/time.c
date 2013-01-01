/*
 * time.c -- time functions
 *
 * Copyright (C) 2004, 2005, 2006, 2007 Svend Sorensen
 * For license terms, see the file COPYING in this distribution.
 */

#include "time.h"
#include <stdio.h>
#include <stdlib.h>

long time_msf_to_frame(int m, int s, int f)
{
	return (m * 60 + s) * 75 + f;
}

void msf_frame_to_msf(long frame, int *m, int *s, int *f)
{
        *f = frame % 75;	/* 0 <= frames <= 74 */
        frame /= 75;
        *s = frame % 60;	/* 0 <= seconds <= 59 */
        frame /= 60;
        *m = frame;		/* 0 <= minutes */
}

void time_frame_to_msf(long frame, int *m, int *s, int *f)
{
	*f = frame % 75;           /* 0 <= frames <= 74 */
	frame /= 75;
	*s = frame % 60;          /* 0 <= seconds <= 59 */
	frame /= 60;
	*m = frame;               /* 0 <= minutes */
}

/* print frame in mm:ss:ff format */
char *time_frame_to_mmssff(long f)
{
	static char msf[9];
	int minutes, seconds, frames;

	msf_frame_to_msf(f, &minutes, &seconds, &frames);
	sprintf(msf, "%02d:%02d:%02d", minutes, seconds, frames);

	return msf;
}
