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
 */

#ifndef SCUMM_DEBUGGER_H
#define SCUMM_DEBUGGER_H

#include "gui/debugger.h"

namespace Scumm {

class ScummEngine;

class ScummDebugger : public GUI::Debugger {
public:
	ScummDebugger(ScummEngine *s);
	virtual ~ScummDebugger(); // we need this here for __SYMBIAN32__

private:
	ScummEngine *_vm;
	bool  _old_soundsPaused;

	// Commands
	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_LoadGame(int argc, const char **argv);
	bool Cmd_SaveGame(int argc, const char **argv);
	bool Cmd_Restart(int argc, const char **argv);

	bool Cmd_PrintActor(int argc, const char **argv);
	bool Cmd_PrintBox(int argc, const char **argv);
	bool Cmd_PrintBoxMatrix(int argc, const char **argv);
	bool Cmd_PrintObjects(int argc, const char **argv);
	bool Cmd_Actor(int argc, const char **argv);
	bool Cmd_Camera(int argc, const char **argv);
	bool Cmd_Object(int argc, const char **argv);
	bool Cmd_Script(int argc, const char **argv);
	bool Cmd_PrintScript(int argc, const char **argv);
	bool Cmd_ImportRes(int argc, const char **argv);

	bool Cmd_PrintDraft(int argc, const char **argv);
	bool Cmd_Passcode(int argc, const char **argv);

	bool Cmd_Debug(int argc, const char **argv);
	bool Cmd_DebugLevel(int argc, const char **argv);

	bool Cmd_Show(int argc, const char **argv);
	bool Cmd_Hide(int argc, const char **argv);

	bool Cmd_IMuse(int argc, const char **argv);

	bool Cmd_ResetCursors(int argc, const char **argv);

	void printBox(int box);
	void drawBox(int box);
};

} // End of namespace Scumm

#endif
