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

#include "sword25/gfx/animation.h"

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resmanager.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/package/packagemanager.h"
#include "sword25/gfx/image/image.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"
#include "sword25/gfx/animationresource.h"
#include "sword25/gfx/bitmapresource.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

Animation::Animation(RenderObjectPtr<RenderObject> parentPtr, const Common::String &fileName) :
	TimedRenderObject(parentPtr, RenderObject::TYPE_ANIMATION) {
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!_initSuccess)
		return;

	initMembers();

	// Vom negativen Fall ausgehen.
	_initSuccess = false;

	initializeAnimationResource(fileName);

	// Erfolg signalisieren.
	_initSuccess = true;
}

Animation::Animation(RenderObjectPtr<RenderObject> parentPtr, const AnimationTemplate &templ) :
	TimedRenderObject(parentPtr, RenderObject::TYPE_ANIMATION) {
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!_initSuccess)
		return;

	initMembers();

	// Vom negativen Fall ausgehen.
	_initSuccess = false;

	_animationTemplateHandle = AnimationTemplate::create(templ);

	// Erfolg signalisieren.
	_initSuccess = true;
}

Animation::Animation(InputPersistenceBlock &reader, RenderObjectPtr<RenderObject> parentPtr, uint handle) :
	TimedRenderObject(parentPtr, RenderObject::TYPE_ANIMATION, handle) {
	// Das BS_RenderObject konnte nicht erzeugt werden, daher muss an dieser Stelle abgebrochen werden.
	if (!_initSuccess)
		return;

	initMembers();

	// Objekt vom Stream laden.
	_initSuccess = unpersist(reader);
}

void Animation::initializeAnimationResource(const Common::String &fileName) {
	// Die Resource wird für die gesamte Lebensdauer des Animations-Objektes gelockt.
	Resource *resourcePtr = Kernel::getInstance()->getResourceManager()->requestResource(fileName);
	if (resourcePtr && resourcePtr->getType() == Resource::TYPE_ANIMATION)
		_animationResourcePtr = static_cast<AnimationResource *>(resourcePtr);
	else {
		error("The resource \"%s\" could not be requested. The Animation can't be created.", fileName.c_str());
		return;
	}

	// Größe und Position der Animation anhand des aktuellen Frames bestimmen.
	computeCurrentCharacteristics();
}

void Animation::initMembers() {
	_currentFrame = 0;
	_currentFrameTime = 0;
	_direction = FORWARD;
	_running = false;
	_finished = false;
	_relX = 0;
	_relY = 0;
	_scaleFactorX = 1.0f;
	_scaleFactorY = 1.0f;
	_modulationColor = 0xffffffff;
	_animationResourcePtr = 0;
	_animationTemplateHandle = 0;
	_framesLocked = false;
}

Animation::~Animation() {
	if (getAnimationDescription()) {
		stop();
		getAnimationDescription()->unlock();
	}

	// Invoke the "delete" callback
	if (_deleteCallback)
		(_deleteCallback)(getHandle());

}

void Animation::play() {
	// If the animation was completed, then play it again from the start.
	if (_finished)
		stop();

	_running = true;
	lockAllFrames();
}

void Animation::pause() {
	_running = false;
	unlockAllFrames();
}

void Animation::stop() {
	_currentFrame = 0;
	_currentFrameTime = 0;
	_direction = FORWARD;
	pause();
}

void Animation::setFrame(uint nr) {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);

	if (nr >= animationDescriptionPtr->getFrameCount()) {
		error("Tried to set animation to illegal frame (%d). Value must be between 0 and %d.",
		               nr, animationDescriptionPtr->getFrameCount());
		return;
	}

	_currentFrame = nr;
	_currentFrameTime = 0;
	computeCurrentCharacteristics();
	forceRefresh();
}

