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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_DEBUGGER_H
#define TSAGE_DEBUGGER_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace TsAGE {

class Debugger : public GUI::Debugger {
public:
	Debugger();
	virtual ~Debugger() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

protected:
	bool Cmd_Scene(int argc, const char **argv);
	bool Cmd_WalkRegions(int argc, const char **argv);
	bool Cmd_PriorityRegions(int argc, const char **argv);
	bool Cmd_SceneRegions(int argc, const char **argv);
	bool Cmd_SetFlag(int argc, const char **argv);
	bool Cmd_GetFlag(int argc, const char **argv);
	bool Cmd_ClearFlag(int argc, const char **argv);
	bool Cmd_Hotspots(int argc, const char **argv);
	bool Cmd_Sound(int argc, const char **argv);
	virtual bool Cmd_ListObjects(int argc, const char **argv) = 0;
	virtual bool Cmd_MoveObject(int argc, const char **argv) = 0;
};

class DemoDebugger : public Debugger {
protected:
	virtual bool Cmd_ListObjects(int argc, const char **argv);
	virtual bool Cmd_MoveObject(int argc, const char **argv);
};

class RingworldDebugger : public Debugger {
protected:
	virtual bool Cmd_ListObjects(int argc, const char **argv);
	virtual bool Cmd_MoveObject(int argc, const char **argv);
};

class BlueForceDebugger : public Debugger {
protected:
	virtual bool Cmd_ListObjects(int argc, const char **argv);
	virtual bool Cmd_MoveObject(int argc, const char **argv);
};

class Ringworld2Debugger : public Debugger {
protected:
	virtual bool Cmd_ListObjects(int argc, const char **argv);
	virtual bool Cmd_MoveObject(int argc, const char **argv);
};

} // End of namespace TsAGE

#endif
