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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_PERSISTENCE_MANAGER_H
#define WINTERMUTE_BASE_PERSISTENCE_MANAGER_H


#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/savestate.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/system.h"
#include "common/rect.h"

namespace Wintermute {

class Vector2;
class BaseGame;
class BasePersistenceManager {
public:
	char *_savedDescription;
	Common::String _savePrefix;
	Common::String _savedName;
	bool saveFile(const Common::String &filename);
	uint32 getDWORD();
	void putDWORD(uint32 val);
	char *getString();
	Common::String getStringObj();
	void putString(const Common::String &val);
	float getFloat();
	void putFloat(float val);
	double getDouble();
	void putDouble(double val);
	void cleanup();
	void getSaveStateDesc(int slot, SaveStateDescriptor &desc);
	void deleteSaveSlot(int slot);
	uint32 getMaxUsedSlot();
	bool getSaveExists(int slot);
	bool initLoad(const Common::String &filename);
	bool initSave(const char *desc);
	bool getBytes(byte *buffer, uint32 size);
	bool putBytes(byte *buffer, uint32 size);
	uint32 _offset;

	bool getIsSaving() { return _saving; }

	uint32 _richBufferSize;
	byte *_richBuffer;

	bool transfer(const char *name, void *val);
	bool transfer(const char *name, int *val);
	bool transfer(const char *name, uint32 *val);
	bool transfer(const char *name, float *val);
	bool transfer(const char *name, double *val);
	bool transfer(const char *name, bool *val);
	bool transfer(const char *name, byte *val);
	bool transfer(const char *name, Rect32 *val);
	bool transfer(const char *name, Point32 *val);
	bool transfer(const char *name, const char **val);
	bool transfer(const char *name, char **val);
	bool transfer(const char *name, Common::String *val);
	bool transfer(const char *name, Vector2 *val);
	bool transfer(const char *name, AnsiStringArray &Val);
	BasePersistenceManager(const char *savePrefix = NULL, bool deleteSingleton = false);
	virtual ~BasePersistenceManager();
	bool checkVersion(byte  verMajor, byte verMinor, byte verBuild);

	uint32 _thumbnailDataSize;
	byte *_thumbnailData;
	Common::String getFilenameForSlot(int slot) const;
private:
	bool _deleteSingleton;
	bool readHeader(const Common::String &filename);
	TimeDate getTimeDate();
	bool putTimeDate(const TimeDate &t);
	Common::WriteStream *_saveStream;
	Common::SeekableReadStream *_loadStream;
	TimeDate _savedTimestamp;
	uint32 _savedPlayTime;
	byte _savedVerMajor;
	byte _savedVerMinor;
	byte _savedVerBuild;
	byte _savedExtMajor;
	byte _savedExtMinor;
	bool _saving;
	// Separate from Base, as this class can do SOME operations without a _gameRef.
	BaseGame *_gameRef;
};

} // end of namespace Wintermute

#endif
