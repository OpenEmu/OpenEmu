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

#ifndef SCUMM_SCRIPT_V4_H
#define SCUMM_SCRIPT_V4_H

#include "scumm/scumm_v5.h"

namespace Scumm {

/**
 * Engine for version 4 SCUMM games; GF_SMALL_HEADER is always set for these.
 */
class ScummEngine_v4 : public ScummEngine_v5 {
	friend class ScummEngine_v5;
public:

	/**
	 * Prepared savegame used by the orginal save/load dialog.
	 * Must be valid as long as the savescreen is active. As we are not
	 * notified when the savescreen is closed, memory is only freed on a game
	 * reset, at the destruction of the engine or when the original save/load
	 * dialog is entered the next time.
	 */
	Common::SeekableReadStream *_savePreparedSavegame;

	void prepareSavegame();
	bool savePreparedSavegame(int slot, char *desc);

public:
	ScummEngine_v4(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

protected:
	virtual void setupOpcodes();

	virtual void scummLoop_handleSaveLoad();

	virtual int readResTypeList(ResType type);
	virtual void readIndexFile();
	virtual void loadCharset(int no);
	virtual void resetRoomObjects();
	virtual void readMAXS(int blockSize);
	virtual void readGlobalObjects();

	virtual void resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);

	void saveVars();
	void loadVars();
	void saveIQPoints();
	void loadIQPoints(byte *ptr, int size);
	void updateIQPoints();

	/* Version 4 script opcodes */
	void o4_ifState();
	void o4_ifNotState();
	void o4_oldRoomEffect();
	void o4_pickupObject();
	void o4_saveLoadGame();
	void o4_saveLoadVars();
};


} // End of namespace Scumm

#endif
