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

#ifndef GOB_MINIGAMES_GEISHA_MOUTH_H
#define GOB_MINIGAMES_GEISHA_MOUTH_H

#include "gob/aniobject.h"

namespace Gob {

namespace Geisha {

/** A kissing/biting mouth in Geisha's "Penetration" minigame. */
class Mouth : public ANIObject {
public:
	Mouth(const ANIFile &ani, const CMPFile &cmp,
	      uint16 mouthAnim, uint16 mouthSprite, uint16 floorSprite);
	~Mouth();

	/** Advance the animation to the next frame. */
	void advance();

	/** Active the mouth's animation. */
	void activate();

	/** Is the mouth deactivated? */
	bool isDeactivated() const;

	/** Set the current position. */
	void setPosition(int16 x, int16 y);

	/** Draw the current frame onto the surface and return the affected rectangle. */
	bool draw(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);
	/** Draw the current frame from the surface and return the affected rectangle. */
	bool clear(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);

private:
	static const int kFloorCount = 2;

	enum State {
		kStateDeactivated,
		kStateActivated,
		kStateDead
	};

	ANIObject *_sprite;
	ANIObject *_floor[kFloorCount];

	State _state;
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_MOUTH_H
