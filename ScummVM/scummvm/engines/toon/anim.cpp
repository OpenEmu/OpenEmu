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

#include "common/debug.h"
#include "common/rect.h"

#include "toon/anim.h"
#include "toon/toon.h"
#include "toon/tools.h"

namespace Toon {

bool Animation::loadAnimation(const Common::String &file) {
	debugC(1, kDebugAnim, "loadAnimation(%s)", file.c_str());

	uint32 fileSize = 0;
	uint8 *fileData = _vm->resources()->getFileData(file, &fileSize);
	if (!fileData)
		return false;

	strcpy(_name, "not_loaded");
	if (strncmp((char *)fileData, "KevinAguilar", 12))
		return false;

	strcpy(_name, file.c_str());

	uint32 headerSize = READ_LE_UINT32(fileData + 16);
	uint32 uncompressedBytes = READ_LE_UINT32(fileData + 20);
	uint32 compressedBytes = READ_LE_UINT32(fileData + 24);
	_numFrames = READ_LE_UINT32(fileData + 28);
	_x1 = READ_LE_UINT32(fileData + 32);
	_y1 = READ_LE_UINT32(fileData + 36);
	_x2 = READ_LE_UINT32(fileData + 40);
	_y2 = READ_LE_UINT32(fileData + 44);
	_paletteEntries = READ_LE_UINT32(fileData + 56);
	_fps = READ_LE_UINT32(fileData + 60);
	uint32 paletteSize = READ_LE_UINT32(fileData + 64);

	uint8 *currentData = fileData + 68;
	if (_paletteEntries) {
		if (paletteSize) {
			delete[] _palette;
			_palette = new uint8[paletteSize];
			memcpy(_palette, currentData, paletteSize);
			currentData += paletteSize;
		} else {
			_palette = 0;
		}
	}

	byte *finalBuffer = new byte[uncompressedBytes];
	if (uncompressedBytes > compressedBytes)
		decompressLZSS(currentData, finalBuffer, uncompressedBytes);
	else
		memcpy(finalBuffer, currentData, uncompressedBytes);

	if (READ_LE_UINT32(finalBuffer) == 0x12345678) {
		uint8 *data = finalBuffer;
		delete[] _frames;
		_frames = new AnimationFrame[_numFrames];
		for (int32 e = 0; e < _numFrames; e++) {
			if (READ_LE_UINT32(data) != 0x12345678)
				return false;

			int32 oldRef = READ_LE_UINT32(data + 4);
			uint32 compressedSize = READ_LE_UINT32(data + 8);
			uint32 decompressedSize = READ_LE_UINT32(data + 12);

			_frames[e]._x1 = READ_LE_UINT32(data + 16);
			_frames[e]._y1 = READ_LE_UINT32(data + 20);
			_frames[e]._x2 = READ_LE_UINT32(data + 24);
			_frames[e]._y2 = READ_LE_UINT32(data + 28);

			uint8 *imageData = data + headerSize;
			if (oldRef != -1 || decompressedSize == 0) {
				_frames[e]._ref = oldRef;
				_frames[e]._data = 0;
			} else {
				_frames[e]._ref = -1;
				_frames[e]._data = new uint8[decompressedSize];
				if (compressedSize < decompressedSize) {
					decompressLZSS(imageData, _frames[e]._data, decompressedSize);
				} else {
					memcpy(_frames[e]._data, imageData, compressedSize);
				}
			}

			data += headerSize + compressedSize;
		}
	}

	_vm->resources()->purgeFileData();
	delete[] finalBuffer;
	return true;
}

Animation::Animation(ToonEngine *vm) : _vm(vm) {
	_palette = NULL;
	_numFrames = 0;
	_frames = NULL;
}

Animation::~Animation() {
	delete[] _palette;
	for (int32 i = 0; i < _numFrames; i++) {
		delete[] _frames[i]._data;
	}
	delete[] _frames;
}

Common::Rect Animation::getRect() {
	debugC(5, kDebugAnim, "getRect");
	return Common::Rect(_x1, _y1, _x2, _y2);
}

void Animation::drawFrame(Graphics::Surface &surface, int32 frame, int16 xx, int16 yy) {
	debugC(3, kDebugAnim, "drawFrame(surface, %d, %d, %d)", frame, xx, yy);
	if (frame < 0)
		frame = 0;

	if (frame >= _numFrames)
		frame = _numFrames - 1;

	if (_numFrames == 0)
		return;

	if (_frames[frame]._ref != -1)
		frame = _frames[frame]._ref;

	if (!_frames[frame]._data)
		return;

	int16 rectX = _frames[frame]._x2 - _frames[frame]._x1;
	int16 rectY = _frames[frame]._y2 - _frames[frame]._y1;
	int16 offsX = 0;
	int16 offsY = 0;

	_vm->addDirtyRect(xx + _x1 + _frames[frame]._x1, yy + _y1 + _frames[frame]._y1, xx + rectX + _x1 + _frames[frame]._x1 , yy + rectY + _y1 + _frames[frame]._y1);

	if (xx + _x1 + _frames[frame]._x1 < 0) {
		offsX = -(xx + _x1 + _frames[frame]._x1);
	}

	if (offsX >= rectX)
		return;
	else
		rectX -= offsX;

	if (yy + _y1 + _frames[frame]._y1 < 0) {
		offsY = -(yy + _y1 + _frames[frame]._y1);
	}

	if (offsY >= rectY)
		return;
	else
		rectY -= offsY;

	if (rectX + xx + _x1 + _frames[frame]._x1 >= surface.w)
		rectX = surface.w - xx - _x1 - _frames[frame]._x1;

	if (rectX < 0)
		return;

	if (rectY + yy + _y1 + _frames[frame]._y1 >= surface.h)
		rectY = surface.h - yy - _y1 - _frames[frame]._y1;

	if (rectY < 0)
		return;

	int32 destPitch = surface.pitch;
	uint8 *srcRow = _frames[frame]._data + offsX + (_frames[frame]._x2 - _frames[frame]._x1) * offsY;
	uint8 *curRow = (uint8 *)surface.pixels + (yy + _frames[frame]._y1 + _y1 + offsY) * destPitch + (xx + _x1 + _frames[frame]._x1 + offsX);
	for (int16 y = 0; y < rectY; y++) {
		uint8 *cur = curRow;
		uint8 *c = srcRow + y * (_frames[frame]._x2 - _frames[frame]._x1);
		for (int16 x = 0; x < rectX; x++) {
			if (*c)
				*cur = *c;
			c++;
			cur++;
		}
		curRow += destPitch;
	}
}

void Animation::drawFrameWithMask(Graphics::Surface &surface, int32 frame, int16 xx, int16 yy, int32 zz, Picture *mask) {
	debugC(1, kDebugAnim, "drawFrameWithMask(surface, %d, %d, %d, %d, mask)", frame, xx, yy, zz);
	warning("STUB: drawFrameWithMask()");
}

void Animation::drawFrameWithMaskAndScale(Graphics::Surface &surface, int32 frame, int16 xx, int16 yy, int32 zz, Picture *mask, int32 scale) {
	debugC(5, kDebugAnim, "drawFrameWithMaskAndScale(surface, %d, %d, %d, %d, mask, %d)", frame, xx, yy, zz, scale);
	if (_frames[frame]._ref != -1)
		frame = _frames[frame]._ref;
	int16 rectX = _frames[frame]._x2 - _frames[frame]._x1;
	int16 rectY = _frames[frame]._y2 - _frames[frame]._y1;

	int16 finalWidth = rectX * scale / 1024;
	int16 finalHeight = rectY * scale / 1024;

	// compute final x1, y1, x2, y2
	int16 xx1 = xx + _x1 + _frames[frame]._x1 * scale / 1024;
	int16 yy1 = yy + _y1 + _frames[frame]._y1 * scale / 1024;
	int16 xx2 = xx1 + finalWidth;
	int16 yy2 = yy1 + finalHeight;
	int16 w = _frames[frame]._x2 - _frames[frame]._x1;

	_vm->addDirtyRect(xx1, yy1, xx2, yy2);

	int32 destPitch = surface.pitch;
	int32 destPitchMask = mask->getWidth();
	uint8 *c = _frames[frame]._data;
	uint8 *curRow = (uint8 *)surface.pixels;
	uint8 *curRowMask = mask->getDataPtr();

	bool shadowFlag = false;
	if (strstr(_name, "SHADOW"))
		shadowFlag = true;

	for (int16 y = yy1; y < yy2; y++) {
		for (int16 x = xx1; x < xx2; x++) {
			if (x < 0 || x >= 1280 || y < 0 || y >= 400)
				continue;

			uint8 *cur = curRow + x + y * destPitch;
			uint8 *curMask = curRowMask + x + y * destPitchMask;

			// find the good c
			int16 xs = (x - xx1) * 1024 / scale;
			int16 ys = (y - yy1) * 1024 / scale;
			uint8 *cc = &c[ys * w + xs];
			if (*cc && ((*curMask) >= zz)) {
				if (shadowFlag)
					*cur = _vm->getShadowLUT()[*cur];
				else
					*cur = *cc;
			}
		}
	}
}

void Animation::applyPalette(int32 offset, int32 srcOffset, int32 numEntries) {
	debugC(1, kDebugAnim, "applyPalette(%d, %d, %d)", offset, srcOffset, numEntries);
	_vm->setPaletteEntries(_palette + srcOffset, offset, numEntries);
}

Common::Rect Animation::getFrameRect(int32 frame) {
	debugC(4, kDebugAnim, "getFrameRect(%d)", frame);
	if ((frame < 0) || (frame >= _numFrames)) {
		return Common::Rect();
	}

	if (_frames[frame]._ref != -1)
		frame = _frames[frame]._ref;

	return Common::Rect(_frames[frame]._x1, _frames[frame]._y1, _frames[frame]._x2, _frames[frame]._y2);
}

int16 Animation::getFrameWidth(int32 frame) {
	debugC(4, kDebugAnim, "getFrameWidth(%d)", frame);
	if ((frame < 0) || (frame >= _numFrames))
		return 0;

	if (_frames[frame]._ref != -1)
		frame = _frames[frame]._ref;

	return _frames[frame]._x2 - _frames[frame]._x1;
}

int16 Animation::getFrameHeight(int32 frame) {
	debugC(4, kDebugAnim, "getFrameHeight(%d)", frame);
	if (frame < 0 || frame >= _numFrames)
		return 0;

	if (_frames[frame]._ref != -1)
		frame = _frames[frame]._ref;

	return _frames[frame]._y2 - _frames[frame]._y1;
}

int16 Animation::getWidth() const {
	return _x2 - _x1;
}

int16 Animation::getHeight() const {
	return _y2 - _y1;
}

void Animation::drawFontFrame(Graphics::Surface &surface, int32 frame, int16 xx, int16 yy, byte *colorMap) {
	debugC(4, kDebugAnim, "drawFontFrame(surface, %d, %d, %d, colorMap)", frame, xx, yy);
	if (frame < 0)
		frame = 0;

	if (frame >= _numFrames)
		frame = _numFrames - 1;

	if (_numFrames == 0)
		return;

	if (_frames[frame]._ref != -1)
		frame = _frames[frame]._ref;

	int16 rectX = _frames[frame]._x2 - _frames[frame]._x1;
	int16 rectY = _frames[frame]._y2 - _frames[frame]._y1;

	if ((xx + _x1 + _frames[frame]._x1 < 0) || (yy + _y1 + _frames[frame]._y1 < 0))
		return;

	if (rectX + xx + _x1 + _frames[frame]._x1 >= surface.w)
		rectX = surface.w - xx - _x1 - _frames[frame]._x1;

	if (rectX < 0)
		return;

	if (rectY + yy + _y1 + _frames[frame]._y1 >= surface.h)
		rectY = surface.h - yy - _y1 - _frames[frame]._y1;

	if (rectY < 0)
		return;

	int32 destPitch = surface.pitch;
	uint8 *c = _frames[frame]._data;
	uint8 *curRow = (uint8 *)surface.pixels + (yy + _frames[frame]._y1 + _y1) * destPitch + (xx + _x1 + _frames[frame]._x1);
	for (int16 y = 0; y < rectY; y++) {
		unsigned char *cur = curRow;
		for (int16 x = 0; x < rectX; x++) {
			if (*c && *c < 4)
				*cur = colorMap[*c];
			c++;
			cur++;
		}
		curRow += destPitch;
	}
}

void Animation::drawFrameOnPicture(int32 frame, int16 xx, int16 yy) {
	debugC(1, kDebugAnim, "drawFrameOnPicture(%d, %d, %d)", frame, xx, yy);
	if (frame < 0)
		frame = 0;

	if (frame >= _numFrames)
		frame = _numFrames - 1;

	if (_numFrames == 0)
		return;

	if (_frames[frame]._ref != -1)
		frame = _frames[frame]._ref;

	int16 rectX = _frames[frame]._x2 - _frames[frame]._x1;
	int16 rectY = _frames[frame]._y2 - _frames[frame]._y1;

	Picture *pic = _vm->getPicture();

	if ((xx + _x1 + _frames[frame]._x1 < 0) || (yy + _y1 + _frames[frame]._y1 < 0))
		return;

	if (rectX + xx + _x1 + _frames[frame]._x1 >= pic->getWidth())
		rectX = pic->getWidth() - xx - _x1 - _frames[frame]._x1;

	if (rectX < 0)
		return;

	if (rectY + yy + _y1 + _frames[frame]._y1 >= pic->getHeight())
		rectY = pic->getHeight() - yy - _y1 - _frames[frame]._y1;

	if (rectY < 0)
		return;

	int32 destPitch = pic->getWidth();
	uint8 *c = _frames[frame]._data;
	uint8 *curRow = (uint8 *)pic->getDataPtr() + (yy + _frames[frame]._y1 + _y1) * destPitch + (xx + _x1 + _frames[frame]._x1);
	for (int16 y = 0; y < rectY; y++) {
		unsigned char *cur = curRow;
		for (int16 x = 0; x < rectX; x++) {
			if (*c)
				*cur = *c;
			c++;
			cur++;
		}
		curRow += destPitch;
	}
}

void AnimationInstance::update(int32 timeIncrement) {
	debugC(5, kDebugAnim, "update(%d)", timeIncrement);
	if (_currentFrame == -1)
		return;

	if (_rangeStart == _rangeEnd) {
		_currentFrame = _rangeStart;
		return;
	}

	if (_playing) {
		_currentTime += timeIncrement;
		_currentFrame = _currentTime / (1000 / _fps);
	}

	if (_looping) {
		_currentFrame = (_currentFrame % (_rangeEnd - _rangeStart + 1)) + _rangeStart;
	} else {
		if (_currentFrame >= _rangeEnd - _rangeStart) {
			_playing = false;
			_currentFrame = _rangeEnd;
		} else {
			_currentFrame = _rangeStart + _currentFrame;
		}
	}
}

AnimationInstance::AnimationInstance(ToonEngine *vm, AnimationInstanceType type) : _vm(vm) {
	_id = 0;
	_type = type;
	_animation = 0;
	_currentFrame = 0;
	_currentTime = 0;
	_fps = 15;
	_looping = true;
	_playing = false;
	_rangeEnd = 0;
	_useMask = false;
	_alignBottom = false;
	_rangeStart = 0;
	_scale = 1024;
	_x = 0;
	_y = 0;
	_z = 0;
	_layerZ = 0;
}

void AnimationInstance::render() {
	debugC(5, kDebugAnim, "render()");
	if (_visible && _animation) {
		int32 frame = _currentFrame;
		if (frame < 0)
			frame = 0;

		if (frame >= _animation->_numFrames)
			frame = _animation->_numFrames - 1;

		int16 x = _x;
		int16 y = _y;

		if (_alignBottom) {
			int32 offsetX = (_animation->_x2 - _animation->_x1) / 2 * (_scale - 1024);
			int32 offsetY = (_animation->_y2 - _animation->_y1) * (_scale - 1024);

			x -= offsetX >> 10;
			y -= offsetY >> 10;
		}

		if (_useMask) {
			//if (_scale == 100) { // 100% scale
			//	_animation->drawFrameWithMask(_vm->getMainSurface(), _currentFrame, _x, _y, _z, _vm->getMask());
			//} else {
			_animation->drawFrameWithMaskAndScale(_vm->getMainSurface(), frame, x, y, _z, _vm->getMask(), _scale);
			//}
		} else {
			_animation->drawFrame(_vm->getMainSurface(), frame, _x, _y);
		}
	}
}

void AnimationInstance::renderOnPicture() {
	debugC(5, kDebugAnim, "renderOnPicture()");
	if (_visible && _animation)
		_animation->drawFrameOnPicture(_currentFrame, _x, _y);
}

void AnimationInstance::playAnimation() {
	debugC(6, kDebugAnim, "playAnimation()");
	_playing = true;
}

void AnimationInstance::setAnimation(Animation *animation, bool setRange) {
	debugC(5, kDebugAnim, "setAnimation(animation)");
	_animation = animation;
	if (animation && setRange) {
		_rangeStart = 0;
		_rangeEnd = animation->_numFrames - 1;
	}
}

void AnimationInstance::setAnimationRange(int32 rangeStart, int32 rangeEnd) {
	debugC(5, kDebugAnim, "setAnimationRange(%d, %d)", rangeStart, rangeEnd);
	_rangeStart = rangeStart;
	_rangeEnd = rangeEnd;

	if (_currentFrame < _rangeStart)
		_currentFrame = _rangeStart;

	if (_currentFrame > _rangeEnd)
		_currentFrame = _rangeEnd;
}

void AnimationInstance::setPosition(int16 x, int16 y, int32 z, bool relative) {
	debugC(5, kDebugAnim, "setPosition(%d, %d, %d, %d)", x, y, z, (relative) ? 1 : 0);
	if (relative || !_animation) {
		_x = x;
		_y = y;
		_z = z;
	} else {
		_x = x - _animation->_x1;
		_y = y - _animation->_y1;
		_z = z;
	}
}

void AnimationInstance::moveRelative(int16 dx, int16 dy, int32 dz) {
	debugC(1, kDebugAnim, "moveRelative(%d, %d, %d)", dx, dy, dz);
	_x += dx;
	_y += dy;
	_z += dz;
}

void AnimationInstance::forceFrame(int32 position) {
	debugC(5, kDebugAnim, "forceFrame(%d)", position);
	_currentFrame = position;
	_rangeStart = position;
	_rangeEnd = position;
}

void AnimationInstance::setFrame(int32 position) {
	debugC(5, kDebugAnim, "setFrame(%d)", position);
	_currentFrame = position;
}

void AnimationInstance::setFps(int32 fps) {
	debugC(4, kDebugAnim, "setFps(%d)", fps);
	_fps = fps;
}

void AnimationInstance::stopAnimation() {
	debugC(5, kDebugAnim, "stopAnimation()");
	_playing = false;
}

void AnimationInstance::setVisible(bool visible) {
	debugC(1, kDebugAnim, "setVisible(%d)", (visible) ? 1 : 0);
	_visible = visible;
}

void AnimationInstance::setScale(int32 scale, bool align) {
	debugC(4, kDebugAnim, "setScale(%d)", scale);
	_scale = scale;
	_alignBottom = align;
}

void AnimationInstance::setUseMask(bool useMask) {
	debugC(1, kDebugAnim, "setUseMask(%d)", (useMask) ? 1 : 0);
	_useMask = useMask;
}

void AnimationInstance::getRect(int16 *x1, int16 *y1, int16 *x2, int16 *y2) const {
	debugC(5, kDebugAnim, "getRect(%d, %d, %d, %d)", *x1, *y1, *x2, *y2);
	int16 rectX = _animation->_frames[_currentFrame]._x2 - _animation->_frames[_currentFrame]._x1;
	int16 rectY = _animation->_frames[_currentFrame]._y2 - _animation->_frames[_currentFrame]._y1;

	int16 finalWidth = rectX * _scale / 1024;
	int16 finalHeight = rectY * _scale / 1024;

	// compute final x1, y1, x2, y2
	*x1 = _x + _animation->_x1 + _animation->_frames[_currentFrame]._x1 * _scale / 1024;
	*y1 = _y + _animation->_y1 + _animation->_frames[_currentFrame]._y1 * _scale / 1024;
	*x2 = *x1 + finalWidth;
	*y2 = *y1 + finalHeight;
}

void AnimationInstance::setX(int16 x, bool relative) {
	debugC(1, kDebugAnim, "setX(%d, %d)", x, (relative) ? 1 : 0);
	if (relative || !_animation)
		_x = x;
	else
		_x = x - _animation->_x1;
}

void AnimationInstance::setY(int16 y, bool relative) {
	debugC(1, kDebugAnim, "setY(%d, %d)", y, (relative) ? 1 : 0);
	if (relative || !_animation)
		_y = y;
	else
		_y = y - _animation->_y1;
}

void AnimationInstance::setZ(int32 z, bool relative) {
	debugC(1, kDebugAnim, "setZ(%d, %d)", z, (relative) ? 1 : 0);
	_z = z;
}

void AnimationInstance::setLayerZ(int32 z) {
	_layerZ = z;
	if (_vm->getAnimationManager()->hasInstance(this))
		_vm->getAnimationManager()->updateInstance(this);
}

int32 AnimationInstance::getLayerZ() const {
	return _layerZ;
}

int16 AnimationInstance::getX2() const {
	return _x + _animation->_x1;
}

int16 AnimationInstance::getY2() const {
	return _y + _animation->_y1;
}

int32 AnimationInstance::getZ2() const {
	return _z;
}

void AnimationInstance::save(Common::WriteStream *stream) {
	// we don't load the animation here
	// it must be loaded externally to avoid leaks.
	stream->writeSint32LE(_currentFrame);
	stream->writeSint32LE(_currentTime);
	stream->writeSint32LE(_layerZ);
	stream->writeSint32LE(_x);
	stream->writeSint32LE(_y);
	stream->writeSint32LE(_z);
	stream->writeSint32LE(_scale);
	stream->writeSint32LE(_playing);
	stream->writeSint32LE(_looping);
	stream->writeSint32LE(_rangeStart);
	stream->writeSint32LE(_rangeEnd);
	stream->writeSint32LE(_rangeStart);
	stream->writeSint32LE(_fps);
	stream->writeSint32LE(_id);
	stream->writeSint32LE(_type);
	stream->writeSint32LE(_visible);
	stream->writeSint32LE(_useMask);
}

void AnimationInstance::load(Common::ReadStream *stream) {
	_currentFrame = stream->readSint32LE();
	_currentTime = stream->readSint32LE();
	_layerZ = stream->readSint32LE();
	_x = stream->readSint32LE();
	_y = stream->readSint32LE();
	_z = stream->readSint32LE();
	_scale = stream->readSint32LE();
	_playing = stream->readSint32LE();
	_looping = stream->readSint32LE();
	_rangeStart = stream->readSint32LE();
	_rangeEnd = stream->readSint32LE();
	_rangeStart = stream->readSint32LE();
	_fps = stream->readSint32LE();
	_id = stream->readSint32LE();
	_type = (AnimationInstanceType)stream->readSint32LE();
	_visible = stream->readSint32LE();
	_useMask = stream->readSint32LE();
}

void AnimationInstance::setLooping(bool enable) {
	debugC(6, kDebugAnim, "setLooping(%d)", (enable) ? 1 : 0);
	_looping = enable;
}

void AnimationInstance::reset() {
	_currentFrame = 0;
	_currentTime = 0;
}

AnimationManager::AnimationManager(ToonEngine *vm) : _vm(vm) {
}

bool AnimationManager::hasInstance(AnimationInstance* instance) {
	for (uint32 i = 0; i < _instances.size(); i++) {
		if (_instances[i] == instance)
			return true;
	}
	return false;
}

void AnimationManager::updateInstance(AnimationInstance* instance) {
	// simply remove and readd the instance in the ordered list
	removeInstance(instance);
	addInstance(instance);
}

void AnimationManager::addInstance(AnimationInstance *instance) {
	// if the instance already exists, we skip the add
	for (uint32 i = 0; i < _instances.size(); i++) {
		if (_instances[i] == instance)
			return;
	}

	int32 found = -1;

	// here we now do an ordered insert (closer to the original game)
	for (uint32 i = 0; i < _instances.size(); i++) {
		if (_instances[i]->getLayerZ() >= instance->getLayerZ()) {
			found = i;
			break;
		}
	}

	if (found == -1)
		_instances.push_back(instance);
	else
		_instances.insert_at(found, instance);
}

void AnimationManager::removeInstance(AnimationInstance *instance) {
	debugC(1, kDebugAnim, "removeInstance(instance)");
	int32 found = -1;
	for (uint32 i = 0; i < _instances.size(); i++) {
		if (_instances[i] == instance) {
			found = i;
			break;
		}
	}

	if (found > -1)
		_instances.remove_at(found);
}

void AnimationManager::removeAllInstances(AnimationInstanceType type) {
	debugC(1, kDebugAnim, "removeInstance(type)");
	for (int32 i = (int32)_instances.size(); i >= 0; i--) {
		if (_instances[i]->getType() & type)
			_instances.remove_at(i);
	}
}

void AnimationManager::update(int32 timeIncrement) {
	debugC(5, kDebugAnim, "update(%d)", timeIncrement);
	for (uint32 i = 0; i < _instances.size(); i++)
		_instances[i]->update(timeIncrement);
}

void AnimationManager::render() {
	debugC(5, kDebugAnim, "render()");
	for (uint32 i = 0; i < _instances.size(); i++) {
		if (_instances[i]->getVisible())
			_instances[i]->render();
	}
}

AnimationInstance *AnimationManager::createNewInstance(AnimationInstanceType type) {
	return new AnimationInstance(_vm, type);
}

} // End of namespace Toon
