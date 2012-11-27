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

#ifndef TOON_DREW_H
#define TOON_DREW_H

#include "toon/character.h"

namespace Toon {

class ToonEngine;

class CharacterDrew : public Character {
public:
	CharacterDrew(ToonEngine *vm);
	virtual ~CharacterDrew();
	bool setupPalette();
	void playStandingAnim();
	void setPosition(int16 x, int16 y);
	void resetScale();
	void update(int32 timeIncrement);
	void playWalkAnim(int32 start, int32 end);
	int32 getRandomIdleAnim();
protected:
	int32 _currentScale;
};

} // End of namespace Toon

#endif
