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

#ifndef CEGUI_GUIELEMENT_H
#define CEGUI_GUIELEMENT_H

#include "common/scummsys.h"
#include "common/system.h"

struct SDL_Surface;

namespace CEGUI {

class SDL_ImageResource;

class GUIElement {
public:
	bool setBackground(WORD backgroundReference);
	void setVisible(bool visibility);
	virtual void forceRedraw();
	virtual bool draw(SDL_Surface *surface);
	virtual ~GUIElement();
	void move(int x, int y);
	int width();
	int height();
	int x();
	int y();
	virtual bool action(int x, int y, bool pushed) = 0;
	bool visible();
	bool drawn();
protected:
	GUIElement(int x = 0, int y = 0, int width = 0, int height = 0);
	bool checkInside(int x, int y);
	bool _visible;
	SDL_ImageResource *_background;
	int _x;
	int _y;
	bool _drawn;
private:
	int _width;
	int _height;
};

} // End of namespace CEGUI

#endif