bool Animation::doRender() {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	assert(_currentFrame < animationDescriptionPtr->getFrameCount());

	// Bitmap des aktuellen Frames holen
	Resource *pResource = Kernel::getInstance()->getResourceManager()->requestResource(animationDescriptionPtr->getFrame(_currentFrame).fileName);
	assert(pResource);
	assert(pResource->getType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmapResource = static_cast<BitmapResource *>(pResource);

	// Framebufferobjekt holen
	GraphicEngine *pGfx = Kernel::getInstance()->getGfx();
	assert(pGfx);

	// Bitmap zeichnen
	bool result;
	if (isScalingAllowed() && (_width != pBitmapResource->getWidth() || _height != pBitmapResource->getHeight())) {
		result = pBitmapResource->blit(_absoluteX, _absoluteY,
		                               (animationDescriptionPtr->getFrame(_currentFrame).flipV ? BitmapResource::FLIP_V : 0) |
		                               (animationDescriptionPtr->getFrame(_currentFrame).flipH ? BitmapResource::FLIP_H : 0),
		                               0, _modulationColor, _width, _height);
	} else {
		result = pBitmapResource->blit(_absoluteX, _absoluteY,
		                               (animationDescriptionPtr->getFrame(_currentFrame).flipV ? BitmapResource::FLIP_V : 0) |
		                               (animationDescriptionPtr->getFrame(_currentFrame).flipH ? BitmapResource::FLIP_H : 0),
		                               0, _modulationColor, -1, -1);
	}

	// Resource freigeben
	pBitmapResource->release();

	return result;
}

void Animation::frameNotification(int timeElapsed) {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	assert(timeElapsed >= 0);

	// Nur wenn die Animation läuft wird sie auch weiterbewegt
	if (_running) {
		// Gesamte vergangene Zeit bestimmen (inkl. Restzeit des aktuellen Frames)
		_currentFrameTime += timeElapsed;

		// Anzahl an zu überpringenden Frames bestimmen
		int skipFrames = animationDescriptionPtr->getMillisPerFrame() == 0 ? 0 : _currentFrameTime / animationDescriptionPtr->getMillisPerFrame();

		// Neue Frame-Restzeit bestimmen
		_currentFrameTime -= animationDescriptionPtr->getMillisPerFrame() * skipFrames;

		// Neuen Frame bestimmen (je nach aktuellener Abspielrichtung wird addiert oder subtrahiert)
		int tmpCurFrame = _currentFrame;
		switch (_direction) {
		case FORWARD:
			tmpCurFrame += skipFrames;
			break;

		case BACKWARD:
			tmpCurFrame -= skipFrames;
			break;

		default:
			assert(0);
		}

		// Deal with overflows
		if (tmpCurFrame < 0) {
			// Loop-Point callback
			if (_loopPointCallback && !(_loopPointCallback)(getHandle()))
				_loopPointCallback = 0;

			// An underflow may only occur if the animation type is JOJO.
			assert(animationDescriptionPtr->getAnimationType() == AT_JOJO);
			tmpCurFrame = - tmpCurFrame;
			_direction = FORWARD;
		} else if (static_cast<uint>(tmpCurFrame) >= animationDescriptionPtr->getFrameCount()) {
			// Loop-Point callback
			if (_loopPointCallback && !(_loopPointCallback)(getHandle()))
				_loopPointCallback = 0;

			switch (animationDescriptionPtr->getAnimationType()) {
			case AT_ONESHOT:
				tmpCurFrame = animationDescriptionPtr->getFrameCount() - 1;
				_finished = true;
				pause();
				break;

			case AT_LOOP:
				tmpCurFrame = tmpCurFrame % animationDescriptionPtr->getFrameCount();
				break;

			case AT_JOJO:
				tmpCurFrame = animationDescriptionPtr->getFrameCount() - (tmpCurFrame % animationDescriptionPtr->getFrameCount()) - 1;
				_direction = BACKWARD;
				break;

			default:
				assert(0);
			}
		}

		if ((int)_currentFrame != tmpCurFrame) {
			forceRefresh();

			if (animationDescriptionPtr->getFrame(_currentFrame).action != "") {
				// action callback
				if (_actionCallback && !(_actionCallback)(getHandle()))
					_actionCallback = 0;
			}
		}

		_currentFrame = static_cast<uint>(tmpCurFrame);
	}

	// Größe und Position der Animation anhand des aktuellen Frames bestimmen
	computeCurrentCharacteristics();

	assert(_currentFrame < animationDescriptionPtr->getFrameCount());
	assert(_currentFrameTime >= 0);
}

void Animation::computeCurrentCharacteristics() {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	const AnimationResource::Frame &curFrame = animationDescriptionPtr->getFrame(_currentFrame);

	Resource *pResource = Kernel::getInstance()->getResourceManager()->requestResource(curFrame.fileName);
	assert(pResource);
	assert(pResource->getType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmap = static_cast<BitmapResource *>(pResource);

	// Größe des Bitmaps auf die Animation übertragen
	_width = static_cast<int>(pBitmap->getWidth() * _scaleFactorX);
	_height = static_cast<int>(pBitmap->getHeight() * _scaleFactorY);

	// Position anhand des Hotspots berechnen und setzen
	int posX = _relX + computeXModifier();
	int posY = _relY + computeYModifier();

	RenderObject::setPos(posX, posY);

	pBitmap->release();
}

bool Animation::lockAllFrames() {
	if (!_framesLocked) {
		AnimationDescription *animationDescriptionPtr = getAnimationDescription();
		assert(animationDescriptionPtr);
		for (uint i = 0; i < animationDescriptionPtr->getFrameCount(); ++i) {
			if (!Kernel::getInstance()->getResourceManager()->requestResource(animationDescriptionPtr->getFrame(i).fileName)) {
				error("Could not lock all animation frames.");
				return false;
			}
		}

		_framesLocked = true;
	}

	return true;
}

bool Animation::unlockAllFrames() {
	if (_framesLocked) {
		AnimationDescription *animationDescriptionPtr = getAnimationDescription();
		assert(animationDescriptionPtr);
		for (uint i = 0; i < animationDescriptionPtr->getFrameCount(); ++i) {
			Resource *pResource;
			if (!(pResource = Kernel::getInstance()->getResourceManager()->requestResource(animationDescriptionPtr->getFrame(i).fileName))) {
				error("Could not unlock all animation frames.");
				return false;
			}

			// Zwei mal freigeben um den Request von LockAllFrames() und den jetzigen Request aufzuheben
			pResource->release();
			if (pResource->getLockCount())
				pResource->release();
		}

		_framesLocked = false;
	}

	return true;
}

Animation::ANIMATION_TYPES Animation::getAnimationType() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	return animationDescriptionPtr->getAnimationType();
}

int Animation::getFPS() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	return animationDescriptionPtr->getFPS();
}

