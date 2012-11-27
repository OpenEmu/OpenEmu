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

#ifndef QUEEN_DEBUG_H
#define QUEEN_DEBUG_H

#include "gui/debugger.h"

namespace Queen {

class QueenEngine;

class Debugger : public GUI::Debugger {
public:
	Debugger(QueenEngine *vm);
	virtual ~Debugger(); // we need this here for __SYMBIAN32__ archaic gcc/UIQ

	int flags() const { return _flags; }

	enum {
		DF_DRAW_AREAS = 1 << 0
	};

private:
	virtual void preEnter();
	virtual void postEnter();

private:
	bool Cmd_Areas(int argc, const char **argv);
	bool Cmd_Asm(int argc, const char **argv);
	bool Cmd_Bob(int argc, const char **argv);
	bool Cmd_GameState(int argc, const char **argv);
	bool Cmd_Info(int argc, const char **argv);
	bool Cmd_Items(int argc, const char **argv);
	bool Cmd_PrintBobs(int argc, const char **argv);
	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_Song(int argc, const char **argv);

private:
	QueenEngine *_vm;
	int _flags;
};

} // End of namespace Queen

#endif
