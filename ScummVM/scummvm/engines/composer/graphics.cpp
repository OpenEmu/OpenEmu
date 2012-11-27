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
#include "common/scummsys.h"

#include "graphics/palette.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "composer/composer.h"
#include "composer/graphics.h"
#include "composer/resource.h"

namespace Composer {

bool Sprite::contains(const Common::Point &pos) const {
	Common::Point adjustedPos = pos - _pos;

	if (adjustedPos.x < 0 || adjustedPos.x >= _surface.w)
		return false;
	if (adjustedPos.y < 0 || adjustedPos.y >= _surface.h)
		return false;
	byte *pixels = (byte *)_surface.pixels;
	return (pixels[(_surface.h - adjustedPos.y - 1) * _surface.w + adjustedPos.x] != 0);
}

enum {
	kAnimOpEvent = 1,
	kAnimOpPlayWave = 2,
	kAnimOpPlayAnim = 3,
	kAnimOpDrawSprite = 4
};

Animation::Animation(Common::SeekableReadStream *stream, uint16 id, Common::Point basePos, uint32 eventParam)
	: _stream(stream), _id(id), _basePos(basePos), _eventParam(eventParam) {
	uint32 size = _stream->readUint32LE();
	_state = _stream->readUint32LE() + 1;

	// probably total size?
	uint32 unknown = _stream->readUint32LE();

	debug(8, "anim: size %d, state %08x, unknown %08x", size, _state, unknown);

	for (uint i = 0; i < size; i++) {
		AnimationEntry entry;
		entry.op = _stream->readUint16LE();
		entry.priority = _stream->readUint16LE();
		entry.state = _stream->readUint16LE();
		entry.counter = 0;
		entry.prevValue = 0;
		debug(8, "anim entry: %04x, %04x, %04x", entry.op, entry.priority, entry.state);
		_entries.push_back(entry);
	}

	_offset = _stream->pos();
}

Animation::~Animation() {
	delete _stream;
}

void Animation::seekToCurrPos() {
	_stream->seek(_offset, SEEK_SET);
}

void ComposerEngine::playAnimation(uint16 animId, int16 x, int16 y, int16 eventParam) {
	// First, we check if this animation is already playing,
	// and if it is, we sabotage that running one first.
	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		Animation *anim = *i;
		if (anim->_id != animId)
			continue;

		stopAnimation(*i);
	}

	Common::SeekableReadStream *stream = NULL;
	Pipe *newPipe = NULL;

	// First, check the existing pipes.
	for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
		Pipe *pipe = *j;
		if (!pipe->hasResource(ID_ANIM, animId))
			continue;
		stream = pipe->getResource(ID_ANIM, animId, false);
		break;
	}

	// If we didn't find it, try the libraries.
	if (!stream) {
		if (!hasResource(ID_ANIM, animId)) {
			warning("ignoring attempt to play invalid anim %d", animId);
			return;
		}
		stream = getResource(ID_ANIM, animId);

		uint32 type = 0;
		for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
			if (i->_archive->hasResource(ID_ANIM, animId)) {
				type = i->_archive->getResourceFlags(ID_ANIM, animId);
				break;
			}

		// If the resource is a pipe itself, then load the pipe
		// and then fish the requested animation out of it.
		if (type != 1) {
			_pipeStreams.push_back(stream);
			newPipe = new Pipe(stream);
			_pipes.push_front(newPipe);
			newPipe->nextFrame();
			stream = newPipe->getResource(ID_ANIM, animId, false);
		}
	}

	Animation *anim = new Animation(stream, animId, Common::Point(x, y), eventParam);
	_anims.push_back(anim);
	runEvent(kEventAnimStarted, animId, eventParam, 0);
	if (newPipe)
		newPipe->_anim = anim;
}

