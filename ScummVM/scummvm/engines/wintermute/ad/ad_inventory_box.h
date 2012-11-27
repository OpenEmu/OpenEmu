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

#ifndef WINTERMUTE_ADINVENTORYBOX_H
#define WINTERMUTE_ADINVENTORYBOX_H

#include "engines/wintermute/base/base_object.h"
#include "common/rect.h"

namespace Wintermute {
class UIButton;
class UIWindow;

class AdInventoryBox : public BaseObject {
public:
	bool _hideSelected;
	DECLARE_PERSISTENT(AdInventoryBox, BaseObject)
	bool _visible;
	virtual bool display();
	UIButton *_closeButton;
	int _spacing;
	int _scrollOffset;
	Rect32 _itemsArea;
	bool listen(BaseScriptHolder *param1, uint32 param2);
	UIWindow *_window;
	AdInventoryBox(BaseGame *inGame);
	virtual ~AdInventoryBox();
	bool loadFile(const char *filename);
	bool loadBuffer(byte *buffer, bool complete = true);
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent);
private:
	bool _exclusive;
	int _scrollBy;
	int _itemHeight;
	int _itemWidth;
};

} // end of namespace Wintermute

#endif
