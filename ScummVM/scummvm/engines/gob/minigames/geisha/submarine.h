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

#ifndef GOB_MINIGAMES_GEISHA_SUBMARINE_H
#define GOB_MINIGAMES_GEISHA_SUBMARINE_H

#include "gob/aniobject.h"

namespace Gob {

namespace Geisha {

/** The submarine Geisha's "Penetration" minigame. */
class Submarine : public ANIObject {
public:
	enum Direction {
		kDirectionNone,
		kDirectionN,
		kDirectionNE,
		kDirectionE,
		kDirectionSE,
		kDirectionS,
		kDirectionSW,
		kDirectionW,
		kDirectionNW
	};

	Submarine(const ANIFile &ani);
	~Submarine();

	Direction getDirection() const;

	/** Turn to the specified direction. */
	void turn(Direction to);

	/** Play the shoot animation. */
	void shoot();

	/** Play the exploding animation. */
	void die();

	/** Play the exiting animation. */
	void leave();

	/** Advance the animation to the next frame. */
	void advance();

	/** Can the submarine move at the moment? */
	bool canMove() const;

	/** Is the submarine dead? */
	bool isDead() const;

	/** Is the submarine shooting? */
	bool isShooting() const;

	/** Has the submarine finished exiting the level? */
	bool hasExited() const;

private:
	enum State {
		kStateNone = 0,
		kStateMove,
		kStateShoot,
		kStateExit,
		kStateExited,
		kStateDie,
		kStateDead
	};

	State _state;
	Direction _direction;

	/** Map the directions to move animation indices. */
	uint16 directionToMove(Direction direction) const;
	/** Map the directions to shoot animation indices. */
	uint16 directionToShoot(Direction direction) const;
	/** Map the directions to explode animation indices. */
	uint16 directionToExplode(Direction direction) const;

	void move();
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_SUBMARINE_H
