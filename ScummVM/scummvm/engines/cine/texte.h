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
 */

#ifndef CINE_TEXTE_H
#define CINE_TEXTE_H


#include "common/scummsys.h"

namespace Cine {

typedef char CommandeType[20];

// Number of characters in a font
#define NUM_FONT_CHARS 256

#define FONT_WIDTH 16
#define FONT_HEIGHT 8

// Used for choosing between font's data and font's mask
#define FONT_DATA 0
#define FONT_MASK 1

struct CharacterEntry {
	byte characterIdx;
	byte characterWidth;
};

struct TextHandler {
	byte textTable[NUM_FONT_CHARS][2][FONT_WIDTH *FONT_HEIGHT];
	CharacterEntry fontParamTable[NUM_FONT_CHARS];
};

extern const char *const *failureMessages;
extern const CommandeType *defaultActionCommand;
extern const CommandeType *systemMenu;
extern const CommandeType *confirmMenu;
extern const char *const *otherMessages;
extern const char *defaultCommandPreposition;
extern const char *const *commandPrepositionTable;

void loadTextData(const char *filename);
void loadErrmessDat(const char *fname);
void freeErrmessDat();
void loadPoldatDat(const char *fname);

int fitLine(const char *ptr, int maxWidth, int &words, int &width);

} // End of namespace Cine

#endif