int Animation::getFrameCount() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	return animationDescriptionPtr->getFrameCount();
}

bool Animation::isScalingAllowed() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	return animationDescriptionPtr->isScalingAllowed();
}

bool Animation::isAlphaAllowed() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	return animationDescriptionPtr->isAlphaAllowed();
}

bool Animation::isColorModulationAllowed() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	return animationDescriptionPtr->isColorModulationAllowed();
}

void Animation::setPos(int relX, int relY) {
	_relX = relX;
	_relY = relY;

	computeCurrentCharacteristics();
}

void Animation::setX(int relX) {
	_relX = relX;

	computeCurrentCharacteristics();
}

void Animation::setY(int relY) {
	_relY = relY;

	computeCurrentCharacteristics();
}

void Animation::setAlpha(int alpha) {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	if (!animationDescriptionPtr->isAlphaAllowed()) {
		warning("Tried to set alpha value on an animation that does not support alpha. Call was ignored.");
		return;
	}

	uint newModulationColor = (_modulationColor & 0x00ffffff) | alpha << 24;
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

void Animation::setModulationColor(uint modulationColor) {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	if (!animationDescriptionPtr->isColorModulationAllowed()) {
		warning("Tried to set modulation color on an animation that does not support color modulation. Call was ignored");
		return;
	}

	uint newModulationColor = (modulationColor & 0x00ffffff) | (_modulationColor & 0xff000000);
	if (newModulationColor != _modulationColor) {
		_modulationColor = newModulationColor;
		forceRefresh();
	}
}

void Animation::setScaleFactor(float scaleFactor) {
	setScaleFactorX(scaleFactor);
	setScaleFactorY(scaleFactor);
}

void Animation::setScaleFactorX(float scaleFactorX) {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	if (!animationDescriptionPtr->isScalingAllowed()) {
		warning("Tried to set x scale factor on an animation that does not support scaling. Call was ignored");
		return;
	}

	if (scaleFactorX != _scaleFactorX) {
		_scaleFactorX = scaleFactorX;
		if (_scaleFactorX <= 0.0f)
			_scaleFactorX = 0.001f;
		forceRefresh();
		computeCurrentCharacteristics();
	}
}

void Animation::setScaleFactorY(float scaleFactorY) {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	if (!animationDescriptionPtr->isScalingAllowed()) {
		warning("Tried to set y scale factor on an animation that does not support scaling. Call was ignored");
		return;
	}

	if (scaleFactorY != _scaleFactorY) {
		_scaleFactorY = scaleFactorY;
		if (_scaleFactorY <= 0.0f)
			_scaleFactorY = 0.001f;
		forceRefresh();
		computeCurrentCharacteristics();
	}
}

const Common::String &Animation::getCurrentAction() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	return animationDescriptionPtr->getFrame(_currentFrame).action;
}

int Animation::getX() const {
	return _relX;
}

int Animation::getY() const {
	return _relY;
}

int Animation::getAbsoluteX() const {
	return _absoluteX + (_relX - _x);
}

int Animation::getAbsoluteY() const {
	return _absoluteY + (_relY - _y);
}

int Animation::computeXModifier() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	const AnimationResource::Frame &curFrame = animationDescriptionPtr->getFrame(_currentFrame);

	Resource *pResource = Kernel::getInstance()->getResourceManager()->requestResource(curFrame.fileName);
	assert(pResource);
	assert(pResource->getType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmap = static_cast<BitmapResource *>(pResource);

	int result = curFrame.flipV ? - static_cast<int>((pBitmap->getWidth() - 1 - curFrame.hotspotX) * _scaleFactorX) :
	             - static_cast<int>(curFrame.hotspotX * _scaleFactorX);

	pBitmap->release();

	return result;
}

