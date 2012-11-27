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

/**
 * @file
 * Sound decoder used in engines:
 *  - gob
 */

#ifndef AUDIO_MODS_INFOGRAMES_H
#define AUDIO_MODS_INFOGRAMES_H

#include "audio/mods/paula.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

/** A player for the Infogrames/RobHubbard2 format */
class Infogrames : public Paula {
public:
	class Instruments {
	public:
		Instruments();
		template<typename T> Instruments(T ins) {
			init();
			bool result = load(ins);
			assert(result);
		}
		~Instruments();

		bool load(Common::SeekableReadStream &ins);
		bool load(const char *ins);
		void unload();

		uint8 getCount() const { return _count; }

	protected:
		struct Sample {
			int8 *data;
			int8 *dataRepeat;
			uint32 length;
			uint32 lengthRepeat;
		} _samples[32];

		uint8 _count;
		int8 *_sampleData;

		void init();

		friend class Infogrames;
	};

	Infogrames(Instruments &ins, bool stereo = false, int rate = 44100,
			int interruptFreq = 0);
	~Infogrames();

	Instruments *getInstruments() const { return _instruments; }
	bool getRepeating() const { return _repCount != 0; }
	void setRepeating (int32 repCount) { _repCount = repCount; }

	bool load(Common::SeekableReadStream &dum);
	bool load(const char *dum);
	void unload();
	void restart() {
		if (_data) {
			// Use the mutex here to ensure we do not call init()
			// while data is being read by the mixer thread.
			_mutex.lock();
			init();
			startPlay();
			_mutex.unlock();
		}
	}

protected:
	Instruments *_instruments;

	static const uint8 tickCount[];
	static const uint16 periods[];
	byte *_data;
	int32 _repCount;

	byte *_subSong;
	byte *_cmdBlocks;
	byte *_volSlideBlocks;
	byte *_periodSlideBlocks;
	uint8 _speedCounter;
	uint8 _speed;

	uint16 _volume;
	int16 _period;
	uint8 _sample;

	struct Slide {
		byte *data;
		int8 amount;
		uint8 dataOffset;
		int16 finetuneNeg;
		int16 finetunePos;
		uint8 curDelay1;
		uint8 curDelay2;
		uint8 flags; // 0: Apply finetune modifier, 2: Don't slide, 7: Continuous
	};
	struct Channel {
		byte *cmdBlockIndices;
		byte *cmdBlocks;
		byte *cmds;
		uint8 ticks;
		uint8 tickCount;
		Slide volSlide;
		Slide periodSlide;
		int16 period;
		int8 periodMod;
		uint8 flags; // 0: Need init, 5: Loop cmdBlocks, 6: Ignore channel
	} _chn[4];

	void init();
	void reset();
	void getNextSample(Channel &chn);
	int16 tune(Slide &slide, int16 start) const;
	virtual void interrupt();
};

} // End of namespace Audio

#endif
