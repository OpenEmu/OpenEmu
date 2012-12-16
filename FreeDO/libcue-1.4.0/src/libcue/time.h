/*
 * time.h -- time declarations
 *
 * Copyright (C) 2004, 2005, 2006, 2007 Svend Sorensen
 * For license terms, see the file COPYING in this distribution.
 */

#ifndef TIME_H
#define TIME_H

void msf_frame_to_msf(long frame, int *m, int *s, int *f);
long time_msf_to_frame(int m, int s, int f);
long time_mmssff_to_frame(char *mmssff);
void time_frame_to_msf(long frame, int *m, int *s, int *f);
char *time_frame_to_mmssff(long f);

#endif
