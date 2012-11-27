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

#include "tinsel/dw.h"
#include "tinsel/drives.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "tinsel/scn.h"
#include "common/file.h"
#include "common/endian.h"
#include "common/textconsole.h"

#include "gui/message.h"

namespace Tinsel {

// FIXME: Avoid non-const global vars

#ifdef DEBUG
// Diagnostic number
int g_newestString;
#endif

// buffer for resource strings
static uint8 *g_textBuffer = 0;

static struct {
	bool		bPresent;
	const char	*szStem;
	SCNHANDLE	hDescription;
	SCNHANDLE	hFlagFilm;

} g_languages[NUM_LANGUAGES] = {

	{ false, "English",	0, 0 },
	{ false, "French",	0, 0 },
	{ false, "German",	0, 0 },
	{ false, "Italian",	0, 0 },
	{ false, "Spanish",	0, 0 },
	{ false, "Hebrew",	0, 0 },
	{ false, "Magyar",	0, 0 },
	{ false, "Japanese",0, 0 },
	{ false, "US",		0, 0 }
};


// Set if we're handling 2-byte characters.
bool g_bMultiByte = false;

LANGUAGE g_textLanguage, g_sampleLanguage = TXT_ENGLISH;

//----------------- LOCAL DEFINES ----------------------------

#define languageExtension	".txt"
#define indexExtension		".idx"
#define sampleExtension		".smp"

//----------------- FUNCTIONS --------------------------------

/**
 * Called to load a resource file for a different language
 * @param newLang			The new language
 */
void ChangeLanguage(LANGUAGE newLang) {
	TinselFile f;
	uint32 textLen = 0;	// length of buffer

	g_textLanguage = newLang;
	g_sampleLanguage = newLang;

	// free the previous buffer
	free(g_textBuffer);
	g_textBuffer = NULL;

	// Try and open the specified language file. If it fails, and the language
	// isn't English, try falling back on opening 'english.txt' - some foreign
	// language versions reused it rather than their proper filename
	if (!f.open(_vm->getTextFile(newLang))) {
		if ((newLang == TXT_ENGLISH) || !f.open(_vm->getTextFile(TXT_ENGLISH))) {
			char buf[50];
			sprintf(buf, CANNOT_FIND_FILE, _vm->getTextFile(newLang));
			GUI::MessageDialog dialog(buf, "OK");
			dialog.runModal();

			error(CANNOT_FIND_FILE, _vm->getTextFile(newLang));
		}
	}

	// Check whether the file is compressed or not -  for compressed files the
	// first long is the filelength and for uncompressed files it is the chunk
	// identifier
	textLen = f.readUint32();
	if (f.eos() || f.err())
		error(FILE_IS_CORRUPT, _vm->getTextFile(newLang));

	if (textLen == CHUNK_STRING || textLen == CHUNK_MBSTRING) {
		// the file is uncompressed

		g_bMultiByte = (textLen == CHUNK_MBSTRING);

		// get length of uncompressed file
		textLen = f.size();
		f.seek(0, SEEK_SET);	// Set to beginning of file

		if (g_textBuffer == NULL) {
			// allocate a text buffer for the strings
			g_textBuffer = (uint8 *)malloc(textLen);

			// make sure memory allocated
			assert(g_textBuffer);
		}

		// load data
		if (f.read(g_textBuffer, textLen) != textLen)
			// file must be corrupt if we get to here
			error(FILE_IS_CORRUPT, _vm->getTextFile(newLang));

		// close the file
		f.close();
	} else {	// the file must be compressed
		error("Compression handling has been removed");
	}
}

/**
 * FindStringBase
 */
static byte *FindStringBase(int id) {
	// base of string resource table
	byte *pText = g_textBuffer;

	// For Tinsel 0, Ids are decremented by 1
	if (TinselV0)
		--id;

	// index into text resource file
	uint32 index = 0;

	// number of chunks to skip
	int chunkSkip = id / STRINGS_PER_CHUNK;

	// number of strings to skip when in the correct chunk
	int strSkip = id % STRINGS_PER_CHUNK;

	// skip to the correct chunk
	while (chunkSkip-- != 0) {
		// make sure chunk id is correct
		assert(READ_LE_UINT32(pText + index) == CHUNK_STRING || READ_LE_UINT32(pText + index) == CHUNK_MBSTRING);

		if (READ_LE_UINT32(pText + index + sizeof(uint32)) == 0) {
			// string does not exist
			return NULL;
		}

		// get index to next chunk
		index = READ_LE_UINT32(pText + index + sizeof(uint32));
	}

	// skip over chunk id and offset
	index += (2 * sizeof(uint32));

	// pointer to strings
	pText = pText + index;

	// skip to the correct string
	while (strSkip-- != 0) {
		// skip to next string

		if (!TinselV2 || ((*pText & 0x80) == 0)) {
			// Tinsel 1, or string of length < 128
			pText += *pText + 1;
		} else if (*pText == 0x80) {
			// string of length 128 - 255
			pText++;		// skip control byte
			pText += *pText + 1;
		} else if (*pText == 0x90) {
			// string of length 256 - 511
			pText++;		// skip control byte
			pText += *pText + 1 + 256;
		} else {	// multiple string
			int subCount;

			subCount = *pText & ~0x80;
			pText++;		// skip control byte

			// skip prior sub-strings
			while (subCount--) {
				// skip control byte, if there is one
				if (*pText == 0x80) {
					pText++;
					pText += *pText + 1;
				} else if (*pText == 0x90) {
					pText++;
					pText += *pText + 1 + 256;
				} else
					pText += *pText + 1;
			}
		}
	}

	return pText;
}


/**
 * Loads a string resource identified by id.
 * @param id			identifier of string to be loaded
 * @param pBuffer		points to buffer that receives the string
 * @param bufferMax		maximum number of chars to be copied to the buffer
 */
int LoadStringResource(int id, int sub, char *pBuffer, int bufferMax) {
	int len;	// length of string

	byte *pText = FindStringBase(id);

	if (pText == NULL) {
		strcpy(pBuffer, "!! HIGH STRING !!");
		return 0;
	}

	if (!TinselV2 || ((*pText & 0x80) == 0)) {
		// get length of string
		len = *pText;
	} else if (*pText == 0x80) {
		// string of length 128 - 255
		pText++;		// skip control byte

		// get length of string
		len = *pText;
	} else if (*pText == 0x90) {
		// string of length 128 - 255
		pText++;		// skip control byte

		// get length of string
		len = *pText + 256;
	} else {
		// multiple string
		pText++;		// skip control byte

		// skip prior sub-strings
		while (sub--) {
			// skip control byte, if there is one
			if (*pText == 0x80) {
				pText++;
				pText += *pText + 1;
			} else if (*pText == 0x90) {
				pText++;
				pText += *pText + 1 + 256;
			} else
				pText += *pText + 1;
		}
		// skip control byte, if there is one
		if (*pText == 0x80) {
			pText++;

			// get length of string
			len = *pText;
		} else if (*pText == 0x90) {
			pText++;

			// get length of string
			len = *pText + 256;
		} else {
			// get length of string
			len = *pText;
		}
	}

	if (len) {
		// the string exists

		// copy the string to the buffer
		if (len < bufferMax) {
			memcpy(pBuffer, pText + 1, len);

			// null terminate
			pBuffer[len] = 0;

			// number of chars copied
			return len + 1;
		} else {
			memcpy(pBuffer, pText + 1, bufferMax - 1);

			// null terminate
			pBuffer[bufferMax - 1] = 0;

			// number of chars copied
			return bufferMax;
		}
	}

	// TEMPORARY DIRTY BODGE
	strcpy(pBuffer, "!! NULL STRING !!");

	// string does not exist
	return 0;
}

int LoadStringRes(int id, char *pBuffer, int bufferMax) {
	return LoadStringResource(id, 0, pBuffer, bufferMax);
}

/**
 * Loads a string resource identified by id
 * @param id			identifier of string to be loaded
 * @param sub			sub-string number
 * @param pBuffer		points to buffer that receives the string
 * @param bufferMax		maximum number of chars to be copied to the buffer
 */
int LoadSubString(int id, int sub, char *pBuffer, int bufferMax) {
	return LoadStringResource(id, sub, pBuffer, bufferMax);
}

/**
 * SubStringCount
 * @param id			Identifier of string to be tested
 */
int SubStringCount(int id) {
	byte *pText;

	pText = FindStringBase(id);

	if (pText == NULL)
		return 0;

	if ((*pText & 0x80) == 0 || *pText == 0x80 || *pText == 0x90) {
		// string of length < 128 or string of length 128 - 255
		// or of length 256 - 511
		return 1;
	} else
		return (*pText & ~0x80);
}


void FreeTextBuffer() {
	free(g_textBuffer);
	g_textBuffer = NULL;
}

/**
 * Called from TINLIB.C from DeclareLanguage().
 */

void LanguageFacts(int language, SCNHANDLE hDescription, SCNHANDLE hFlagFilm) {
	assert(language >= 0 && language < NUM_LANGUAGES);

	g_languages[language].hDescription = hDescription;
	g_languages[language].hFlagFilm	 = hFlagFilm;
}

/**
 * Gets the current subtitles language
 */
LANGUAGE TextLanguage() {
	return g_textLanguage;
}

/**
 * Gets the current voice language
 */
LANGUAGE SampleLanguage() {
	return g_sampleLanguage;
}

int NumberOfLanguages() {
	int i, count;

	for (i = 0, count = 0; i < NUM_LANGUAGES; i++) {
		if (g_languages[i].bPresent)
			count++;
	}
	return count;
}

LANGUAGE NextLanguage(LANGUAGE thisOne) {
	int i;

	for (i = thisOne+1; i < NUM_LANGUAGES; i++) {
		if (g_languages[i].bPresent)
			return (LANGUAGE)i;
	}

	for (i = 0; i < thisOne; i++) {
		if (g_languages[i].bPresent)
			return (LANGUAGE)i;
	}

	// No others!
	return thisOne;
}

LANGUAGE PrevLanguage(LANGUAGE thisOne) {
	int i;

	for (i = thisOne-1; i >= 0; i--) {
		if (g_languages[i].bPresent)
			return (LANGUAGE)i;
	}

	for (i = NUM_LANGUAGES-1; i > thisOne; i--) {
		if (g_languages[i].bPresent)
			return (LANGUAGE)i;
	}

	// No others!
	return thisOne;
}

SCNHANDLE LanguageDesc(LANGUAGE thisOne) {
	return g_languages[thisOne].hDescription;
}

SCNHANDLE LanguageFlag(LANGUAGE thisOne) {
	return g_languages[thisOne].hFlagFilm;
}

} // End of namespace Tinsel
