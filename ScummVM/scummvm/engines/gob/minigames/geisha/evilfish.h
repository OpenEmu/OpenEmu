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

#ifndef GOB_MINIGAMES_GEISHA_EVILFISH_H
#define GOB_MINIGAMES_GEISHA_EVILFISH_H

#include "gob/aniobject.h"

namespace Gob {

namespace Geisha {

/** An "evil" fish in Geisha's "Diving" minigame. */
class EvilFish : public ANIObject {
public:
	enum Direction {
		kDirectionLeft  = 0,
		kDirectionRight = 1
	};

	EvilFish(const ANIFile &ani, uint16 screenWidth,
	         uint16 animSwimLeft, uint16 animSwimRight,
	         uint16 animTurnLeft, uint16 animTurnRight, uint16 animDie);
	~EvilFish();

	/** Enter from this direction / screen edge. */
	void enter(Direction from, int16 y);
	/** Leave the screen in the current direction. */
	void leave();

	/** Kill the fish. */
	void die();

	/** Advance the animation to the next frame. */
	void advance();

	/** Change the fish's animations, effectively making it a different fish type. */
	void mutate(uint16 animSwimLeft, uint16 animSwimRight,
	            uint16 animTurnLeft, uint16 animTurnRight, uint16 animDie);

	/** Is the fish dead? */
	bool isDead() const;

private:
	enum State {
		kStateNone,
		kStateSwimLeft,
		kStateSwimRight,
		kStateTurnLeft,
		kStateTurnRight,
		kStateDie
	};

	uint16 _screenWidth;

	uint16 _animSwimLeft;
	uint16 _animSwimRight;
	uint16 _animTurnLeft;
	uint16 _animTurnRight;
	uint16 _animDie;

	bool _shouldLeave;

	State _state;
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_EVILFISH_H
