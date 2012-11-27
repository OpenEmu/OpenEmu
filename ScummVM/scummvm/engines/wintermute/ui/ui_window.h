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

#ifndef WINTERMUTE_UIWINDOW_H
#define WINTERMUTE_UIWINDOW_H


#include "engines/wintermute/ui/ui_object.h"
#include "common/events.h"

namespace Wintermute {

class UIButton;
class BaseViewport;
class UIWindow : public UIObject {
	uint32 _fadeColor;
public:
	bool getWindowObjects(BaseArray<UIObject *> &Objects, bool InteractiveOnly);

	bool _pauseMusic;
	void cleanup();
	virtual void makeFreezable(bool freezable);
	BaseViewport *_viewport;
	bool _clipContents;
	bool _inGame;
	bool _isMenu;
	bool _fadeBackground;

	virtual bool handleMouseWheel(int delta);
	UIWindow *_shieldWindow;
	UIButton *_shieldButton;
	bool close();
	bool goSystemExclusive();
	bool goExclusive();
	TWindowMode _mode;
	bool moveFocus(bool forward = true);
	virtual bool handleMouse(TMouseEvent Event, TMouseButton Button);
	Point32 _dragFrom;
	bool _dragging;
	DECLARE_PERSISTENT(UIWindow, UIObject)
	bool _transparent;
	bool showWidget(const char *name, bool visible = true);
	bool enableWidget(const char *name, bool enable = true);
	Rect32 _titleRect;
	Rect32 _dragRect;
	virtual bool display(int offsetX = 0, int offsetY = 0);
	UIWindow(BaseGame *inGame);
	virtual ~UIWindow();
	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	BaseArray<UIObject *> _widgets;
	TTextAlign _titleAlign;
	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);
	UITiledImage *_backInactive;
	BaseFont *_fontInactive;
	BaseSprite *_imageInactive;
	virtual bool listen(BaseScriptHolder *param1, uint32 param2);
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent);

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace Wintermute

#endif