void ComposerEngine::stopAnimation(Animation *anim, bool localOnly, bool pipesOnly) {
	// disable the animation
	anim->_state = 0;

	// stop any animations it may have spawned
	for (uint j = 0; j < anim->_entries.size(); j++) {
		AnimationEntry &entry = anim->_entries[j];
		if (!entry.prevValue)
			continue;
		if (localOnly) {
			if (pipesOnly)
				continue;
			if (entry.op == kAnimOpDrawSprite) {
				removeSprite(entry.prevValue, anim->_id);
			} else if (entry.op == kAnimOpPlayWave) {
				if (_currSoundPriority >= entry.priority) {
					_mixer->stopAll();
					_audioStream = NULL;
				}
			}
		} else {
			if (entry.op != kAnimOpPlayAnim)
				continue;
			for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
				if ((*i)->_id == entry.prevValue)
					stopAnimation(*i);
			}
		}
	}

	// kill any pipe owned by the animation
	for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
		Pipe *pipe = *j;
		if (pipe->_anim != anim)
			continue;
		j = _pipes.reverse_erase(j);
		delete pipe;
		break;
	}
}

void ComposerEngine::playWaveForAnim(uint16 id, uint16 priority, bool bufferingOnly) {
	if (_audioStream && _audioStream->numQueuedStreams() != 0) {
		if (_currSoundPriority < priority)
			return;
		if (_currSoundPriority > priority) {
			_mixer->stopAll();
			_audioStream = NULL;
		}
	}
	Common::SeekableReadStream *stream = NULL;
	bool fromPipe = true;
	if (!bufferingOnly && hasResource(ID_WAVE, id)) {
		stream = getResource(ID_WAVE, id);
		fromPipe = false;
	} else {
		for (Common::List<Pipe *>::iterator k = _pipes.begin(); k != _pipes.end(); k++) {
			Pipe *pipe = *k;
			if (!pipe->hasResource(ID_WAVE, id))
				continue;
			stream = pipe->getResource(ID_WAVE, id, true);
			break;
		}
	}
	if (!stream)
		return;

	uint32 size = stream->size();
	if (!fromPipe) {
		// non-pipe buffers have fixed wav header (data at +44, size at +40)
		stream->skip(40);
		size = stream->readUint32LE();
	}
	byte *buffer = (byte *)malloc(size);
	stream->read(buffer, size);
	if (!_audioStream)
		_audioStream = Audio::makeQueuingAudioStream(22050, false);
	_audioStream->queueBuffer(buffer, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	_currSoundPriority = priority;
	delete stream;
	if (!_mixer->isSoundHandleActive(_soundHandle))
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, _audioStream);
}

