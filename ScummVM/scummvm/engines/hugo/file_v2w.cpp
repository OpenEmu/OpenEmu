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
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/util.h"

namespace Hugo {
FileManager_v2w::FileManager_v2w(HugoEngine *vm) : FileManager_v2d(vm) {
}

FileManager_v2w::~FileManager_v2w() {
}

/**
 * Display a Windows help file
 * Same comment than in SCI: maybe in the future we can implement this, but for now this message should suffice
 */
void FileManager_v2w::instructions() const {
	Utils::notifyBox(Common::String::format("Please use an external viewer to open the game's help file: HUGOWIN%d.HLP", _vm->_gameVariant + 1));
}

} // End of namespace Hugo
