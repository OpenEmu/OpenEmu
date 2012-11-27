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

#ifndef WINTERMUTE_BASE_ENGINE_H
#define WINTERMUTE_BASE_ENGINE_H

#include "common/str.h"
#include "common/singleton.h"
#include "common/random.h"
#include "common/language.h"

namespace Wintermute {

class BaseFileManager;
class BaseRegistry;
class BaseGame;
class SystemClassRegistry;
class BaseEngine : public Common::Singleton<Wintermute::BaseEngine> {
	void init(Common::Language lang);
	BaseFileManager *_fileManager;
	Common::String _gameId;
	BaseGame *_gameRef;
	// We need random numbers
	Common::RandomSource *_rnd;
	SystemClassRegistry *_classReg;
public:
	BaseEngine();
	~BaseEngine();
	static void createInstance(const Common::String &gameid, Common::Language lang);
	void setGameRef(BaseGame *gameRef) { _gameRef = gameRef; }

	Common::RandomSource *getRandomSource() { return _rnd; }
	uint32 randInt(int from, int to);

	SystemClassRegistry *getClassRegistry(){ return _classReg; }
	BaseGame *getGameRef() { return _gameRef; }
	BaseFileManager *getFileManager() { return _fileManager; }
	static void LOG(bool res, const char *fmt, ...);
	const char *getGameId() { return _gameId.c_str(); }
};

} // end of namespace Wintermute

#endif
