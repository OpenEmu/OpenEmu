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

#ifndef TOON_FLUX_H
#define TOON_FLUX_H

#include "toon/character.h"

class ToonEngine;

namespace Toon {

class CharacterFlux : public Character {
public:
	CharacterFlux(ToonEngine *vm);
	virtual ~CharacterFlux();

	void setPosition(int16 x, int16 y);
	void playStandingAnim();
	void playWalkAnim(int32 start, int32 end);
	void update(int32 timeIncrement);
	int32 getRandomIdleAnim();
	void setVisible(bool visible);
	static int32 fixFacingForAnimation(int32 originalFacing, int32 animationId);
};

} // End of namespace Toon

#endif