void ComposerEngine::processAnimFrame() {
	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		Animation *anim = *i;

		anim->seekToCurrPos();

		if (anim->_state <= 1) {
			bool normalEnd = (anim->_state == 1);
			if (normalEnd) {
				runEvent(kEventAnimDone, anim->_id, anim->_eventParam, 0);
			}
			stopAnimation(anim, true, normalEnd);
			delete anim;
			i = _anims.reverse_erase(i);

			continue;
		}

		for (uint j = 0; j < anim->_entries.size(); j++) {
			AnimationEntry &entry = anim->_entries[j];
			if (entry.op != kAnimOpEvent)
				break;
			if (entry.counter) {
				entry.counter--;
			} else {
				if ((anim->_state > 1) && (anim->_stream->pos() + 2 > anim->_stream->size())) {
					warning("anim with id %d ended too soon", anim->_id);
					anim->_state = 0;
					break;
				}

				uint16 event = anim->_stream->readUint16LE();
				anim->_offset += 2;
				if (event == 0xffff) {
					entry.counter = anim->_stream->readUint16LE() - 1;
					anim->_offset += 2;
				} else {
					debug(4, "anim: event %d", event);
					runEvent(event, anim->_id, 0, 0);
				}
			}
		}
	}

	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
		Animation *anim = *i;

		// did the anim get disabled?
		if (anim->_state == 0) {
			stopAnimation(anim, true, false);
			delete anim;
			i = _anims.reverse_erase(i);
			continue;
		}

		anim->_state--;

		bool foundWait = false;
		for (uint j = 0; j < anim->_entries.size(); j++) {
			AnimationEntry &entry = anim->_entries[j];

			// only skip these at the start
			if (!foundWait && (entry.op == kAnimOpEvent))
				continue;
			foundWait = true;

			if (entry.counter) {
				entry.counter--;
				if ((entry.op == kAnimOpPlayWave) && entry.prevValue) {
					debug(4, "anim: continue play wave %d", entry.prevValue);
					playWaveForAnim(entry.prevValue, entry.priority, true);
				}
			} else {
				anim->seekToCurrPos();
				if ((anim->_state > 1) && (anim->_stream->pos() + 2 > anim->_stream->size())) {
					warning("anim with id %d ended too soon", anim->_id);
					anim->_state = 0;
					break;
				}

				uint16 data = anim->_stream->readUint16LE();
				anim->_offset += 2;
				if (data == 0xffff) {
					entry.counter = anim->_stream->readUint16LE() - 1;
					anim->_offset += 2;
				} else {
					switch (entry.op) {
					case kAnimOpEvent:
						debug(4, "anim: event %d", data);
						runEvent(data, anim->_id, 0, 0);
						break;
					case kAnimOpPlayWave:
						debug(4, "anim: play wave %d", data);
						playWaveForAnim(data, entry.priority, false);
						break;
					case kAnimOpPlayAnim:
						debug(4, "anim: play anim %d", data);
						playAnimation(data, anim->_basePos.x, anim->_basePos.y, 1);
						break;
					case kAnimOpDrawSprite:
						if (!data || (entry.prevValue && (data != entry.prevValue))) {
							debug(4, "anim: erase sprite %d", entry.prevValue);
							removeSprite(entry.prevValue, anim->_id);
						}
						if (data) {
							int16 x = anim->_stream->readSint16LE();
							int16 y = anim->_stream->readSint16LE();
							Common::Point pos(x, y);
							anim->_offset += 4;
							uint16 animId = anim->_id;
							if (anim->_state == entry.state)
								animId = 0;
							debug(4, "anim: draw sprite %d at (relative) %d,%d", data, x, y);
							bool wasVisible = spriteVisible(data, animId);
							addSprite(data, animId, entry.priority, anim->_basePos + pos);
							if (wasVisible) {
								// make sure modified sprite isn't removed by another entry
								for (uint k = 0; k < anim->_entries.size(); k++) {
									if (anim->_entries[k].op != kAnimOpDrawSprite)
										continue;
									if (anim->_entries[k].prevValue == data)
										anim->_entries[k].prevValue = 1;
								}
							}
						}
						break;
					default:
						warning("unknown anim op %d", entry.op);
					}

					entry.prevValue = data;
				}
			}
		}
	}

	for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
		Pipe *pipe = *j;
		pipe->nextFrame();

		// V1 pipe audio; see OldPipe
		if (pipe->hasResource(ID_WAVE, 0xffff))
			playWaveForAnim(0xffff, 0, false);
	}
}

void ComposerEngine::playPipe(uint16 id) {
	stopPipes();

	if (!hasResource(ID_PIPE, id)) {
		error("couldn't find pipe %d", id);
	}

	Common::SeekableReadStream *stream = getResource(ID_PIPE, id);
	OldPipe *pipe = new OldPipe(stream);
	_pipes.push_front(pipe);
	//pipe->nextFrame();

	const Common::Array<uint16> *scripts = pipe->getScripts();
	if (scripts && !scripts->empty())
		runScript((*scripts)[0], 1, 0, 0);
}

void ComposerEngine::stopPipes() {
	for (Common::List<Pipe *>::iterator j = _pipes.begin(); j != _pipes.end(); j++) {
		const Common::Array<uint16> *scripts = (*j)->getScripts();
		if (scripts) {
			for (uint i = 0; i < scripts->size(); i++) {
				removeSprite((*scripts)[i], 0);
				stopOldScript((*scripts)[i]);
			}
		}
		delete *j;
	}

	_pipes.clear();

	// substreams may need to remain valid until the end of a page
	for (uint i = 0; i < _pipeStreams.size(); i++)
		delete _pipeStreams[i];
	_pipeStreams.clear();
}

