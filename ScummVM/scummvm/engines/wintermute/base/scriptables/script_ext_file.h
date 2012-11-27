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

#ifndef WINTERMUTES_SXFILE_H
#define WINTERMUTES_SXFILE_H


#include "engines/wintermute/base/base_scriptable.h"
#include "common/stream.h"

namespace Wintermute {

class BaseFile;

class SXFile : public BaseScriptable {
public:
	DECLARE_PERSISTENT(SXFile, BaseScriptable)
	ScValue *scGetProperty(const Common::String &name);
	bool scSetProperty(const char *name, ScValue *value);
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	const char *scToString();
	SXFile(BaseGame *inGame, ScStack *Stack);
	virtual ~SXFile();
private:
	Common::SeekableReadStream *_readFile;
	Common::WriteStream *_writeFile;
	int _mode; // 0..none, 1..read, 2..write, 3..append
	bool _textMode;
	void close();
	void cleanup();
	uint32 getPos();
	uint32 getLength();
	bool setPos(uint32 pos, int whence = SEEK_SET);
	char *_filename;
	Common::WriteStream *openForWrite(const Common::String &filename, bool binary);
	Common::WriteStream *openForAppend(const Common::String &filename, bool binary);
};

} // end of namespace Wintermute

#endif
