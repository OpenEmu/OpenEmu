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

#include "common/macresman.h"
#include "common/stream.h"

#include "pegasus/elements.h"
#include "pegasus/graphics.h"
#include "pegasus/surface.h"

namespace Pegasus {

DisplayElement::DisplayElement(const DisplayElementID id) : IDObject(id) {
	_elementIsDisplaying = false;
	_elementIsVisible = false;
	_elementOrder = 0;
	_triggeredElement = this;
	_nextElement = 0;
}

DisplayElement::~DisplayElement() {
	if (isDisplaying())
		((PegasusEngine *)g_engine)->_gfx->removeDisplayElement(this);
}

void DisplayElement::setDisplayOrder(const DisplayOrder order) {
	if (_elementOrder != order) {
		_elementOrder = order;
		if (isDisplaying()) {
			((PegasusEngine *)g_engine)->_gfx->removeDisplayElement(this);
			((PegasusEngine *)g_engine)->_gfx->addDisplayElement(this);
			triggerRedraw();
		}
	}
}

void DisplayElement::startDisplaying() {
	if (!isDisplaying()) {
		((PegasusEngine *)g_engine)->_gfx->addDisplayElement(this);
		triggerRedraw();
	}
}

void DisplayElement::stopDisplaying() {
	if (isDisplaying()) {
		triggerRedraw();
		((PegasusEngine *)g_engine)->_gfx->removeDisplayElement(this);
	}
}

void DisplayElement::setBounds(const CoordType left, const CoordType top, const CoordType right, const CoordType bottom) {
	setBounds(Common::Rect(left, top, right, bottom));
}

void DisplayElement::getBounds(Common::Rect &r) const {
	r = _bounds;
}

void DisplayElement::sizeElement(const CoordType h, const CoordType v) {
	Common::Rect newBounds = _bounds;
	newBounds.right = _bounds.left + h;
	newBounds.bottom = _bounds.top + v;
	setBounds(newBounds);
}

void DisplayElement::moveElementTo(const CoordType h, const CoordType v) {
	Common::Rect newBounds = _bounds;
	newBounds.moveTo(h, v);
	setBounds(newBounds);
}

void DisplayElement::moveElement(const CoordType dh, const CoordType dv) {
	Common::Rect newBounds = _bounds;
	newBounds.translate(dh, dv);
	setBounds(newBounds);
}

void DisplayElement::getLocation(CoordType &h, CoordType &v) const {
	h = _bounds.left;
	v = _bounds.top;
}

void DisplayElement::centerElementAt(const CoordType h, const CoordType v) {
	Common::Rect newBounds = _bounds;
	newBounds.moveTo(h - (_bounds.width() / 2), v - (_bounds.height() / 2));
	setBounds(newBounds);
}

void DisplayElement::getCenter(CoordType &h, CoordType &v) const {
	h = (_bounds.left + _bounds.right) / 2;
	v = (_bounds.top + _bounds.bottom) / 2;
}

void DisplayElement::setBounds(const Common::Rect &r) {
	if (r != _bounds) {
		triggerRedraw();
		_bounds = r;
		triggerRedraw();
	}
}

void DisplayElement::hide() {
	if (_elementIsVisible) {
		triggerRedraw();
		_elementIsVisible = false;
	}
}

void DisplayElement::show() {
	if (!_elementIsVisible) {
		_elementIsVisible = true;
		triggerRedraw();
	}
}

// Only invalidates this element's bounding rectangle if all these conditions are true:
// -- The triggered element is this element.
// -- The element is displaying on the display list.
// -- The element is visible.
// -- The element is part of the active layer OR is one of the reserved items.
void DisplayElement::triggerRedraw() {
	GraphicsManager *gfx = ((PegasusEngine *)g_engine)->_gfx;

	if (_triggeredElement == this) {
		if (validToDraw(gfx->getBackOfActiveLayer(), gfx->getFrontOfActiveLayer()))
			gfx->invalRect(_bounds);
	} else {
		_triggeredElement->triggerRedraw();
	}
}

void DisplayElement::setTriggeredElement(DisplayElement *element) {
	if (element)
		_triggeredElement = element;
	else
		_triggeredElement = this;
}

bool DisplayElement::validToDraw(DisplayOrder backLayer, DisplayOrder frontLayer) {
	return	isDisplaying() && _elementIsVisible &&
			(getObjectID() <= kHighestReservedElementID ||
			(getDisplayOrder() >= backLayer &&
			getDisplayOrder() <= frontLayer));
}

DropHighlight::DropHighlight(const DisplayElementID id) : DisplayElement(id) {
	_highlightColor = 0;
	_thickness = 2;
	_cornerDiameter = 0;
}

void DropHighlight::draw(const Common::Rect &) {
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();

	// Since this is only used in two different ways, I'm only
	// going to implement it in those two ways. Deal with it.

	Common::Rect rect = _bounds;
	rect.grow(-_thickness);
	screen->frameRect(rect, _highlightColor);
	rect.grow(1);
	screen->frameRect(rect, _highlightColor);

	if (_cornerDiameter == 8 && _thickness == 4) {
		rect.grow(1);
		screen->frameRect(rect, _highlightColor);
		screen->hLine(rect.left + 1, rect.top - 1, rect.right - 2, _highlightColor);
		screen->hLine(rect.left + 1, rect.bottom, rect.right - 2, _highlightColor);
		screen->vLine(rect.left - 1, rect.top + 1, rect.bottom - 2, _highlightColor);
		screen->vLine(rect.right, rect.top + 1, rect.bottom - 2, _highlightColor);
	}
}

IdlerAnimation::IdlerAnimation(const DisplayElementID id) : Animation(id) {
	_lastTime = 0xffffffff;
}

void IdlerAnimation::startDisplaying() {
	if (!isDisplaying()) {
		Animation::startDisplaying();
		startIdling();
	}
}

void IdlerAnimation::stopDisplaying() {
	if (isDisplaying()) {
		Animation::stopDisplaying();
		stopIdling();
	}
}

void IdlerAnimation::useIdleTime() {
	uint32 currentTime = getTime();

	if (currentTime != _lastTime) {
		_lastTime = currentTime;
		timeChanged(_lastTime);
	}
}

void IdlerAnimation::timeChanged(const TimeValue) {
	triggerRedraw();
}

FrameSequence::FrameSequence(const DisplayElementID id) : IdlerAnimation(id) {
	_duration = 0;
	_currentFrameNum = 0;
	_resFork = new Common::MacResManager();
	_numFrames = 0;
}

FrameSequence::~FrameSequence() {
	delete _resFork;
}

void FrameSequence::useFileName(const Common::String &fileName) {
	_resFork->open(fileName);
}

void FrameSequence::openFrameSequence() {
	if (!_resFork->hasResFork())
		return;

	Common::SeekableReadStream *res = _resFork->getResource(MKTAG('P', 'F', 'r', 'm'), 0x80);

	if (!res)
		return;

	uint32 scale = res->readUint32BE();
	_bounds.top = res->readUint16BE();
	_bounds.left = res->readUint16BE();
	_bounds.bottom = res->readUint16BE();
	_bounds.right = res->readUint16BE();
	_numFrames = res->readUint16BE();
	_duration = 0;

	_frameTimes.clear();
	for (uint32 i = 0; i < _numFrames; i++) {
		TimeValue time = res->readUint32BE();
		_duration += time;
		_frameTimes.push_back(_duration);
	}

	setScale(scale);
	setSegment(0, _duration);
	setTime(0);
	_currentFrameNum = 0;
	newFrame(_currentFrameNum);
	triggerRedraw();

	delete res;
}

void FrameSequence::closeFrameSequence() {
	stop();
	_resFork->close();
	_duration = 0;
	_numFrames = 0;
	_frameTimes.clear();
}

void FrameSequence::timeChanged(const TimeValue time) {
	int16 frameNum = 0;
	for (int16 i = _numFrames - 1; i >= 0; i--) {
		if (_frameTimes[i] < time) {
			frameNum = i;
			break;
		}
	}

	if (frameNum != _currentFrameNum) {
		_currentFrameNum = frameNum;
		newFrame(_currentFrameNum);
		triggerRedraw();
	}
}

void FrameSequence::setFrameNum(const int16 frameNum) {
	int16 f = CLIP<int>(frameNum, 0, _numFrames);

	if (_currentFrameNum != f) {
		_currentFrameNum = f;
		setTime(_frameTimes[f]);
		newFrame(f);
		triggerRedraw();
	}
}

bool FrameSequence::isSequenceOpen() const {
	return _numFrames != 0;
}

Sprite::Sprite(const DisplayElementID id) : DisplayElement(id) {
	_numFrames = 0;
	_currentFrameNum = 0xffffffff;
	_currentFrame = 0;
}

Sprite::~Sprite() {
	discardFrames();
}

void Sprite::discardFrames() {
	if (!_frameArray.empty()) {
		for (uint32 i = 0; i < _numFrames; i++) {
			SpriteFrame *frame = _frameArray[i].frame;
			frame->_referenceCount--;
			if (frame->_referenceCount == 0)
				delete frame;
		}

		_frameArray.clear();
		_numFrames = 0;
		_currentFrame = 0;
		_currentFrameNum = 0xffffffff;
		setBounds(0, 0, 0, 0);
	}
}

void Sprite::addPICTResourceFrame(const ResIDType pictID, bool transparent, const CoordType left, const CoordType top) {
	SpriteFrame *frame = new SpriteFrame();
	frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, pictID, transparent);
	addFrame(frame, left, top);
}

