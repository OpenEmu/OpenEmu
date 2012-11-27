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

#ifndef WINTERMUTE_ADRESPONSEBOX_H
#define WINTERMUTE_ADRESPONSEBOX_H


#include "engines/wintermute/base/base_object.h"

namespace Wintermute {

class UIButton;
class UIWindow;
class UIObject;
class AdResponse;
class AdResponseBox : public BaseObject {
public:
	BaseObject *getNextAccessObject(BaseObject *CurrObject);
	BaseObject *getPrevAccessObject(BaseObject *CurrObject);
	bool getObjects(BaseArray<UIObject *> &objects, bool interactiveOnly);

	bool handleResponse(AdResponse *response);
	void setLastResponseText(const char *text, const char *textOrig);
	char *_lastResponseText;
	char *_lastResponseTextOrig;
	DECLARE_PERSISTENT(AdResponseBox, BaseObject)
	ScScript *_waitingScript;
	virtual bool listen(BaseScriptHolder *param1, uint32 param2);
	typedef enum {
		EVENT_PREV,
		EVENT_NEXT,
		EVENT_RESPONSE
	} TResponseEvent;

	bool weedResponses();
	bool display();
	int _spacing;
	int _scrollOffset;
	BaseFont *_fontHover;
	BaseFont *_font;
	bool createButtons();
	bool invalidateButtons();
	void clearButtons();
	void clearResponses();
	AdResponseBox(BaseGame *inGame);
	virtual ~AdResponseBox();
	BaseArray<AdResponse *> _responses;
	BaseArray<UIButton *> _respButtons;
	UIWindow *_window;
	UIWindow *_shieldWindow;
	bool _horizontal;
	Rect32 _responseArea;
	int _verticalAlign;
	TTextAlign _align;
	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent);
};

} // end of namespace Wintermute

#endif
