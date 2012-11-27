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

#include "parallaction/parallaction.h"

#include "common/textconsole.h"

namespace Parallaction {

void Parallaction_br::_c_null(void *) {
	// do nothing :)
}

void Parallaction_br::_c_blufade(void *) {
	warning("Parallaction_br::_c_blufade() not yet implemented");
}

void Parallaction_br::_c_resetpalette(void *) {
	warning("Parallaction_br::_c_resetpalette() not yet implemented");
}

void Parallaction_br::_c_ferrcycle(void *) {
	warning("Parallaction_br::_c_ferrcycle() not yet implemented");
}

void Parallaction_br::_c_lipsinc(void *) {
	warning("Unexpected lipsinc routine call! Please notify the team");
}

void Parallaction_br::_c_albcycle(void *) {
	warning("Parallaction_br::_c_albcycle() not yet implemented");
}

void Parallaction_br::_c_password(void *) {
	warning("Parallaction_br::_c_password() not yet implemented");
}




} // namespace Parallaction
