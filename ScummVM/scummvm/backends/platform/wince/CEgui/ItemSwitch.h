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

#ifndef CEGUI_ITEMSWITCH_H
#define CEGUI_ITEMSWITCH_H

#include "common/scummsys.h"
#include "common/system.h"

#include "Panel.h"
#include "EventsBuffer.h"

using GUI::Key;

namespace CEGUI {

class SDL_ImageResource;

class ItemSwitch : public PanelItem {
public:
	ItemSwitch(WORD referenceTrue, WORD referenceFalse, bool *item);
	ItemSwitch(WORD referenceTrue, WORD referenceFalse, int *item, int max);
	virtual ~ItemSwitch();
	virtual bool action(int x, int y, bool pushed);
private:
	void init(WORD referenceTrue, WORD referenceFalse);
	bool *_item;
	static bool _itemdummy;
	int  *_itemmultiple, _itemmax;
	SDL_ImageResource *_backgroundTrue;
	SDL_ImageResource *_backgroundFalse;
};

} // End of namespace CEGUI

#endif
