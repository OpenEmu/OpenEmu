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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/hashmap.h"
#include "common/file.h"
#include "common/str.h"

#ifndef MOHAWK_RESOURCE_H
#define MOHAWK_RESOURCE_H

namespace Mohawk {

// Main FourCC's
#define ID_MHWK MKTAG('M','H','W','K') // Main FourCC
#define ID_RSRC MKTAG('R','S','R','C') // Resource Directory Tag

// Myst Resource FourCC's
#define ID_CLRC MKTAG('C','L','R','C') // Cursor Hotspots
#define ID_EXIT MKTAG('E','X','I','T') // Card Exit Scripts
#define ID_HINT MKTAG('H','I','N','T') // Cursor Hints
#define ID_INIT MKTAG('I','N','I','T') // Card Entrance Scripts
#define ID_MSND MKTAG('M','S','N','D') // Standard Mohawk Sound
#define ID_RLST MKTAG('R','L','S','T') // Resource List, Specifies HotSpots
#define ID_RSFL MKTAG('R','S','F','L') // ??? (system.dat only)
#define ID_VIEW MKTAG('V','I','E','W') // Card Details
#define ID_WDIB MKTAG('W','D','I','B') // LZ-Compressed Windows Bitmap

// Myst Masterpiece Edition Resource FourCC's (In addition to Myst FourCC's)
#define ID_HELP MKTAG('H','E','L','P') // Help Chunk
#define ID_MJMP MKTAG('M','J','M','P') // MSND Jumps (To reduce MSND duplication)
#define ID_PICT MKTAG('P','I','C','T') // JPEG/PICT/WDIB Image

// Riven Resource FourCC's
#define ID_BLST MKTAG('B','L','S','T') // Card Hotspot Enabling Lists
#define ID_CARD MKTAG('C','A','R','D') // Card Scripts
#define ID_FLST MKTAG('F','L','S','T') // Card SFXE Lists
#define ID_HSPT MKTAG('H','S','P','T') // Card Hotspots
#define ID_MLST MKTAG('M','L','S','T') // Card Movie Lists
#define ID_NAME MKTAG('N','A','M','E') // Object Names
#define ID_PLST MKTAG('P','L','S','T') // Card Picture Lists
#define ID_RMAP MKTAG('R','M','A','P') // Card Codes
#define ID_SFXE MKTAG('S','F','X','E') // Water Effect Animations
#define ID_SLST MKTAG('S','L','S','T') // Card Ambient Sound Lists
#define ID_TMOV MKTAG('t','M','O','V') // QuickTime Movie

// Riven Saved Game FourCC's
#define ID_VARS MKTAG('V','A','R','S') // Variable Values
#define ID_VERS MKTAG('V','E','R','S') // Version Info
#define ID_ZIPS MKTAG('Z','I','P','S') // Zip Mode Status

// Zoombini Resource FourCC's
#define ID_SND  MKTAG( 0 ,'S','N','D') // Standard Mohawk Sound
#define ID_CURS MKTAG('C','U','R','S') // Cursor
#define ID_SCRB MKTAG('S','C','R','B') // Feature Script
#define ID_SCRS MKTAG('S','C','R','S') // Snoid Script
#define ID_NODE MKTAG('N','O','D','E') // Walk Node
#define ID_PATH MKTAG('P','A','T','H') // Walk Path
#define ID_SHPL MKTAG('S','H','P','L') // Shape List

// Living Books Resource FourCC's
#define ID_TCUR MKTAG('t','C','U','R') // Cursor
#define ID_BITL MKTAG('B','I','T','L') // Book Item List
#define ID_CTBL MKTAG('C','T','B','L') // Color Table
#define ID_SCRP MKTAG('S','C','R','P') // Script
#define ID_SPR  MKTAG('S','P','R','#') // Sprite?
#define ID_VRSN MKTAG('V','R','S','N') // Version
#define ID_ANI  MKTAG('A','N','I',' ') // Animation
#define ID_SHP  MKTAG('S','H','P','#') // Shape
#define ID_WAV  MKTAG('W','A','V',' ') // Old Sound Resource
#define ID_BMAP MKTAG('B','M','A','P') // Old Mohawk Bitmap
#define ID_BCOD MKTAG('B','C','O','D') // Book Code

// JamesMath Resource FourCC's
#define ID_TANM MKTAG('t','A','N','M') // Animation?
#define ID_TMFO MKTAG('t','M','F','O') // ???

// CSTime Resource FourCC's
#define ID_CINF MKTAG('C','I','N','F') // Case Info
#define ID_CONV MKTAG('C','O','N','V') // Conversation
#define ID_HOTS MKTAG('H','O','T','S') // Hotspot
#define ID_INVO MKTAG('I','N','V','O') // Inventory Object
#define ID_QARS MKTAG('Q','A','R','S') // Question and Responses
#define ID_SCEN MKTAG('S','C','E','N') // Scene
#define ID_STRI MKTAG('S','T','R','I') // String Entry?

// Mohawk Wave Tags
#define ID_WAVE MKTAG('W','A','V','E') // Game Sound (Third Tag)
#define ID_ADPC MKTAG('A','D','P','C') // Game Sound Chunk
#define ID_DATA MKTAG('D','a','t','a') // Game Sound Chunk
#define ID_CUE  MKTAG('C','u','e','#') // Game Sound Chunk

// Mohawk MIDI Tags
#define ID_MIDI MKTAG('M','I','D','I') // Game Sound (Third Tag), instead of WAVE
#define ID_PRG  MKTAG('P','r','g','#') // MIDI Patch

// Common Resource FourCC's
#define ID_TBMP MKTAG('t','B','M','P') // Standard Mohawk Bitmap
#define ID_TWAV MKTAG('t','W','A','V') // Standard Mohawk Sound
#define ID_TPAL MKTAG('t','P','A','L') // Standard Mohawk Palette
#define ID_TCNT MKTAG('t','C','N','T') // Shape Count (CSWorld, CSAmtrak, JamesMath)
#define ID_TSCR MKTAG('t','S','C','R') // Script (CSWorld, CSAmtrak, Treehouse)
#define ID_STRL MKTAG('S','T','R','L') // String List (Zoombini, CSWorld, CSAmtrak)
#define ID_TBMH MKTAG('t','B','M','H') // Standard Mohawk Bitmap
#define ID_TMID MKTAG('t','M','I','D') // Standard Mohawk MIDI
#define ID_REGS MKTAG('R','E','G','S') // Registration Data - Shape Offsets (Zoombini, Treehouse)
#define ID_BYTS MKTAG('B','Y','T','S') // Byte Array? (Used as Database Entry in CSWorld, CSAmtrak)
#define ID_INTS MKTAG('I','N','T','S') // uint16 Array? (CSWorld, CSAmtrak)
#define ID_BBOX MKTAG('B','B','O','X') // Boxes? (CSWorld, CSAmtrak)
#define ID_SYSX MKTAG('S','Y','S','X') // MIDI Sysex


class Archive {
public:
	Archive();
	virtual ~Archive();

