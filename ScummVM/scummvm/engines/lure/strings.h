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

#ifndef LURE_STRINGS_H
#define LURE_STRINGS_H

#include "lure/luredefs.h"
#include "lure/memory.h"

namespace Lure {

class CharacterEntry {
public:
	uint8 _numBits;
	uint32 _sequence;
	char _ascii;

	CharacterEntry(uint8 numBits, uint32 sequence, char ascii): _numBits(numBits),
		_sequence(sequence), _ascii(ascii) {}
};

#define MAX_NUM_CHARS 218

class StringData {
private:
	MemoryBlock *_strings[3];
	MemoryBlock *_names;
	CharacterEntry *_chars[MAX_NUM_CHARS];
	uint8 _numChars;
	byte *_srcPos;
	byte *_stringTable;
	byte _bitMask;

	void add(const char *sequence, char ascii);
	bool initPosition(uint16 stringId);
	char readCharacter();
	byte readBit();
public:
	StringData();
	~StringData();
	static StringData &getReference();

	void getString(uint16 stringId, char *dest, const char *hotspotName, const char *characterName,
		int hotspotArticle = 0, int characterArticle = 0);
	void getString(uint16 stringId, char *dest) {
		getString(stringId, dest, NULL, NULL);
	}
	char *getName(uint8 nameIndex);
};

} // namespace Lure

#endif
