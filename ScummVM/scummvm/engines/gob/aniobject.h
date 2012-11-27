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

#ifndef GOB_ANIOBJECT_H
#define GOB_ANIOBJECT_H

#include "common/system.h"

#include "gob/backbuffer.h"

namespace Gob {

class ANIFile;
class CMPFile;
class Surface;

/** An ANI object, controlling an animation within an ANI file. */
class ANIObject : public BackBuffer {
public:
	enum Mode {
		kModeContinuous, ///< Play the animation continuously.
		kModeOnce        ///< Play the animation only once.
	};

	/** Create an animation object from an ANI file. */
	ANIObject(const ANIFile &ani);
	/** Create an animation object from a CMP sprite. */
	ANIObject(const CMPFile &cmp);
	virtual ~ANIObject();

	/** Make the object visible/invisible. */
	void setVisible(bool visible);

	/** Is the object currently visible? */
	bool isVisible() const;

	/** Pause/Unpause the animation. */
	void setPause(bool pause);

	/** Is the animation currently paused? */
	bool isPaused() const;

	/** Set the animation mode. */
	void setMode(Mode mode);

	/** Set the current position to the animation's default. */
	virtual void setPosition();
	/** Set the current position. */
	virtual void setPosition(int16 x, int16 y);

	/** Return the current position. */
	void getPosition(int16 &x, int16 &y) const;

	/** Return the frame position after another n frames. */
	void getFramePosition(int16 &x, int16 &y, uint16 n = 0) const;
	/** Return the current frame size after another n frames. */
	void getFrameSize(int16 &width, int16 &height, uint16 n = 0) const;

	/** Are there coordinates within the animation sprite? */
	bool isIn(int16 x, int16 y) const;
	/** Is this object within the animation sprite? */
	bool isIn(const ANIObject &obj) const;

	/** Set the animation number. */
	void setAnimation(uint16 animation);

	/** Rewind the current animation to the first frame. */
	void rewind();

	/** Set the animation to a specific frame. */
	void setFrame(uint16 frame);

	/** Return the current animation number. */
	uint16 getAnimation() const;
	/** Return the current frame number. */
	uint16 getFrame() const;

	/** Is this the last frame within this animation cycle? */
	bool lastFrame() const;

	/** Draw the current frame onto the surface and return the affected rectangle. */
	virtual bool draw(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);
	/** Draw the current frame from the surface and return the affected rectangle. */
	virtual bool clear(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);

	/** Advance the animation to the next frame. */
	virtual void advance();

private:
	const ANIFile *_ani; ///< The managed ANI file.
	const CMPFile *_cmp; ///< The managed CMP file.

	uint16 _animation; ///< The current animation number
	uint16 _frame;     ///< The current frame.

	bool _visible; ///< Is the object currently visible?
	bool _paused;  ///< Is the animation currently paused?

	Mode _mode; ///< The animation mode.

	int16 _x; ///< The current X position.
	int16 _y; ///< The current Y position.


	bool drawCMP(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);
	bool drawANI(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);
};

} // End of namespace Gob

#endif // GOB_ANIOBJECT_H
