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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SKY_DEBUG_H
#define SKY_DEBUG_H


#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Sky {

class Logic;
class Mouse;
class Screen;
class SkyCompact;

class Debugger : public GUI::Debugger {
public:
	Debugger(Logic *logic, Mouse *mouse, Screen *screen, SkyCompact *skyCompact);
	virtual ~Debugger(); // we need this here for __SYMBIAN32__ archaic gcc/UIQ
	bool showGrid()	{	return _showGrid; }

private:
	virtual void preEnter();
	virtual void postEnter();

private:
	bool Cmd_ShowGrid(int argc, const char **argv);
	bool Cmd_ReloadGrid(int argc, const char **argv);
	bool Cmd_ShowCompact(int argc, const char **argv);
	bool Cmd_LogicCommand(int argc, const char **argv);
	bool Cmd_Info(int argc, const char **argv);
	bool Cmd_ScriptVar(int argc, const char **argv);
	bool Cmd_Section(int argc, const char **argv);
	bool Cmd_LogicList(int argc, const char **argv);

	void dumpCompact(uint16 cptId);

	Logic *_logic;
	Mouse *_mouse;
	Screen *_screen;
	SkyCompact *_skyCompact;

	bool _showGrid;
};


class Debug {
public:
	static void logic(uint32 logic);
	static void script(uint32 command, uint16 *scriptData);
	static void mcode(uint32 mcode, uint32 a, uint32 b, uint32 c);
};

} // End of namespace Sky

#endif
