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

#ifndef SWORD25_CONSOLE_H
#define SWORD25_CONSOLE_H

#include "gui/debugger.h"

namespace Sword25 {

class Sword25Engine;

class Sword25Console : public GUI::Debugger {
public:
	Sword25Console(Sword25Engine *vm);
	virtual ~Sword25Console(void);

private:
	Sword25Engine *_vm;
};

} // End of namespace Sword25

#endif