uint32 Sprite::addFrame(SpriteFrame *frame, const CoordType left, const CoordType top) {
	SpriteFrameRec frameRecord;
	frameRecord.frame = frame;
	frameRecord.frameLeft = left;
	frameRecord.frameTop = top;
	_frameArray.push_back(frameRecord);
	_numFrames++;
	frame->_referenceCount++;

	Common::Rect frameBounds;
	frame->getSurfaceBounds(frameBounds);

	// 9/3/96
	// BB Should this be + left or - left?
	frameBounds.moveTo(_bounds.left + left, _bounds.top + top);

	frameBounds.extend(_bounds);

	if (_bounds != frameBounds)
		setBounds(frameBounds);

	return _numFrames - 1;
}

void Sprite::removeFrame(const uint32 frameNum) {
	_frameArray[frameNum].frame->_referenceCount--;
	if (_frameArray[frameNum].frame->_referenceCount == 0)
		delete _frameArray[frameNum].frame;

	// Calculate the new bounds
	Common::Rect frameBounds;
	for (uint32 i = 0; i < _numFrames; i++) {
		if (i == frameNum)
			continue;

		Common::Rect r;
		_frameArray[i].frame->getSurfaceBounds(r);
		r.translate(_frameArray[i].frameLeft, _frameArray[i].frameTop);
		frameBounds.extend(r);
	}

	_frameArray.remove_at(frameNum);

	frameBounds.moveTo(_bounds.left, _bounds.top);
	setBounds(frameBounds);

	if (_currentFrameNum == frameNum)
		triggerRedraw();
	else if (_currentFrameNum != 0xffffffff && _currentFrameNum > frameNum)
		--_currentFrameNum;
}

