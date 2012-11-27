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

#ifndef GROOVIE_GRAPHICS_H
#define GROOVIE_GRAPHICS_H

#include "graphics/surface.h"

namespace Groovie {

class GroovieEngine;

class GraphicsMan {
public:
	GraphicsMan(GroovieEngine *vm);
	~GraphicsMan();

	// Buffers
	void update();
	void change();
	void mergeFgAndBg();
	void updateScreen(Graphics::Surface *source);
	Graphics::Surface _foreground;	// The main surface that most things are drawn to
	Graphics::Surface _background;	// Used occasionally, mostly (only?) in puzzles

	// Palette fading
	bool isFading();
	void fadeIn(byte *pal);
	void fadeOut();

private:
	GroovieEngine *_vm;

	bool _changed;

	// Palette fading
	void applyFading(int step);
	int _fading;
	byte _paletteFull[256 * 3];
	uint32 _fadeStartTime;
};

} // End of Groovie namespace

#endif // GROOVIE_GRAPHICS_H
