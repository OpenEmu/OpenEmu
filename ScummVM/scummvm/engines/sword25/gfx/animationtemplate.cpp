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

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resource.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"

namespace Sword25 {

uint AnimationTemplate::create(const Common::String &sourceAnimation) {
	AnimationTemplate *animationTemplatePtr = new AnimationTemplate(sourceAnimation);

	if (animationTemplatePtr->isValid()) {
		return AnimationTemplateRegistry::instance().resolvePtr(animationTemplatePtr);
	} else {
		delete animationTemplatePtr;
		return 0;
	}
}

uint AnimationTemplate::create(const AnimationTemplate &other) {
	AnimationTemplate *animationTemplatePtr = new AnimationTemplate(other);

	if (animationTemplatePtr->isValid()) {
		return AnimationTemplateRegistry::instance().resolvePtr(animationTemplatePtr);
	} else {
		delete animationTemplatePtr;
		return 0;
	}
}

uint AnimationTemplate::create(InputPersistenceBlock &reader, uint handle) {
	AnimationTemplate *animationTemplatePtr = new AnimationTemplate(reader, handle);

	if (animationTemplatePtr->isValid()) {
		return AnimationTemplateRegistry::instance().resolvePtr(animationTemplatePtr);
	} else {
		delete animationTemplatePtr;
		return 0;
	}
}

AnimationTemplate::AnimationTemplate(const Common::String &sourceAnimation) {
	// Objekt registrieren.
	AnimationTemplateRegistry::instance().registerObject(this);

	_valid = false;

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt
	_sourceAnimationPtr = requestSourceAnimation(sourceAnimation);

	// Erfolg signalisieren
	_valid = (_sourceAnimationPtr != 0);
}

AnimationTemplate::AnimationTemplate(const AnimationTemplate &other) : AnimationDescription() {
	// Objekt registrieren.
	AnimationTemplateRegistry::instance().registerObject(this);

	_valid = false;

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt.
	if (!other._sourceAnimationPtr)
		return;
	_sourceAnimationPtr = requestSourceAnimation(other._sourceAnimationPtr->getFileName());

	// Alle Member kopieren.
	_animationType = other._animationType;
	_FPS = other._FPS;
	_millisPerFrame = other._millisPerFrame;
	_scalingAllowed = other._scalingAllowed;
	_alphaAllowed = other._alphaAllowed;
	_colorModulationAllowed = other._colorModulationAllowed;
	_frames = other._frames;
	_sourceAnimationPtr = other._sourceAnimationPtr;
	_valid = other._valid;

	_valid &= (_sourceAnimationPtr != 0);
}

AnimationTemplate::AnimationTemplate(InputPersistenceBlock &reader, uint handle) {
	// Objekt registrieren.
	AnimationTemplateRegistry::instance().registerObject(this, handle);

	// Objekt laden.
	_valid = unpersist(reader);
}

AnimationResource *AnimationTemplate::requestSourceAnimation(const Common::String &sourceAnimation) const {
	ResourceManager *RMPtr = Kernel::getInstance()->getResourceManager();
	Resource *resourcePtr;
	if (NULL == (resourcePtr = RMPtr->requestResource(sourceAnimation)) || resourcePtr->getType() != Resource::TYPE_ANIMATION) {
		error("The resource \"%s\" could not be requested or is has an invalid type. The animation template can't be created.", sourceAnimation.c_str());
		return 0;
	}
	return static_cast<AnimationResource *>(resourcePtr);
}

AnimationTemplate::~AnimationTemplate() {
	// Animations-Resource freigeben
	if (_sourceAnimationPtr) {
		_sourceAnimationPtr->release();
	}

	// Objekt deregistrieren
	AnimationTemplateRegistry::instance().deregisterObject(this);
}

void AnimationTemplate::addFrame(int index) {
	if (validateSourceIndex(index)) {
		_frames.push_back(_sourceAnimationPtr->getFrame(index));
	}
}

void AnimationTemplate::setFrame(int destIndex, int srcIndex) {
	if (validateDestIndex(destIndex) && validateSourceIndex(srcIndex)) {
		_frames[destIndex] = _sourceAnimationPtr->getFrame(srcIndex);
	}
}

bool AnimationTemplate::validateSourceIndex(uint index) const {
	if (index > _sourceAnimationPtr->getFrameCount()) {
		warning("Tried to insert a frame (\"%d\") that does not exist in the source animation (\"%s\"). Ignoring call.",
		                 index, _sourceAnimationPtr->getFileName().c_str());
		return false;
	} else
		return true;
}

bool AnimationTemplate::validateDestIndex(uint index) const {
	if (index > _frames.size()) {
		warning("Tried to change a nonexistent frame (\"%d\") in a template animation. Ignoring call.",
		                 index);
		return false;
	} else
		return true;
}

void AnimationTemplate::setFPS(int FPS) {
	_FPS = FPS;
	_millisPerFrame = 1000000 / _FPS;
}

bool AnimationTemplate::persist(OutputPersistenceBlock &writer) {
	bool Result = true;

	// Parent persistieren.
	Result &= AnimationDescription::persist(writer);

	// Frameanzahl schreiben.
	writer.write(_frames.size());

	// Frames einzeln persistieren.
	Common::Array<const Frame>::const_iterator Iter = _frames.begin();
	while (Iter != _frames.end()) {
		writer.write(Iter->hotspotX);
		writer.write(Iter->hotspotY);
		writer.write(Iter->flipV);
		writer.write(Iter->flipH);
		writer.writeString(Iter->fileName);
		writer.writeString(Iter->action);
		++Iter;
	}

	// Restliche Member persistieren.
	writer.writeString(_sourceAnimationPtr->getFileName());
	writer.write(_valid);

	return Result;
}

bool AnimationTemplate::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	// Parent wieder herstellen.
	result &= AnimationDescription::unpersist(reader);

	// Frameanzahl lesen.
	uint frameCount;
	reader.read(frameCount);

	// Frames einzeln wieder herstellen.
	for (uint i = 0; i < frameCount; ++i) {
		Frame frame;
		reader.read(frame.hotspotX);
		reader.read(frame.hotspotY);
		reader.read(frame.flipV);
		reader.read(frame.flipH);
		reader.readString(frame.fileName);
		reader.readString(frame.action);

		_frames.push_back(frame);
	}

	// Die Animations-Resource wird für die gesamte Lebensdauer des Objektes gelockt
	Common::String sourceAnimation;
	reader.readString(sourceAnimation);
	_sourceAnimationPtr = requestSourceAnimation(sourceAnimation);

	reader.read(_valid);

	return _sourceAnimationPtr && reader.isGood() && result;
}

} // End of namespace Sword25
