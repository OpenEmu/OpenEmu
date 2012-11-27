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

#ifndef AUDIO_MODS_MODULE_H
#define AUDIO_MODS_MODULE_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Modules {

#include "common/pack-start.h"	// START STRUCT PACKING

struct note_t {
	byte sample;
	byte note;
	uint16 period;
	uint16 effect;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

typedef note_t pattern_t[64][4];

struct sample_t {
	byte name[23];
	uint16 len;
	byte finetune;
	byte vol;
	uint16 repeat;
	uint16 replen;
	int8 *data;
};

struct sample_offs {
	byte name[23];
	uint16 len;
	uint32 offs;
};

class Module {
public:
	byte songname[21];

	static const int NUM_SAMPLES = 31;
	sample_t sample[NUM_SAMPLES];
	sample_offs commonSamples[NUM_SAMPLES];

	byte songlen;
	byte undef;
	byte songpos[128];
	uint32 sig;
	pattern_t *pattern;

	Module();
	~Module();

	bool load(Common::SeekableReadStream &stream, int offs);
	static byte periodToNote(int16 period, byte finetune = 0);
	static int16 noteToPeriod(byte note, byte finetune = 0);

private:
	static const int16 periods[16][60];
	static const uint32 signatures[];
};

} // End of namespace Modules

#endif
