/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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

#ifndef	SWORD2_CONSOLE_H
#define	SWORD2_CONSOLE_H

#include "gui/debugger.h"
#include "sword2/debug.h"

namespace Sword2 {

class Debugger : public GUI::Debugger {
private:
	void varGet(int var);
	void varSet(int var, int val);

	bool _displayDebugText;
	bool _displayWalkGrid;
	bool _displayMouseMarker;
	bool _displayTime;
	bool _displayPlayerMarker;
	bool _displayTextNumbers;

	bool _rectFlicker;

	int32 _startTime;

	int32 _showVar[MAX_SHOWVARS];

	byte _debugTextBlocks[MAX_DEBUG_TEXTS];

	void clearDebugTextBlocks();
	void makeDebugTextBlock(char *text, int16 x, int16 y);

	void plotCrossHair(int16 x, int16 y, uint8 pen);
	void drawRect(int16 x, int16 y, int16 x2, int16 y2, uint8 pen);

public:
	Debugger(Sword2Engine *vm);

	int16 _rectX1, _rectY1;
	int16 _rectX2, _rectY2;

	uint8 _draggingRectangle;
	bool _definingRectangles;

	bool _testingSnR;

	int32 _speechScriptWaiting;

	int32 _textNumber;

	int32 _graphType;
	int32 _graphAnimRes;
	int32 _graphAnimPc;
	uint32 _graphNoFrames;

	void buildDebugText();
	void drawDebugGraphics();

private:
	virtual void preEnter();
	virtual void postEnter();

private:
	Sword2Engine *_vm;

	// Commands
	bool Cmd_Mem(int argc, const char **argv);
	bool Cmd_Tony(int argc, const char **argv);
	bool Cmd_Res(int argc, const char **argv);
	bool Cmd_ResList(int argc, const char **argv);
	bool Cmd_Starts(int argc, const char **argv);
	bool Cmd_Start(int argc, const char **argv);
	bool Cmd_Info(int argc, const char **argv);
	bool Cmd_WalkGrid(int argc, const char **argv);
	bool Cmd_Mouse(int argc, const char **argv);
	bool Cmd_Player(int argc, const char **argv);
	bool Cmd_ResLook(int argc, const char **argv);
	bool Cmd_CurrentInfo(int argc, const char **argv);
	bool Cmd_RunList(int argc, const char **argv);
	bool Cmd_Kill(int argc, const char **argv);
	bool Cmd_Nuke(int argc, const char **argv);
	bool Cmd_Var(int argc, const char **argv);
	bool Cmd_Rect(int argc, const char **argv);
	bool Cmd_Clear(int argc, const char **argv);
	bool Cmd_DebugOn(int argc, const char **argv);
	bool Cmd_DebugOff(int argc, const char **argv);
	bool Cmd_SaveRest(int argc, const char **argv);
	bool Cmd_TimeOn(int argc, const char **argv);
	bool Cmd_TimeOff(int argc, const char **argv);
	bool Cmd_Text(int argc, const char **argv);
	bool Cmd_ShowVar(int argc, const char **argv);
	bool Cmd_HideVar(int argc, const char **argv);
	bool Cmd_Version(int argc, const char **argv);
	bool Cmd_AnimTest(int argc, const char **argv);
	bool Cmd_TextTest(int argc, const char **argv);
	bool Cmd_LineTest(int argc, const char **argv);
	bool Cmd_Events(int argc, const char **argv);
	bool Cmd_Sfx(int argc, const char **argv);
	bool Cmd_English(int argc, const char **argv);
	bool Cmd_Finnish(int argc, const char **argv);
	bool Cmd_Polish(int argc, const char **argv);
	bool Cmd_FxQueue(int argc, const char **argv);
};

} // End of namespace Sword2

#endif
