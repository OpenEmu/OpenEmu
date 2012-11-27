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
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_CUSTOM_H
#define TONY_CUSTOM_H

#include "common/str.h"
#include "tony/mpal/mpal.h"

namespace Tony {

using namespace MPAL;

struct MusicFileEntry {
	const char *_name;
	int _sync;
};

#define INIT_CUSTOM_FUNCTION            MapCustomFunctions

#define BEGIN_CUSTOM_FUNCTION_MAP()                                            \
	static void AssignError(int num) {                                           \
		error("Custom function %u has been already assigned!", num);               \
	}                                                                            \
	void INIT_CUSTOM_FUNCTION(LPCUSTOMFUNCTION *lpMap, Common::String *lpStrMap) \
	{

#define END_CUSTOM_FUNCTION_MAP()                                              \
	}

#define ASSIGN(num, func)                                                      \
	if (lpMap[num] != NULL)                                                      \
		AssignError(num);                                                          \
	lpMap[num] = func;                                                           \
	lpStrMap[num] = #func;

class RMTony;
class RMPointer;
class RMGameBoxes;
class RMLocation;
class RMInventory;
class RMInput;

void charsSaveAll(Common::OutSaveFile *f);
void charsLoadAll(Common::InSaveFile *f);
void mCharResetCodes();
void saveChangedHotspot(Common::OutSaveFile *f);
void loadChangedHotspot(Common::InSaveFile *f);
void reapplyChangedHotspot();

void restoreMusic(CORO_PARAM);
void saveMusic(Common::OutSaveFile *f);
void loadMusic(Common::InSaveFile *f);

void INIT_CUSTOM_FUNCTION(LPCUSTOMFUNCTION *lpMap, Common::String *lpStrMap);
void setupGlobalVars(RMTony *tony, RMPointer *ptr, RMGameBoxes *box, RMLocation *loc, RMInventory *inv, RMInput *input);

#endif

} // end of namespace Tony
