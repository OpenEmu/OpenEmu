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

#ifndef COMMON_WINEXE_NE_H
#define COMMON_WINEXE_NE_H

#include "common/list.h"
#include "common/str.h"
#include "common/winexe.h"

namespace Common {

template<class T> class Array;
class SeekableReadStream;

/** The default Windows resources. */
enum NEResourceType {
	kNECursor = 0x8001,
	kNEBitmap = 0x8002,
	kNEIcon = 0x8003,
	kNEMenu = 0x8004,
	kNEDialog = 0x8005,
	kNEString = 0x8006,
	kNEFontDir = 0x8007,
	kNEFont = 0x8008,
	kNEAccelerator = 0x8009,
	kNERCData = 0x800A,
	kNEMessageTable = 0x800B,
	kNEGroupCursor = 0x800C,
	kNEGroupIcon = 0x800D,
	kNEVersion = 0x8010,
	kNEDlgInclude = 0x8011,
	kNEPlugPlay = 0x8013,
	kNEVXD = 0x8014,
	kNEAniCursor = 0x8015,
	kNEAniIcon = 0x8016,
	kNEHTML = 0x8017,
	kNEManifest = 0x8018
};

/**
 * A class able to load resources from a Windows New Executable, such
 * as cursors, bitmaps, and sounds.
 *
 * See http://en.wikipedia.org/wiki/New_Executable for more info.
 */
class NEResources {
public:
	NEResources();
	~NEResources();

	/** Clear all information. */
	void clear();

	/** Load from an EXE file. */
	bool loadFromEXE(const String &fileName);

	/** Load from a Windows compressed EXE file. */
	bool loadFromCompressedEXE(const String &fileName);

	/** Load from a stream. */
	bool loadFromEXE(SeekableReadStream *stream);

	/** Return a list of resources for a given type. */
	const Array<WinResourceID> getIDList(uint16 type) const;

	/** Return a stream to the specified resource (or 0 if non-existent). */
	SeekableReadStream *getResource(uint16 type, WinResourceID id);

private:
	/** A resource. */
	struct Resource {
		WinResourceID id;

		uint16 type; ///< Type of the resource.

		uint32 offset; ///< Offset within the EXE.
		uint32 size;   ///< Size of the data.

		uint16 flags;
		uint16 handle;
		uint16 usage;
	};

	SeekableReadStream *_exe;        ///< Current file.

	/** All resources. */
	List<Resource> _resources;

	/** Read the offset to the resource table. */
	uint32 getResourceTableOffset();
	/** Read the resource table. */
	bool readResourceTable(uint32 offset);

	/** Find a specific resource. */
	const Resource *findResource(uint16 type, WinResourceID id) const;

	/** Read a resource string. */
	static String getResourceString(SeekableReadStream &exe, uint32 offset);
};

} // End of namespace Common

#endif
