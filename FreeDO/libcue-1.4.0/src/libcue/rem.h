/*
 * Copyright (c) 2009, 2010 Jochen Keil
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY Jochen Keil ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jochen Keil BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef REM_H
#define REM_H

#include <stdio.h>

enum Cmt {
	REM_DATE,	/* date of cd/track */
	REM_REPLAYGAIN_ALBUM_GAIN,
	REM_REPLAYGAIN_ALBUM_PEAK,
	REM_REPLAYGAIN_TRACK_GAIN,
	REM_REPLAYGAIN_TRACK_PEAK,
	REM_END		/* terminating REM (for stepping through REMs) */
};

typedef struct Rem Rem;

/**
 * return new allocated rem struct
 */
Rem*
rem_new(	void);

/**
 * free rem struct
 */
void
rem_free(	Rem*);

/**
 * return true if allocated rem struct is empty
 */
int
rem_is_emtpy(	Rem*);

/**
 * set value of rem comment
 * @param unsigned int: enum of rem comment
 */
void
rem_set(	unsigned int,
		char*,
		Rem*);

/**
 * return pointer to value for rem comment
 * @param unsigned int: enum of rem comment
 */
char*
rem_get(	unsigned int,
		Rem*);

/**
 * dump all rem comments in human readable form
 * @param bool: if track: true; else false
 */
void
rem_dump(	Rem*);

#endif
