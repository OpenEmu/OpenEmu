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

 // Console module header file

#ifndef SAGA_CONSOLE_H
#define SAGA_CONSOLE_H

#include "gui/debugger.h"

namespace Saga {

class Console : public GUI::Debugger {
public:
	Console(SagaEngine *vm);
	virtual ~Console();

private:
	bool cmdActorWalkTo(int argc, const char **argv);

	bool cmdAnimInfo(int argc, const char **argv);
	bool cmdCutawayInfo(int argc, const char **argv);
	bool cmdPlayCutaway(int argc, const char **argv);

	bool cmdCurrentScene(int argc, const char **argv);
	bool cmdCurrentChapter(int argc, const char **argv);
	bool cmdSceneChange(int argc, const char **argv);
	bool cmdChapterChange(int argc, const char **argv);

	bool cmdActionMapInfo(int argc, const char **argv);
	bool cmdObjectMapInfo(int argc, const char **argv);

	bool cmdWakeUpThreads(int argc, const char **argv);

	bool cmdCurrentPanelMode(int argc, const char **argv);
	bool cmdSetPanelMode(int argc, const char **argv);

	bool cmdSetFontMapping(int argc, const char **argv);

	bool cmdGlobalFlagsInfo(int argc, const char **argv);
	bool cmdSetGlobalFlag(int argc, const char **argv);
	bool cmdClearGlobalFlag(int argc, const char **argv);

private:
	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
