/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "channel3.h"
#include "../savestate.h"
#include <cstring>
#include <algorithm>

static inline unsigned toPeriod(const unsigned nr3, const unsigned nr4) {
	return 0x800 - ((nr4 << 8 & 0x700) | nr3);
}

Channel3::Channel3() :
	disableMaster(master, waveCounter),
	lengthCounter(disableMaster, 0xFF),
	cycleCounter(0),
	soMask(0),
	prevOut(0),
	waveCounter(SoundUnit::COUNTER_DISABLED),
	lastReadTime(0),
	nr0(0),
	nr3(0),
	nr4(0),
	wavePos(0),
	rShift(4),
	sampleBuf(0),
	master(false),
	cgb(false)
{}

void Channel3::setNr0(const unsigned data) {
	nr0 = data & 0x80;
	
	if (!(data & 0x80))
		disableMaster();
}

void Channel3::setNr2(const unsigned data) {
	rShift = (data >> 5 & 3U) - 1;
	
	if (rShift > 3)
		rShift = 4;
}

void Channel3::setNr4(const unsigned data) {
	lengthCounter.nr4Change(nr4, data, cycleCounter);
		
	nr4 = data & 0x7F;
	
	if (data & nr0/* & 0x80*/) {
		if (!cgb && waveCounter == cycleCounter + 1) {
			const unsigned pos = ((wavePos + 1) & 0x1F) >> 1;
			
			if (pos < 4)
				waveRam[0] = waveRam[pos];
			else
				std::memcpy(waveRam, waveRam + (pos & ~3), 4);
		}
		
		master = true;
		wavePos = 0;
		lastReadTime = waveCounter = cycleCounter + toPeriod(nr3, data) + 3;
	}
}

void Channel3::setSo(const unsigned long soMask) {
	this->soMask = soMask;
}

void Channel3::reset() {
	cycleCounter = 0x1000 | (cycleCounter & 0xFFF); // cycleCounter >> 12 & 7 represents the frame sequencer position.

// 	lengthCounter.reset();
	sampleBuf = 0;
}

void Channel3::init(const bool cgb) {
	this->cgb = cgb;
	lengthCounter.init(cgb);
}

void Channel3::setStatePtrs(SaveState &state) {
	state.spu.ch3.waveRam.set(waveRam, sizeof waveRam);
}

void Channel3::saveState(SaveState &state) const {
	lengthCounter.saveState(state.spu.ch3.lcounter);
	
	state.spu.ch3.waveCounter = waveCounter;
	state.spu.ch3.lastReadTime = lastReadTime;
	state.spu.ch3.nr3 = nr3;
	state.spu.ch3.nr4 = nr4;
	state.spu.ch3.wavePos = wavePos;
	state.spu.ch3.sampleBuf = sampleBuf;
	state.spu.ch3.master = master;
}

void Channel3::loadState(const SaveState &state) {
	lengthCounter.loadState(state.spu.ch3.lcounter, state.spu.cycleCounter);
	
	cycleCounter = state.spu.cycleCounter;
	waveCounter = std::max(state.spu.ch3.waveCounter, state.spu.cycleCounter);
	lastReadTime = state.spu.ch3.lastReadTime;
	nr3 = state.spu.ch3.nr3;
	nr4 = state.spu.ch3.nr4;
	wavePos = state.spu.ch3.wavePos & 0x1F;
	sampleBuf = state.spu.ch3.sampleBuf;
	master = state.spu.ch3.master;
	
	nr0 = state.mem.ioamhram.get()[0x11A] & 0x80;
	setNr2(state.mem.ioamhram.get()[0x11C]);
}

void Channel3::updateWaveCounter(const unsigned long cc) {
	if (cc >= waveCounter) {
		const unsigned period = toPeriod(nr3, nr4);
		const unsigned long periods = (cc - waveCounter) / period;

		lastReadTime = waveCounter + periods * period;
		waveCounter = lastReadTime + period;

		wavePos += periods + 1;
		wavePos &= 0x1F;

		sampleBuf = waveRam[wavePos >> 1];
	}
}

void Channel3::update(Gambatte::uint_least32_t *buf, const unsigned long soBaseVol, unsigned long cycles) {
	const unsigned long outBase = (nr0/* & 0x80*/) ? soBaseVol & soMask : 0;
	
	if (outBase && rShift != 4) {
		const unsigned long endCycles = cycleCounter + cycles;
		
		for (;;) {
			const unsigned long nextMajorEvent = lengthCounter.getCounter() < endCycles ? lengthCounter.getCounter() : endCycles;
			unsigned long out = outBase * (master ? ((sampleBuf >> (~wavePos << 2 & 4) & 0xF) >> rShift) * 2 - 15ul : 0 - 15ul);
		
			while (waveCounter <= nextMajorEvent) {
				*buf += out - prevOut;
				prevOut = out;
				buf += waveCounter - cycleCounter;
				cycleCounter = waveCounter;
			
				lastReadTime = waveCounter;
				waveCounter += toPeriod(nr3, nr4);
				++wavePos;
				wavePos &= 0x1F;
				sampleBuf = waveRam[wavePos >> 1];
				out = outBase * (/*master ? */((sampleBuf >> (~wavePos << 2 & 4) & 0xF) >> rShift) * 2 - 15ul/* : 0 - 15ul*/);
			}
		
			if (cycleCounter < nextMajorEvent) {
				*buf += out - prevOut;
				prevOut = out;
				buf += nextMajorEvent - cycleCounter;
				cycleCounter = nextMajorEvent;
			}
		
			if (lengthCounter.getCounter() == nextMajorEvent) {
				lengthCounter.event();
			} else
				break;
		}
	} else {
		if (outBase) {
			const unsigned long out = outBase * (0 - 15ul);
			
			*buf += out - prevOut;
			prevOut = out;
		}
		
		cycleCounter += cycles;
		
		while (lengthCounter.getCounter() <= cycleCounter) {
			updateWaveCounter(lengthCounter.getCounter());
			lengthCounter.event();
		}
		
		updateWaveCounter(cycleCounter);
	}
	
	if (cycleCounter & SoundUnit::COUNTER_MAX) {
		lengthCounter.resetCounters(cycleCounter);
		
		if (waveCounter != SoundUnit::COUNTER_DISABLED)
			waveCounter -= SoundUnit::COUNTER_MAX;
		
		lastReadTime -= SoundUnit::COUNTER_MAX;
		cycleCounter -= SoundUnit::COUNTER_MAX;
	}
}
