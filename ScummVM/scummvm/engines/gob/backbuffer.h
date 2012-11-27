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

#ifndef GOB_BACKBUFFER_H
#define GOB_BACKBUFFER_H

#include "common/system.h"

namespace Gob {

class Surface;

class BackBuffer {
public:
	BackBuffer();
	~BackBuffer();

protected:
	void trashBuffer();
	void resizeBuffer(uint16 width, uint16 height);

	bool saveScreen   (const Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);
	bool restoreScreen(      Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);

	bool hasBuffer() const;
	bool hasSavedBackground() const;

private:
	Surface *_background; ///< The saved background.
	bool _saved;          ///< Was the background saved?

	int16 _saveLeft;   ///< The left position of the saved background.
	int16 _saveTop;    ///< The top of the saved background.
	int16 _saveRight;  ///< The right position of the saved background.
	int16 _saveBottom; ///< The bottom position of the saved background.
};

} // End of namespace Gob

#endif // GOB_BACKBUFFER_H
