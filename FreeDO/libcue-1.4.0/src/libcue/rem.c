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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rem.h"

struct Rem {
	unsigned int cmt;
	char *value;
};

Rem*
rem_new(	void)
{
	Rem* new_rem = NULL;

	Rem rem[] = {
		{REM_DATE,			NULL},
		{REM_REPLAYGAIN_ALBUM_GAIN,	NULL},
		{REM_REPLAYGAIN_ALBUM_PEAK,	NULL},
		{REM_REPLAYGAIN_TRACK_GAIN,	NULL},
		{REM_REPLAYGAIN_TRACK_PEAK,	NULL},
		{REM_END,			NULL}
	};

	/* sizeof(rem) = number of elements in rem[] * sizeof(Rem) */
	new_rem = (Rem*)calloc(sizeof(rem) / sizeof(Rem), sizeof(Rem));
	if (new_rem == NULL)
		fprintf(stderr, "rem_new(): problem allocating memory\n");
	else
		memcpy(new_rem, rem, sizeof(rem));

	return new_rem;
}

void
rem_free(	Rem* rem)
{
	if (rem == NULL)
		return;

	Rem* ptr = rem;

	do
	{
		free(ptr->value);
	}
	while ((++ptr)->cmt != REM_END);

	free(rem);
}

int
rem_is_emtpy(	Rem* rem)
{
	if (rem == NULL)
		return 1;

	do
	{
		if (rem->value != NULL)
			return 0;
	} while ((++rem)->cmt != REM_END);

	return 1;
}

void
rem_set(	unsigned int cmt,
		char* value,
		Rem* rem)
{
	if (rem == NULL || value == NULL)
		return;

	do
	{
		if (rem->cmt == cmt)
		{
			free(rem->value);
			rem->value = strdup(value);
			return;
		}
	} while ((++rem)->cmt != REM_END);
}

char*
rem_get(	unsigned int cmt,
		Rem* rem)
{
	if (rem == NULL)
		return NULL;

	do
	{
		if (rem->cmt == cmt)
		{
			if (rem->value != NULL)
				return rem->value;
			else
				return NULL;
		}
	} while ((++rem)->cmt != REM_END);

	return NULL;
}

void
rem_dump(	Rem* rem)
{
	if (rem == NULL)
		return;

	do
	{
		fprintf(stdout, "REM %u: %s\n", rem->cmt, rem->value);
	} while ((++rem)->cmt != REM_END);
}
