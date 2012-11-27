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

#ifndef SCUMM_SCRIPT_V3_H
#define SCUMM_SCRIPT_V3_H

#include "scumm/scumm_v4.h"

namespace Scumm {

/**
 * Engine for version 3 SCUMM games; GF_SMALL_NAMES is always set for these.
 */
class ScummEngine_v3 : public ScummEngine_v4 {
public:
	ScummEngine_v3(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v3();

	virtual void resetScumm();

protected:
	virtual void setupOpcodes();

	virtual void readRoomsOffsets();
	virtual void loadCharset(int no);

	virtual void processKeyboard(Common::KeyState lastKeyHit);

	/* Version 3 script opcodes */
	void o3_setBoxFlags();
	void o3_waitForActor();
	void o3_waitForSentence();
};

/**
 * Engine for old format version 3 SCUMM games; GF_OLD_BUNDLE is always set for these.
 */
class ScummEngine_v3old : public ScummEngine_v3 {
public:
	ScummEngine_v3old(OSystem *syst, const DetectorResult &dr);

protected:
	virtual int readResTypeList(ResType type);
	virtual void readIndexFile();
	virtual void setupRoomSubBlocks();
	virtual void resetRoomSubBlocks();
	virtual void resetRoomObjects();
};


} // End of namespace Scumm

#endif
