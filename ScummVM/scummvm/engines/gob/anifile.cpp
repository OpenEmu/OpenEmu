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

#include "common/stream.h"
#include "common/substream.h"

#include "gob/gob.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/surface.h"
#include "gob/video.h"
#include "gob/cmpfile.h"
#include "gob/anifile.h"

namespace Gob {

ANIFile::ANIFile(GobEngine *vm, const Common::String &fileName,
                 uint16 width, uint8 bpp) : _vm(vm),
	_width(width), _bpp(bpp), _hasPadding(false) {

	bool bigEndian = false;
	Common::String endianFileName = fileName;

	if ((_vm->getEndiannessMethod() == kEndiannessMethodAltFile) &&
	    !_vm->_dataIO->hasFile(fileName)) {
		// If the game has alternate big-endian files, look if one exist

		Common::String alternateFileName = fileName;
		alternateFileName.setChar('_', 0);

		if (_vm->_dataIO->hasFile(alternateFileName)) {
			bigEndian      = true;
			endianFileName = alternateFileName;
		}
	} else if ((_vm->getEndiannessMethod() == kEndiannessMethodBE) ||
	           ((_vm->getEndiannessMethod() == kEndiannessMethodSystem) &&
	            (_vm->getEndianness() == kEndiannessBE)))
		// Game always little endian or it follows the system and it is big endian
		bigEndian = true;

	Common::SeekableReadStream *ani = _vm->_dataIO->getFile(endianFileName);
	if (ani) {
		Common::SeekableSubReadStreamEndian sub(ani, 0, ani->size(), bigEndian, DisposeAfterUse::YES);

		// The big endian version pads a few fields to even size
		_hasPadding = bigEndian;

		load(sub, fileName);
		return;
	}

	warning("ANIFile::ANIFile(): No such file \"%s\" (\"%s\")", endianFileName.c_str(), fileName.c_str());
}

ANIFile::~ANIFile() {
	for (LayerArray::iterator l = _layers.begin(); l != _layers.end(); ++l)
		delete *l;
}

void ANIFile::load(Common::SeekableSubReadStreamEndian &ani, const Common::String &fileName) {
	ani.skip(2); // Unused

	uint16 animationCount = ani.readUint16();
	uint16 layerCount     = ani.readUint16();

	if (layerCount < 1)
		warning("ANIFile::load(): Less than one layer (%d) in file \"%s\"",
		        layerCount, fileName.c_str());

		// Load the layers
	if (layerCount > 0) {
		ani.skip(13); // The first layer is ignored?
		if (_hasPadding)
			ani.skip(1);

		_layers.reserve(layerCount - 1);
		for (int i = 0; i < layerCount - 1; i++)
			_layers.push_back(loadLayer(ani));
	}

	_maxWidth  = 0;
	_maxHeight = 0;

	// Load the animations
	_animations.resize(animationCount);
	_frames.resize(animationCount);

	for (uint16 animation = 0; animation < animationCount; animation++) {
		loadAnimation(_animations[animation], _frames[animation], ani);

		_maxWidth  = MAX<uint16>(_maxWidth , _animations[animation].width);
		_maxHeight = MAX<uint16>(_maxHeight, _animations[animation].height);
	}
}

void ANIFile::loadAnimation(Animation &animation, FrameArray &frames,
                            Common::SeekableSubReadStreamEndian &ani) {

	// Animation properties

	animation.name = Util::readString(ani, 13);
	if (_hasPadding)
		ani.skip(1);

	ani.skip(13); // The name a second time?!?
	if (_hasPadding)
		ani.skip(1);

	ani.skip(2);  // Unknown

	animation.x      = (int16) ani.readUint16();
	animation.y      = (int16) ani.readUint16();
	animation.deltaX = (int16) ani.readUint16();
	animation.deltaY = (int16) ani.readUint16();

	animation.transp = ani.readByte() != 0;

	if (_hasPadding)
		ani.skip(1);

	uint16 frameCount = ani.readUint16();

	// Load the frames

	frames.resize(MAX<uint16>(1, frameCount));
	loadFrames(frames, ani);

	animation.frameCount = frames.size();

	animation.width  = 0;
	animation.height = 0;

	// Calculate the areas of each frame

	animation.frameAreas.resize(animation.frameCount);
	for (uint16 i = 0; i < animation.frameCount; i++) {
		const ChunkList &frame = frames[i];
		FrameArea &area = animation.frameAreas[i];

		area.left  = area.top    =  0x7FFF;
		area.right = area.bottom = -0x7FFF;

		for (ChunkList::const_iterator c = frame.begin(); c != frame.end(); c++) {
			uint16 cL, cT, cR, cB;

			if (!getCoordinates(c->layer, c->part, cL, cT, cR, cB))
				continue;

			const uint16 width  = cR - cL + 1;
			const uint16 height = cB - cT + 1;

			const uint16 l = c->x;
			const uint16 t = c->y;
			const uint16 r = l + width  - 1;
			const uint16 b = t + height - 1;

			area.left   = MIN<int16>(area.left  , l);
			area.top    = MIN<int16>(area.top   , t);
			area.right  = MAX<int16>(area.right , r);
			area.bottom = MAX<int16>(area.bottom, b);
		}

		if ((area.left <= area.right) && (area.top <= area.bottom)) {
			animation.width  = MAX<uint16>(animation.width , area.right  - area.left + 1);
			animation.height = MAX<uint16>(animation.height, area.bottom - area.top  + 1);
		}
	}
}

void ANIFile::loadFrames(FrameArray &frames, Common::SeekableSubReadStreamEndian &ani) {
	uint32 curFrame = 0;

	bool end = false;
	while (!end) {
		frames[curFrame].push_back(AnimationChunk());
		AnimationChunk &chunk = frames[curFrame].back();

		uint8 layerFlags = ani.readByte();

		// Chunk properties
		chunk.layer = (layerFlags & 0x0F) - 1;
		chunk.part  = ani.readByte();
		chunk.x     = (int8) ani.readByte();
		chunk.y     = (int8) ani.readByte();

		// X multiplier/offset
		int16 xOff = ((layerFlags & 0xC0) >> 6) << 7;
		if (chunk.x >= 0)
			chunk.x += xOff;
		else
			chunk.x -= xOff;

		// Y multiplier/offset
		int16 yOff = ((layerFlags & 0x30) >> 4) << 7;
		if (chunk.y >= 0)
			chunk.y += yOff;
		else
			chunk.y -= yOff;

		uint8 multiPart = ani.readByte();
		if      (multiPart == 0xFF) // No more frames in this animation
			end = true;
		else if (multiPart != 0x01) // No more chunks in this frame
			curFrame++;

		// Shouldn't happen, but just to be safe
		if (curFrame >= frames.size())
			frames.resize(curFrame + 1);

		if (_hasPadding)
			ani.skip(1);

		if (ani.eos() || ani.err())
			error("ANIFile::loadFrames(): Read error");
	}
}

CMPFile *ANIFile::loadLayer(Common::SeekableSubReadStreamEndian &ani) {
	Common::String file = Util::setExtension(Util::readString(ani, 13), "");
	if (_hasPadding)
		ani.skip(1);

	return new CMPFile(_vm, file, _width, 0, _bpp);
}

uint16 ANIFile::getAnimationCount() const {
	return _animations.size();
}

void ANIFile::getMaxSize(uint16 &width, uint16 &height) const {
	width  = _maxWidth;
	height = _maxHeight;
}

const ANIFile::Animation &ANIFile::getAnimationInfo(uint16 animation) const {
	assert(animation < _animations.size());

	return _animations[animation];
}

bool ANIFile::getCoordinates(uint16 layer, uint16 part,
                             uint16 &left, uint16 &top, uint16 &right, uint16 &bottom) const {

	if (layer >= _layers.size())
		return false;

	return _layers[layer]->getCoordinates(part, left, top, right, bottom);
}

void ANIFile::draw(Surface &dest, uint16 animation, uint16 frame, int16 x, int16 y) const {
	if (animation >= _animations.size())
		return;

	const Animation &anim = _animations[animation];
	if (frame >= anim.frameCount)
		return;

	const ChunkList &chunks = _frames[animation][frame];

	for (ChunkList::const_iterator c = chunks.begin(); c != chunks.end(); ++c)
		drawLayer(dest, c->layer, c->part, x + c->x, y + c->y, anim.transp ? 0 : -1);
}

void ANIFile::drawLayer(Surface &dest, uint16 layer, uint16 part,
                        int16 x, int16 y, int32 transp) const {

	if (layer >= _layers.size())
		return;

	_layers[layer]->draw(dest, part, x, y, transp);
}

void ANIFile::recolor(uint8 from, uint8 to) {
	for (LayerArray::iterator l = _layers.begin(); l != _layers.end(); ++l)
		(*l)->recolor(from, to);
}

} // End of namespace Gob
