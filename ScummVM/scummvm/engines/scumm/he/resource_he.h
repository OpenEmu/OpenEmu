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

#ifndef SCUMM_HE_RESOURCE_HE_H
#define SCUMM_HE_RESOURCE_HE_H

#include "common/macresman.h"
#include "common/winexe_pe.h"

namespace Scumm {

class ScummEngine_v70he;

class ResExtractor {
public:
	ResExtractor(ScummEngine_v70he *scumm);
	virtual ~ResExtractor();

	void setCursor(int id);

protected:
	struct CachedCursor {
		bool valid;
		int id;
		byte *bitmap;
		int width, height;
		int hotspotX, hotspotY;
		uint32 lastUsed;
		byte *palette;
		int palSize;
	};

	Common::String _fileName;
	ScummEngine_v70he *_vm;

	virtual bool extractResource(int id, CachedCursor *cc) = 0;

private:
	enum {
		MAX_CACHED_CURSORS = 10
	};

	ResExtractor::CachedCursor *findCachedCursor(int id);
	ResExtractor::CachedCursor *getCachedCursorSlot();

	CachedCursor _cursorCache[MAX_CACHED_CURSORS];
};

class Win32ResExtractor : public ResExtractor {
public:
	Win32ResExtractor(ScummEngine_v70he *scumm);
	~Win32ResExtractor() {}

private:
	Common::PEResources _exe;

	bool extractResource(int id, CachedCursor *cc);
};

class MacResExtractor : public ResExtractor {
public:
	MacResExtractor(ScummEngine_v70he *scumm);
	~MacResExtractor() {}

private:
	Common::MacResManager *_resMgr;

	bool extractResource(int id, CachedCursor *cc);
};

} // End of namespace Scumm

#endif