void Sprite::setCurrentFrameIndex(const int32 frameNum) {
	if (frameNum < 0) {
		if (_currentFrameNum != 0xffffffff) {
			_currentFrameNum = 0xffffffff;
			_currentFrame = 0;
			triggerRedraw();
		}
	} else if (_numFrames > 0) {
		uint32 f = frameNum % _numFrames;
		if (f != _currentFrameNum) {
			_currentFrameNum = f;
			_currentFrame = &_frameArray[f];
			triggerRedraw();
		}
	}
}

SpriteFrame *Sprite::getFrame(const int32 index) {
	if (index < 0 || (uint32)index >= _numFrames)
		return 0;

	return _frameArray[index].frame;
}

void Sprite::draw(const Common::Rect &r) {
	if (_currentFrame) {
		Common::Rect frameBounds;
		_currentFrame->frame->getSurfaceBounds(frameBounds);

		frameBounds.translate(_bounds.left + _currentFrame->frameLeft, _bounds.top + _currentFrame->frameTop);
		Common::Rect r1 = frameBounds.findIntersectingRect(r);

		Common::Rect r2 = frameBounds;
		r2.translate(-_bounds.left - _currentFrame->frameLeft, -_bounds.top - _currentFrame->frameTop);

		_currentFrame->frame->drawImage(r2, r1);
	}
}