	bool openFile(const Common::String &fileName);
	virtual bool openStream(Common::SeekableReadStream *stream) = 0;
	void close();

	bool isOpen() const { return _stream != 0; }

	bool hasResource(uint32 tag, uint16 id) const;
	bool hasResource(uint32 tag, const Common::String &resName) const;
	Common::SeekableReadStream *getResource(uint32 tag, uint16 id);
	uint32 getOffset(uint32 tag, uint16 id) const;
	uint16 findResourceID(uint32 tag, const Common::String &resName) const;
	Common::String getName(uint32 tag, uint16 id) const;

	Common::Array<uint32> getResourceTypeList() const;
	Common::Array<uint16> getResourceIDList(uint32 type) const;

protected:
	Common::SeekableReadStream *_stream;

	struct Resource {
		uint32 offset;
		uint32 size;
		Common::String name;
	};

	typedef Common::HashMap<uint16, Resource> ResourceMap;
	typedef Common::HashMap<uint32, ResourceMap> TypeMap;
	TypeMap _types;
};

class MohawkArchive : public Archive {
public:
	MohawkArchive() : Archive() {}
	~MohawkArchive() {}

	bool openStream(Common::SeekableReadStream *stream);
};

class LivingBooksArchive_v1 : public Archive {
public:
	LivingBooksArchive_v1() : Archive() {}
	~LivingBooksArchive_v1() {}

	bool openStream(Common::SeekableReadStream *stream);
};

class DOSArchive_v2 : public Archive {
public:
	DOSArchive_v2() : Archive() {}
	~DOSArchive_v2() {}

	bool openStream(Common::SeekableReadStream *stream);
};

} // End of namespace Mohawk

#endif
