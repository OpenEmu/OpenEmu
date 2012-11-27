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

#ifndef CEGUI_TOOLBARHANDLER_H
#define CEGUI_TOOLBARHANDLER_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/config-manager.h"

#include "Toolbar.h"

using Common::String;
using Common::HashMap;

namespace CEGUI {

class ToolbarHandler {
public:
	ToolbarHandler();
	bool add(const String &name, const Toolbar &toolbar);
	bool setActive(const String &name);
	bool action(int x, int y, bool pushed);
	void setVisible(bool visible);
	bool visible();
	String activeName();
	void forceRedraw();
	void setOffset(int offset);
	int getOffset();
	bool draw(SDL_Surface *surface, SDL_Rect *rect);
	bool drawn();
	Toolbar *active();
	virtual ~ToolbarHandler();
private:

	HashMap<String, Toolbar *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _toolbarMap;
	String _current;
	Toolbar *_active;
	int _offset;
};

} // End of namespace CEGUI

#endif