bool ComposerEngine::spriteVisible(uint16 id, uint16 animId) {
	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (i->_id != id)
			continue;
		if (i->_animId && animId && (i->_animId != animId))
			continue;
		return true;
	}

	return false;
}

Sprite *ComposerEngine::addSprite(uint16 id, uint16 animId, uint16 zorder, const Common::Point &pos) {
	Sprite sprite;
	bool foundSprite = false;

	// re-use old sprite, if any (the BMAP for this id might well have
	// changed in the meantime, but the scripts depend on that!)
	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (i->_id != id)
			continue;
		if (getGameType() == GType_ComposerV1) {
			if (i->_animId != animId)
				continue;
		} else if (i->_animId && animId && (i->_animId != animId))
			continue;

		dirtySprite(*i);

		// if the zordering is identical, modify it in-place
		if (i->_zorder == zorder) {
			i->_animId = animId;
			i->_pos = pos;
			dirtySprite(*i);
			return &(*i);
		}

		// otherwise, take a copy and remove it from the list
		sprite = *i;
		foundSprite = true;
		_sprites.erase(i);
		break;
	}

	sprite._animId = animId;
	sprite._zorder = zorder;
	sprite._pos = pos;

	if (!foundSprite) {
		sprite._id = id;
		if (!initSprite(sprite)) {
			debug(1, "ignoring addSprite on invalid sprite %d", id);
			return NULL;
		}
	}

	dirtySprite(sprite);

	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (sprite._zorder <= i->_zorder)
			continue;
		// insert *before* this sprite
		_sprites.insert(i, sprite);
		--i;
		return &(*i);
	}
	_sprites.push_back(sprite);
	return &_sprites.back();
}

void ComposerEngine::removeSprite(uint16 id, uint16 animId) {
	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (!i->_id || (id && i->_id != id))
			continue;
		if (getGameType() == GType_ComposerV1) {
			if (i->_animId != animId)
				continue;
		} else if (i->_animId && animId && (i->_animId != animId))
			continue;
		dirtySprite(*i);
		i->_surface.free();
		i = _sprites.reverse_erase(i);
		if (id)
			break;
	}
}

const Sprite *ComposerEngine::getSpriteAtPos(const Common::Point &pos) {
	for (Common::List<Sprite>::iterator i = _sprites.reverse_begin(); i != _sprites.end(); --i) {
		// avoid highest-level objects (e.g. the cursor)
		if (!i->_zorder)
			continue;

		if (i->contains(pos))
			return &(*i);
	}

	return NULL;
}

void ComposerEngine::dirtySprite(const Sprite &sprite) {
	Common::Rect rect(sprite._pos.x, sprite._pos.y, sprite._pos.x + sprite._surface.w, sprite._pos.y + sprite._surface.h);
	rect.clip(_screen.w, _screen.h);
	if (rect.isEmpty())
		return;

	for (uint i = 0; i < _dirtyRects.size(); i++) {
		if (!_dirtyRects[i].intersects(rect))
			continue;
		_dirtyRects[i].extend(rect);
		return;
	}

	_dirtyRects.push_back(rect);
}

void ComposerEngine::redraw() {
	if (!_needsUpdate && _dirtyRects.empty())
		return;

	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		Common::Rect rect(i->_pos.x, i->_pos.y, i->_pos.x + i->_surface.w, i->_pos.y + i->_surface.h);
		bool intersects = false;
		for (uint j = 0; j < _dirtyRects.size(); j++) {
			if (!_dirtyRects[j].intersects(rect))
				continue;
			intersects = true;
			break;
		}
		if (!intersects)
			continue;
		drawSprite(*i);
	}

	for (uint i = 0; i < _dirtyRects.size(); i++) {
		const Common::Rect &rect = _dirtyRects[i];
		byte *pixels = (byte *)_screen.pixels + (rect.top * _screen.pitch) + rect.left;
		_system->copyRectToScreen(pixels, _screen.pitch, rect.left, rect.top, rect.width(), rect.height());
	}
	_system->updateScreen();

	_needsUpdate = false;
	_dirtyRects.clear();
}

