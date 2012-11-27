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

#ifndef LURE_DEBUGGER_H
#define LURE_DEBUGGER_H

#include "gui/debugger.h"

namespace Lure {

class Debugger : public GUI::Debugger {
public:
	Debugger();
	virtual ~Debugger() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

private:
	bool cmd_enterRoom(int argc, const char **argv);
	bool cmd_listRooms(int argc, const char **argv);
	bool cmd_listFields(int argc, const char **argv);
	bool cmd_setField(int argc, const char **argv);
	bool cmd_queryField(int argc, const char **argv);
	bool cmd_giveItem(int argc, const char **argv);
	bool cmd_hotspots(int argc, const char **argv);
	bool cmd_hotspot(int argc, const char **argv);
	bool cmd_room(int argc, const char **argv);
	bool cmd_showAnim(int argc, const char **argv);
	bool cmd_saveStrings(int argc, const char **argv);
	bool cmd_debug(int argc, const char **argv);
	bool cmd_script(int argc, const char **argv);
};

extern const char *directionList[5];

} // End of namespace Lure

#endif
