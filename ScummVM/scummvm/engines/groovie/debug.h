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

#ifndef GROOVIE_DEBUG_H
#define GROOVIE_DEBUG_H

#include "gui/debugger.h"

namespace Groovie {

class GroovieEngine;
class Script;

class Debugger : public GUI::Debugger {
public:
	Debugger(GroovieEngine *vm);
	~Debugger();

private:
	GroovieEngine *_vm;
	Script *_script;

	int getNumber(const char *arg);

	bool cmd_step(int argc, const char **argv);
	bool cmd_go(int argc, const char **argv);
	bool cmd_pc(int argc, const char **argv);
	bool cmd_bg(int argc, const char **argv);
	bool cmd_fg(int argc, const char **argv);
	bool cmd_mem(int argc, const char **argv);
	bool cmd_loadgame(int argc, const char **argv);
	bool cmd_savegame(int argc, const char **argv);
	bool cmd_playref(int argc, const char **argv);
	bool cmd_dumppal(int argc, const char **argv);
};

} // End of Groovie namespace

#endif // GROOVIE_DEBUG_H