void ComposerEngine::loadCTBL(uint16 id, uint fadePercent) {
	Common::SeekableReadStream *stream = getResource(ID_CTBL, id);

	uint16 numEntries = stream->readUint16LE();
	debug(1, "CTBL: %d entries", numEntries);

	if ((numEntries > 256) || (stream->size() < 2 + (numEntries * 3)))
		error("CTBL %d was invalid (%d entries, size %d)", id, numEntries, stream->size());

	byte buffer[256 * 3];
	stream->read(buffer, numEntries * 3);
	delete stream;

	for (uint16 i = 0; i < numEntries * 3; i++)
		buffer[i] = ((unsigned int)buffer[i] * fadePercent) / 100;

	_system->getPaletteManager()->setPalette(buffer, 0, numEntries);
	_needsUpdate = true;
}

void ComposerEngine::setBackground(uint16 id) {
	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++) {
		if (i->_id)
			continue;
		dirtySprite(*i);
		i->_surface.free();
		i->_id = id;
		if (!initSprite(*i))
			error("failed to set background %d", id);
		dirtySprite(*i);
		i->_id = 0;
		return;
	}

	Sprite *background = addSprite(id, 0, 0xffff, Common::Point());
	if (background)
		background->_id = 0;
}

static void decompressSLWM(byte *buffer, Common::SeekableReadStream *stream) {
	uint bitsLeft = 0;
	uint16 lastBits = 0;
	byte currBit;
	while (true) {
		if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
		currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

		if (currBit) {
			// single byte
			*buffer++ = stream->readByte();
			continue;
		}

		if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
		currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

		uint start;
		uint count;
		if (currBit) {
			uint orMask = stream->readByte();
			uint in = stream->readByte();
			count = in & 7;
			start = ((in & ~7) << 5) | orMask;
			if (!count) {
				count = stream->readByte();
				if (!count)
					break;
				count -= 2;
			}
		} else {
			// count encoded in the next two bits
			count = 0;

			if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
			currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

			count = (count << 1) | currBit;

			if (bitsLeft == 0) { bitsLeft = 16; lastBits = stream->readUint16LE(); }
			currBit = (lastBits & 1); lastBits >>= 1; bitsLeft--;

			count = (count << 1) | currBit;

			start = stream->readByte();
		}

		count += 2;
		start++;
		for (uint i = 0; i < count; i++) {
			*buffer = *(buffer - start);
			buffer++;
		}
	}
}

// bitmap compression types
enum {
	kBitmapUncompressed = 0,
	kBitmapSpp32 = 1,
	kBitmapSLW8 = 3,
	kBitmapRLESLWM = 4,
	kBitmapSLWM = 5
};

