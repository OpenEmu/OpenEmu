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

#ifndef COMMON_WINEXE_PE_H
#define COMMON_WINEXE_PE_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/winexe.h"

namespace Common {

template<class T> class Array;
class SeekableReadStream;

/** The default Windows PE resources. */
enum PEResourceType {
	kPECursor =       0x01,
	kPEBitmap =       0x02,
	kPEIcon =         0x03,
	kPEMenu =         0x04,
	kPEDialog =       0x05,
	kPEString =       0x06,
	kPEFontDir =      0x07,
	kPEFont =         0x08,
	kPEAccelerator =  0x09,
	kPERCData =       0x0A,
	kPEMessageTable = 0x0B,
	kPEGroupCursor =  0x0C,
	kPEGroupIcon =    0x0E,
	kPEVersion =      0x10,
	kPEDlgInclude =   0x11,
	kPEPlugPlay =     0x13,
	kPEVXD =          0x14,
	kPEAniCursor =    0x15,
	kPEAniIcon =      0x16
};

/**
 * A class able to load resources from a Windows Portable Executable, such
 * as cursors, bitmaps, and sounds.
 */
class PEResources {
public:
	PEResources();
	~PEResources();

	/** Clear all information. */
	void clear();

	/** Load from an EXE file. */
	bool loadFromEXE(const String &fileName);

	/** Load from a stream. */
	bool loadFromEXE(SeekableReadStream *stream);

	/** Return a list of resource types. */
	const Array<WinResourceID> getTypeList() const;

	/** Return a list of names for a given type. */
	const Array<WinResourceID> getNameList(const WinResourceID &type) const;

	/** Return a list of languages for a given type and name. */
	const Array<WinResourceID> getLangList(const WinResourceID &type, const WinResourceID &name) const;

	/** Return a stream to the specified resource, taking the first language found (or 0 if non-existent). */
	SeekableReadStream *getResource(const WinResourceID &type, const WinResourceID &name);

	/** Return a stream to the specified resource (or 0 if non-existent). */
	SeekableReadStream *getResource(const WinResourceID &type, const WinResourceID &name, const WinResourceID &lang);

private:
	struct Section {
		uint32 virtualAddress;
		uint32 size;
		uint32 offset;
	};

	HashMap<String, Section, IgnoreCase_Hash, IgnoreCase_EqualTo> _sections;

	SeekableReadStream *_exe;

	void parseResourceLevel(Section &section, uint32 offset, int level);
	WinResourceID _curType, _curName, _curLang;

	struct Resource {
		uint32 offset;
		uint32 size;
	};

	typedef HashMap<WinResourceID, Resource, WinResourceID_Hash, WinResourceID_EqualTo> LangMap;
	typedef HashMap<WinResourceID,  LangMap, WinResourceID_Hash, WinResourceID_EqualTo> NameMap;
	typedef HashMap<WinResourceID,  NameMap, WinResourceID_Hash, WinResourceID_EqualTo> TypeMap;

	TypeMap _resources;
};

} // End of namespace Common

#endif