int Animation::computeYModifier() const {
	AnimationDescription *animationDescriptionPtr = getAnimationDescription();
	assert(animationDescriptionPtr);
	const AnimationResource::Frame &curFrame = animationDescriptionPtr->getFrame(_currentFrame);

	Resource *pResource = Kernel::getInstance()->getResourceManager()->requestResource(curFrame.fileName);
	assert(pResource);
	assert(pResource->getType() == Resource::TYPE_BITMAP);
	BitmapResource *pBitmap = static_cast<BitmapResource *>(pResource);

	int result = curFrame.flipH ? - static_cast<int>((pBitmap->getHeight() - 1 - curFrame.hotspotY) * _scaleFactorY) :
	             - static_cast<int>(curFrame.hotspotY * _scaleFactorY);

	pBitmap->release();

	return result;
}

bool Animation::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	result &= RenderObject::persist(writer);

	writer.write(_relX);
	writer.write(_relY);
	writer.write(_scaleFactorX);
	writer.write(_scaleFactorY);
	writer.write(_modulationColor);
	writer.write(_currentFrame);
	writer.write(_currentFrameTime);
	writer.write(_running);
	writer.write(_finished);
	writer.write(static_cast<uint>(_direction));

	// Je nach Animationstyp entweder das Template oder die Ressource speichern.
	if (_animationResourcePtr) {
		uint marker = 0;
		writer.write(marker);
		writer.writeString(_animationResourcePtr->getFileName());
	} else if (_animationTemplateHandle) {
		uint marker = 1;
		writer.write(marker);
		writer.write(_animationTemplateHandle);
	} else {
		assert(false);
	}

	//writer.write(_AnimationDescriptionPtr);

	writer.write(_framesLocked);

	// The following is only there to for compatibility with older saves
	// resp. the original engine.
	writer.write((uint)1);
	writer.writeString("LuaLoopPointCB");
	writer.write(getHandle());
	writer.write((uint)1);
	writer.writeString("LuaActionCB");
	writer.write(getHandle());
	writer.write((uint)1);
	writer.writeString("LuaDeleteCB");
	writer.write(getHandle());

	result &= RenderObject::persistChildren(writer);

	return result;
}

// -----------------------------------------------------------------------------

bool Animation::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	result &= RenderObject::unpersist(reader);

	reader.read(_relX);
	reader.read(_relY);
	reader.read(_scaleFactorX);
	reader.read(_scaleFactorY);
	reader.read(_modulationColor);
	reader.read(_currentFrame);
	reader.read(_currentFrameTime);
	reader.read(_running);
	reader.read(_finished);
	uint direction;
	reader.read(direction);
	_direction = static_cast<Direction>(direction);

	// Animationstyp einlesen.
	uint marker;
	reader.read(marker);
	if (marker == 0) {
		Common::String resourceFilename;
		reader.readString(resourceFilename);
		initializeAnimationResource(resourceFilename);
	} else if (marker == 1) {
		reader.read(_animationTemplateHandle);
	} else {
		assert(false);
	}

	reader.read(_framesLocked);
	if (_framesLocked)
		lockAllFrames();


	// The following is only there to for compatibility with older saves
	// resp. the original engine.
	uint callbackCount;
	Common::String callbackFunctionName;
	uint callbackData;

	// loop point callback
	reader.read(callbackCount);
	assert(callbackCount == 1);
	reader.readString(callbackFunctionName);
	assert(callbackFunctionName == "LuaLoopPointCB");
	reader.read(callbackData);
	assert(callbackData == getHandle());

	// loop point callback
	reader.read(callbackCount);
	assert(callbackCount == 1);
	reader.readString(callbackFunctionName);
	assert(callbackFunctionName == "LuaActionCB");
	reader.read(callbackData);
	assert(callbackData == getHandle());

	// loop point callback
	reader.read(callbackCount);
	assert(callbackCount == 1);
	reader.readString(callbackFunctionName);
	assert(callbackFunctionName == "LuaDeleteCB");
	reader.read(callbackData);
	assert(callbackData == getHandle());

	// Set the callbacks
	setCallbacks();

	result &= RenderObject::unpersistChildren(reader);

	return reader.isGood() && result;
}

// -----------------------------------------------------------------------------

AnimationDescription *Animation::getAnimationDescription() const {
	if (_animationResourcePtr)
		return _animationResourcePtr;
	else
		return AnimationTemplateRegistry::instance().resolveHandle(_animationTemplateHandle);
}

} // End of namespace Sword25
