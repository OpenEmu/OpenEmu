/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * String resource managment routines
 */

#ifndef TINSEL_STRRES_H
#define TINSEL_STRRES_H

#include "common/scummsys.h"
#include "tinsel/dw.h"

namespace Tinsel {

#define	STRINGS_PER_CHUNK	64	// number of strings per chunk in the language text files
#define	FIRST_STR_ID		1	// id number of first string in string table
#define	MAX_STRING_SIZE		255	// maximum size of a string in the resource table
#define	MAX_STRRES_SIZE		300000	// maximum size of string resource file

// Set if we're handling 2-byte characters.
extern bool g_bMultiByte;

/*----------------------------------------------------------------------*\
|*				Function Prototypes			*|
\*----------------------------------------------------------------------*/

/**
 * Called to load a resource file for a different language
 * @param newLang			The new language
 */
void ChangeLanguage(LANGUAGE newLang);

/**
 * Loads a string resource identified by id.
 * @param id			identifier of string to be loaded
 * @param pBuffer		points to buffer that receives the string
 * @param bufferMax		maximum number of chars to be copied to the buffer
 */
int LoadStringRes(int id, char *pBuffer, int bufferMax);

/**
 * Loads a string resource identified by id
 * @param id			identifier of string to be loaded
 * @param sub			sub-string number
 * @param pBuffer		points to buffer that receives the string
 * @param bufferMax		maximum number of chars to be copied to the buffer
 */
int LoadSubString(int id, int sub, char *pBuffer, int bufferMax);

int SubStringCount(int id);	// identifier of string to be tested

/**
 * Frees the text buffer allocated from ChangeLanguage()
 */
void FreeTextBuffer();

/**
 * Called from TINLIB.C from DeclareLanguage().
 */

void LanguageFacts(int language, SCNHANDLE hDescription, SCNHANDLE hFlagFilm);

/**
 * Gets the current subtitles language
 */
LANGUAGE TextLanguage();

/**
 * Gets the current voice language
 */
LANGUAGE SampleLanguage();

int NumberOfLanguages();
LANGUAGE NextLanguage(LANGUAGE thisOne);
LANGUAGE PrevLanguage(LANGUAGE thisOne);
SCNHANDLE LanguageDesc(LANGUAGE thisOne);
SCNHANDLE LanguageFlag(LANGUAGE thisOne);

} // End of namespace Tinsel

#endif