void ComposerEngine::decompressBitmap(uint16 type, Common::SeekableReadStream *stream, byte *buffer, uint32 size, uint width, uint height) {
	uint outSize = width * height;

	switch (type) {
	case kBitmapUncompressed:
		if (stream->size() - (uint)stream->pos() != size)
			error("kBitmapUncompressed stream had %d bytes left, supposed to be %d",
				stream->size() - (uint)stream->pos(), size);
		if (size != outSize)
			error("kBitmapUncompressed size %d doesn't match required size %d",
				size, outSize);
		stream->read(buffer, size);
		break;
	case kBitmapSpp32:
		byte lookup[16];
		stream->read(lookup, 16);
		while (size--) {
			uint in = stream->readByte();
			byte lowBits = in & 0xF;
			byte highBits = (in & 0xF0) >> 4;
			if (highBits == 0xf) {
				// run of a single color
				uint count = (uint)stream->readByte() + 3;
				size--;
				if (outSize < count)
					error("kBitmapSpp32 only needed %d bytes, but got run of %d",
						outSize, count);
				outSize -= count;
				memset(buffer, lookup[lowBits], count);
				buffer += count;
			} else {
				// two pixels
				if (!outSize)
					error("kBitmapSpp32 had too many pixels");
				*buffer++ = lookup[highBits];
				outSize--;
				if (outSize) {
					*buffer++ = lookup[lowBits];
					outSize--;
				}
			}
		}
		break;
	case kBitmapSLW8:
		while (size--) {
			byte val = stream->readByte();
			if (val != 0xff) {
				*buffer++ = val;
				continue;
			}
			uint count = stream->readByte();
			size--;

			uint16 step;
			if (!(count & 0x80)) {
				step = stream->readByte();
				size--;
			} else {
				count = (count ^ 0x80);
				step = stream->readUint16LE();
				size -= 2;
			}
			count += 4;
			// this is often overlapping (for repeating patterns)
			for (uint i = 0; i < count; i++) {
				*buffer = *(buffer - step  - 1);
				buffer++;
			}
		}
		break;
	case kBitmapRLESLWM:
		{
		uint32 bufSize = stream->readUint32LE();
		byte *tempBuf = new byte[bufSize];
		decompressSLWM(tempBuf, stream);

		uint instrPos = tempBuf[0] + 1;
		instrPos += READ_LE_UINT16(tempBuf + instrPos) + 2;
		byte *instr = tempBuf + instrPos;

		uint line = 0;
		while (line++ < height) {
			uint pixels = 0;

			while (pixels < width) {
				byte data = *instr++;
				byte color = tempBuf[(data & 0x7F) + 1];
				if (!(data & 0x80)) {
					*buffer++ = color;
					pixels++;
				} else {
					byte count = *instr++;
					if (!count) {
						while (pixels++ < width)
							*buffer++ = color;
						break;
					}
					for (uint i = 0; i < count; i++) {
						*buffer++ = color;
						pixels++;
					}
				}
			}
		}
		delete[] tempBuf;
		}
		break;
	case kBitmapSLWM:
		decompressSLWM(buffer, stream);
		break;
	default:
		error("decompressBitmap can't handle type %d", type);
	}
}

Common::SeekableReadStream *ComposerEngine::getStreamForSprite(uint16 id) {
	for (Common::List<Pipe *>::iterator k = _pipes.begin(); k != _pipes.end(); k++) {
		Pipe *pipe = *k;
		if (!pipe->hasResource(ID_BMAP, id))
			continue;
		return pipe->getResource(ID_BMAP, id, true);
	}
	if (hasResource(ID_BMAP, id))
		return getResource(ID_BMAP, id);
	return NULL;
}

bool ComposerEngine::initSprite(Sprite &sprite) {
	Common::SeekableReadStream *stream = getStreamForSprite(sprite._id);
	if (!stream)
		return false;

	uint16 type = stream->readUint16LE();
	int16 height = stream->readSint16LE();
	int16 width = stream->readSint16LE();
	uint32 size = stream->readUint32LE();
	debug(1, "loading BMAP: type %d, width %d, height %d, size %d", type, width, height, size);

	if (width > 0 && height > 0) {
		sprite._surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		decompressBitmap(type, stream, (byte *)sprite._surface.pixels, size, width, height);
	} else {
		// there are some sprites (e.g. a -998x-998 one in Gregory's title screen)
		// which have an invalid size, but the original engine doesn't notice for
		// RLE sprites since the width/height is ignored until the actual draw
		if (type != kBitmapRLESLWM)
			error("sprite (type %d) had invalid size %dx%d", type, width, height);
		delete stream;
		return false;
	}
	delete stream;

	return true;
}

void ComposerEngine::drawSprite(const Sprite &sprite) {
	int x = sprite._pos.x;
	int y = sprite._pos.y;

	// incoming data is BMP-style (bottom-up), so flip it
	byte *pixels = (byte *)_screen.pixels;
	for (int j = 0; j < sprite._surface.h; j++) {
		if (j + y < 0)
			continue;
		if (j + y >= _screen.h)
			break;
		byte *in = (byte *)sprite._surface.pixels + (sprite._surface.h - j - 1) * sprite._surface.w;
		byte *out = pixels + ((j + y) * _screen.w) + x;
		for (int i = 0; i < sprite._surface.w; i++)
			if ((x + i >= 0) && (x + i < _screen.w) && in[i])
				out[i] = in[i];
	}
}

} // End of namespace Composer