SpriteSequence::SpriteSequence(const DisplayElementID id, const DisplayElementID spriteID) :
		FrameSequence(id), _sprite(spriteID), _transparent(false) {
}

void SpriteSequence::openFrameSequence() {
	if (!isSequenceOpen()) {
		FrameSequence::openFrameSequence();

		if (isSequenceOpen()) {
			uint32 numFrames = getNumFrames();

			for (uint32 i = 0; i < numFrames; ++i) {
				SpriteFrame *frame = new SpriteFrame();
				frame->initFromPICTResource(_resFork, i + 0x80, _transparent);
				_sprite.addFrame(frame, 0, 0);
			}

			_sprite.setBounds(_bounds);
		}
	}
}

void SpriteSequence::closeFrameSequence() {
	if (isSequenceOpen()) {
		FrameSequence::closeFrameSequence();
		_sprite.discardFrames();
	}
}

void SpriteSequence::setBounds(const Common::Rect &bounds) {
	FrameSequence::setBounds(bounds);
	_sprite.setBounds(_bounds);
}

void SpriteSequence::draw(const Common::Rect &r) {
	_sprite.draw(r);
}

void SpriteSequence::newFrame(const uint16 frame) {
	_sprite.setCurrentFrameIndex(frame);
}

#define DRAW_PIXEL() \
	if (bytesPerPixel == 2) \
		*((uint16 *)dst) = black; \
	else \
		*((uint32 *)dst) = black; \
	dst += bytesPerPixel

#define SKIP_PIXEL() \
	dst += bytesPerPixel

void ScreenDimmer::draw(const Common::Rect &r) {
	// We're going to emulate QuickDraw's srcOr+gray mode here
	// In this mode, every other y column is all black (odd-columns).
	// Basically, every row does three black and then one transparent
	// repeatedly.

	// The output is identical to the original

	uint32 black = g_system->getScreenFormat().RGBToColor(0, 0, 0);
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();
	byte bytesPerPixel = g_system->getScreenFormat().bytesPerPixel;

	// We're currently doing it to the whole screen to simplify the code

	for (int y = 0; y < 480; y++) {
		byte *dst = (byte *)screen->getBasePtr(0, y);

		for (int x = 0; x < 640; x += 4) {
			if (y & 1) {
				DRAW_PIXEL();
				DRAW_PIXEL();
				SKIP_PIXEL();
				DRAW_PIXEL();
			} else {
				SKIP_PIXEL();
				DRAW_PIXEL();
				DRAW_PIXEL();
				DRAW_PIXEL();
			}
		}
	}
}

#undef DRAW_PIXEL
#undef SKIP_PIXEL

SoundLevel::SoundLevel(const DisplayElementID id) : DisplayElement(id) {
	_soundLevel = 0;
}

void SoundLevel::incrementLevel() {
	if (_soundLevel < 12) {
		_soundLevel++;
		triggerRedraw();
	}
}

void SoundLevel::decrementLevel() {
	if (_soundLevel > 0) {
		_soundLevel--;
		triggerRedraw();
	}
}

uint16 SoundLevel::getSoundLevel() {
	return CLIP<int>(_soundLevel * 22, 0, 256);
}

void SoundLevel::setSoundLevel(uint16 level) {
	uint16 newLevel = (level + 21) / 22;

	if (newLevel != _soundLevel) {
		_soundLevel = newLevel;
		triggerRedraw();
	}
}

void SoundLevel::draw(const Common::Rect &r) {
	Common::Rect levelRect(_bounds.right + (8 * (_soundLevel - 12)), _bounds.top, _bounds.right, _bounds.bottom);
	levelRect = r.findIntersectingRect(levelRect);

	if (!levelRect.isEmpty()) {
		Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();
		screen->fillRect(levelRect, g_system->getScreenFormat().RGBToColor(0, 0, 0));
	}
}

} // End of namespace Pegasus
