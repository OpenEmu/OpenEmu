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

#ifndef SWORD25_ANIMATIONRESOURCE_H
#define SWORD25_ANIMATIONRESOURCE_H

#include "common/xmlparser.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/resource.h"
#include "sword25/gfx/animationdescription.h"
#include "sword25/gfx/animation.h"

namespace Sword25 {

class Kernel;
class PackageManager;

class AnimationResource : public Resource, public AnimationDescription, public Common::XMLParser {
public:
	AnimationResource(const Common::String &filename);
	virtual ~AnimationResource();

	virtual const Frame &getFrame(uint index) const {
		return _frames[index];
	}
	virtual uint getFrameCount() const {
		return _frames.size();
	}
	virtual void unlock() {
		release();
	}

	Animation::ANIMATION_TYPES getAnimationType() const {
		return _animationType;
	}
	int getFPS() const {
		return _FPS;
	}
	int getMillisPerFrame() const {
		return _millisPerFrame;
	}
	bool isScalingAllowed() const {
		return _scalingAllowed;
	}
	bool isAlphaAllowed() const {
		return _alphaAllowed;
	}
	bool isColorModulationAllowed() const {
		return _colorModulationAllowed;
	}
	bool isValid() const {
		return _valid;
	}

private:
	bool _valid;

	Common::Array<Frame> _frames;

	PackageManager *_pPackage;


	bool computeFeatures();
	bool precacheAllFrames() const;

	// Parser
	CUSTOM_XML_PARSER(AnimationResource) {
		XML_KEY(animation)
			XML_PROP(fps, true)
			XML_PROP(type, true)

			XML_KEY(frame)
				XML_PROP(file, true)
				XML_PROP(hotspotx, true)
				XML_PROP(hotspoty, true)
				XML_PROP(fliph, false)
				XML_PROP(flipv, false)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parseBooleanKey(Common::String s, bool &result);

	// Parser callback methods
	bool parserCallback_animation(ParserNode *node);
	bool parserCallback_frame(ParserNode *node);
};

} // End of namespace Sword25

#endif
