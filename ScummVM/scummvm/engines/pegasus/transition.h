/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_TRANSITION_H
#define PEGASUS_TRANSITION_H

#include "pegasus/fader.h"

namespace Graphics {
struct Surface;
}

namespace Pegasus {

class ScreenFader : public Fader {
public:
	ScreenFader();
	virtual ~ScreenFader();

	void doFadeOutSync(const TimeValue = kOneSecondPerThirtyTicks, const TimeScale = kThirtyTicksPerSecond, bool isBlack = true);
	void doFadeInSync(const TimeValue = kHalfSecondPerThirtyTicks, const TimeScale = kThirtyTicksPerSecond, bool isBlack = true);

	void setFaderValue(const int32);

private:
	bool _isBlack;
	uint32 fadePixel(uint32 color, int32 percent) const;
	Graphics::Surface *_screen;
};

// Transitions are faders that range over [0,1000], which makes their
// "resolution" one tenth of a percent

static const int kTransitionBottom = 0;
static const int kTransitionTop = 1000;

static const int kTransitionRange = kTransitionTop - kTransitionBottom;

class Transition : public FaderAnimation {
public:
	Transition(const DisplayElementID id);
	virtual ~Transition() {}

	virtual void setBounds(const Common::Rect &);

	virtual void setInAndOutElements(DisplayElement *, DisplayElement *);
	DisplayElement *getInElement() { return _inPicture; }
	DisplayElement *getOutElement() { return _outPicture; }

protected:
	DisplayElement *_outPicture;
	DisplayElement *_inPicture;

	CoordType _boundsWidth, _boundsHeight;
};

class Slide : public Transition {
public:
	Slide(const DisplayElementID id) : Transition(id) {}
	virtual ~Slide() {}

	virtual void setSlideDirection(SlideDirection dir) { _direction = dir; }
	virtual void draw(const Common::Rect &);

	virtual void setDirection(const SlideDirection dir) { _direction = dir; }

protected:
	virtual void adjustSlideRects(Common::Rect &, Common::Rect &);
	virtual void drawElements(const Common::Rect &, const Common::Rect &, const Common::Rect &);
	virtual void drawSlideElement(const Common::Rect &, const Common::Rect &, DisplayElement *);

	SlideDirection _direction;
};

class Push : public Slide {
public:
	Push(const DisplayElementID id) : Slide(id) {}
	virtual ~Push() {}

protected:
	virtual void adjustSlideRects(Common::Rect &, Common::Rect &);
};

} // End of namespace Pegasus

#endif
