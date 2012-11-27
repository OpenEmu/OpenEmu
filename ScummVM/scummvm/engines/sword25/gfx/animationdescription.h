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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_ANIMATIONDESCRIPTION_H
#define SWORD25_ANIMATIONDESCRIPTION_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/gfx/animation.h"

namespace Sword25 {

class AnimationDescription : public Persistable {
protected:
	AnimationDescription() :
		_animationType(Animation::AT_LOOP),
		_FPS(10),
		_millisPerFrame(0),
		_scalingAllowed(true),
		_alphaAllowed(true),
		_colorModulationAllowed(true)
	{}

public:
	struct Frame {
		// Die Hotspot-Angabe bezieht sich auf das ungeflippte Bild!!
		int         hotspotX;
		int         hotspotY;
		bool        flipV;
		bool        flipH;
		Common::String  fileName;
		Common::String  action;
	};

	virtual const Frame    &getFrame(uint index) const = 0;
	virtual uint  getFrameCount() const = 0;
	virtual void  unlock() = 0;

	Animation::ANIMATION_TYPES   getAnimationType() const {
		return _animationType;
	}
	int                             getFPS() const {
		return _FPS;
	}
	int                             getMillisPerFrame() const {
		return _millisPerFrame;
	}
	bool                            isScalingAllowed() const {
		return _scalingAllowed;
	}
	bool                            isAlphaAllowed() const {
		return _alphaAllowed;
	}
	bool                            isColorModulationAllowed() const {
		return _colorModulationAllowed;
	}

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

protected:
	Animation::ANIMATION_TYPES   _animationType;
	int                          _FPS;
	int                          _millisPerFrame;
	bool                         _scalingAllowed;
	bool                         _alphaAllowed;
	bool                         _colorModulationAllowed;
};

} // End of namespace Sword25

#endif
