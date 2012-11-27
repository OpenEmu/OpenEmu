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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "teenagent/teenagent.h"
#include "teenagent/animation.h"

#include "common/endian.h"
#include "common/textconsole.h"

namespace TeenAgent {

Animation::Animation() : id(0), x(0), y(0), loop(true), paused(false), ignore(false), data(0), dataSize(0), framesCount(0), frames(0), index(0) {
}

Animation::~Animation() {
	free();
}

Surface *Animation::firstFrame() {
	if (frames == NULL || framesCount == 0)
		return NULL;

	Surface *r = frames;
	uint16 pos = READ_LE_UINT16(data + 1);
	if (pos != 0) {
		r->x = pos % kScreenWidth;
		r->y = pos / kScreenWidth;
	}
	return r;
}

Surface *Animation::currentFrame(int dt) {
	if (paused)
		return firstFrame();

	if (frames == NULL || framesCount == 0)
		return NULL;

	Surface *r;

	if (data != NULL) {
		uint32 frame = 3 * index;
		debugC(2, kDebugAnimation, "%u/%u", index, dataSize / 3);
		index += dt;

		if (!loop && index >= dataSize / 3) {
			return NULL;
		}

		if (data[frame] - 1 >= framesCount) {
			warning("invalid frame %u(0x%x) (max %u) index %u, mod %u", frame, frame, framesCount, index - 1, dataSize / 3);
			return NULL;
		}

		r = frames + data[frame] - 1;
		uint16 pos = READ_LE_UINT16(data + frame + 1);
		index %= (dataSize / 3);

		if (pos != 0) {
			x = r->x = pos % kScreenWidth;
			y = r->y = pos / kScreenWidth;
		}
	} else {
		debugC(2, kDebugAnimation, "index %u", index);
		r = frames + index;
		index += dt;
		index %= framesCount;
	}

	return r;
}

void Animation::restart() {
	paused = false;
	ignore = false;
	index = 0;
}

void Animation::free() {
	id = 0;
	x = y = 0;
	loop = true;
	paused = false;
	ignore = false;

	delete[] data;
	data = NULL;
	dataSize = 0;

	framesCount = 0;
	delete[] frames;
	frames = NULL;

	index = 0;
}

void Animation::load(Common::SeekableReadStream &s, Type type) {
	//FIXME: do not reload the same animation each time
	free();

	if (s.size() <= 1) {
		debugC(1, kDebugAnimation, "empty animation");
		return;
	}

	uint16 pos = 0;
	int off = 0;
	switch (type) {
	case kTypeLan:
		dataSize = s.readUint16LE();
		if (s.eos()) {
			debugC(1, kDebugAnimation, "empty animation");
			return;
		}

		dataSize -= 2;
		data = new byte[dataSize];
		dataSize = s.read(data, dataSize);
		for (int i = 0; i < dataSize; ++i)
			debugC(2, kDebugAnimation, "%02x ", data[i]);
		debugC(2, kDebugAnimation, ", %u frames", dataSize / 3);
		framesCount = s.readByte();
		debugC(1, kDebugAnimation, "%u physical frames", framesCount);
		if (framesCount == 0)
			return;

		frames = new Surface[framesCount];

		s.skip(framesCount * 2 - 2); //sizes
		pos = s.readUint16LE();
		debugC(3, kDebugAnimation, "pos?: 0x%04x", pos);

		for (uint16 i = 0; i < framesCount; ++i) {
			frames[i].load(s, Surface::kTypeLan);
			frames[i].x = 0;
			frames[i].y = 0;
		}
		break;

	case kTypeInventory: {
		dataSize = 3 * s.readByte();
		data = new byte[dataSize];

		framesCount = 0;
		for (byte i = 0; i < dataSize / 3; ++i) {
			int idx = i * 3;
			byte unk = s.readByte();
			debugC(3, kDebugAnimation, "unk?: 0x%02x", unk);
			data[idx] = s.readByte();
			if (data[idx] == 0)
				data[idx] = 1; //fixme: investigate
			if (data[idx] > framesCount)
				framesCount = data[idx];
			data[idx + 1] = 0;
			data[idx + 2] = 0;
			debugC(2, kDebugAnimation, "frame #%u", data[idx]);
		}

		frames = new Surface[framesCount];

		for (uint16 i = 0; i < framesCount; ++i) {
			frames[i].load(s, Surface::kTypeOns);
		}
	}
	break;

	case kTypeVaria:
		framesCount = s.readByte();
		debugC(1, kDebugAnimation, "loading varia resource, %u physical frames", framesCount);
		uint16 offset[255];
		for (byte i = 0; i < framesCount; ++i) {
			offset[i] = s.readUint16LE();
			debugC(0, kDebugAnimation, "%u: %04x", i, offset[i]);
		}
		frames = new Surface[framesCount];
		for (uint16 i = 0; i < framesCount; ++i) {
			debugC(0, kDebugAnimation, "%04x", offset[i]);
			s.seek(offset[i] + off);
			frames[i].load(s, Surface::kTypeOns);
		}

		break;
	}

	debugC(2, kDebugAnimation, "%u frames", dataSize / 3);
}

} // End of namespace TeenAgent
