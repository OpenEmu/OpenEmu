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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_UIEDIT_H
#define WINTERMUTE_UIEDIT_H

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/ui/ui_object.h"
#include "common/events.h"

namespace Wintermute {
class BaseFont;
class UIEdit : public UIObject {
public:
	DECLARE_PERSISTENT(UIEdit, UIObject)
	int _maxLength;
	int insertChars(int pos, const byte *chars, int num);
	int deleteChars(int start, int end);
	bool _cursorVisible;
	uint32 _lastBlinkTime;
	virtual bool display(int offsetX, int offsetY);
	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	int _scrollOffset;
	int _frameWidth;
	uint32 _cursorBlinkRate;
	void setCursorChar(const char *character);
	char *_cursorChar;
	int _selEnd;
	int _selStart;
	BaseFont *_fontSelected;
	UIEdit(BaseGame *inGame);
	virtual ~UIEdit();

	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent);

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace Wintermute

#endif
