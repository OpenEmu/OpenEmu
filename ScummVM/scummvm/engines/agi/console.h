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

#ifndef AGI_CONSOLE_H
#define AGI_CONSOLE_H

namespace Agi {

class AgiEngine;
class PreAgiEngine;
class MickeyEngine;
class WinnieEngine;

struct AgiDebug {
	int enabled;
	int opcodes;
	int logic0;
	int steps;
	int priority;
	int statusline;
	int ignoretriggers;
};

class Console : public GUI::Debugger {
public:
	Console(AgiEngine *vm);

private:
	bool Cmd_SetVar(int argc, const char **argv);
	bool Cmd_SetFlag(int argc, const char **argv);
	bool Cmd_SetObj(int argc, const char **argv);
	bool Cmd_RunOpcode(int argc, const char **argv);
	bool Cmd_Agiver(int argc, const char **argv);
	bool Cmd_Flags(int argc, const char **argv);
	bool Cmd_Vars(int argc, const char **argv);
	bool Cmd_Objs(int argc, const char **argv);
	bool Cmd_Opcode(int argc, const char **argv);
	bool Cmd_Logic0(int argc, const char **argv);
	bool Cmd_Trigger(int argc, const char **argv);
	bool Cmd_Step(int argc, const char **argv);
	bool Cmd_Debug(int argc, const char **argv);
	bool Cmd_Cont(int argc, const char **argv);
	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_BT(int argc, const char **argv);

private:
	AgiEngine *_vm;
};

class MickeyConsole : public GUI::Debugger {
public:
	MickeyConsole(MickeyEngine *mickey);
	virtual ~MickeyConsole() {}

private:
	MickeyEngine *_mickey;

	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_DrawPic(int argc, const char **argv);
	bool Cmd_DrawObj(int argc, const char **argv);
};

class WinnieConsole : public GUI::Debugger {
public:
	WinnieConsole(WinnieEngine *winnie);
	virtual ~WinnieConsole() {}

private:
	WinnieEngine *_winnie;

	bool Cmd_CurRoom(int argc, const char **argv);
};

} // End of namespace Agi

#endif /* AGI_CONSOLE_H */
