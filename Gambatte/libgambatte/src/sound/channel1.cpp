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
#include "channel1.h"
#include "../savestate.h"
#include <algorithm>

Channel1::SweepUnit::SweepUnit(MasterDisabler &disabler, DutyUnit &dutyUnit) :
	disableMaster(disabler),
	dutyUnit(dutyUnit),
	shadow(0),
	nr0(0),
	negging(false)
{}

unsigned Channel1::SweepUnit::calcFreq() {
	unsigned freq = shadow >> (nr0 & 0x07);
	
	if (nr0 & 0x08) {
		freq = shadow - freq;
		negging = true;
	} else
		freq = shadow + freq;
	
	if (freq & 2048)
		disableMaster();
	
	return freq;
}

void Channel1::SweepUnit::event() {
	const unsigned long period = nr0 >> 4 & 0x07;
	
	if (period) {
		const unsigned freq = calcFreq();
		
		if (!(freq & 2048) && (nr0 & 0x07)) {
			shadow = freq;
			dutyUnit.setFreq(freq, counter);
			calcFreq();
		}
		
		counter += period << 14;
	} else
		counter += 8ul << 14;
}

void Channel1::SweepUnit::nr0Change(const unsigned newNr0) {
	if (negging && !(newNr0 & 0x08))
		disableMaster();
	
	nr0 = newNr0;
}

void Channel1::SweepUnit::nr4Init(const unsigned long cc) {
	negging = false;
	shadow = dutyUnit.getFreq();
	
	const unsigned period = nr0 >> 4 & 0x07;
	const unsigned shift = nr0 & 0x07;
	
	if (period | shift)
		counter = ((cc >> 14) + (period ? period : 8)) << 14;
	else
		counter = COUNTER_DISABLED;
	
	if (shift)
		calcFreq();
}

void Channel1::SweepUnit::reset() {
	counter = COUNTER_DISABLED;
}

void Channel1::SweepUnit::saveState(SaveState &state) const {
	state.spu.ch1.sweep.counter = counter;
	state.spu.ch1.sweep.shadow = shadow;
	state.spu.ch1.sweep.nr0 = nr0;
	state.spu.ch1.sweep.negging = negging;
}

void Channel1::SweepUnit::loadState(const SaveState &state) {
	counter = std::max(state.spu.ch1.sweep.counter, state.spu.cycleCounter);
	shadow = state.spu.ch1.sweep.shadow;
	nr0 = state.spu.ch1.sweep.nr0;
	negging = state.spu.ch1.sweep.negging;
}

Channel1::Channel1() :
	staticOutputTest(*this, dutyUnit),
	disableMaster(master, dutyUnit),
	lengthCounter(disableMaster, 0x3F),
	envelopeUnit(staticOutputTest),
	sweepUnit(disableMaster, dutyUnit),
	cycleCounter(0),
	soMask(0),
	prevOut(0),
	nr4(0),
	master(false)
{
	setEvent();
}

void Channel1::setEvent() {
// 	nextEventUnit = &dutyUnit;
// 	if (sweepUnit.getCounter() < nextEventUnit->getCounter())
		nextEventUnit = &sweepUnit;
	if (envelopeUnit.getCounter() < nextEventUnit->getCounter())
		nextEventUnit = &envelopeUnit;
	if (lengthCounter.getCounter() < nextEventUnit->getCounter())
		nextEventUnit = &lengthCounter;
}

void Channel1::setNr0(const unsigned data) {
	sweepUnit.nr0Change(data);
	setEvent();
}

void Channel1::setNr1(const unsigned data) {
	lengthCounter.nr1Change(data, nr4, cycleCounter);
	dutyUnit.nr1Change(data, cycleCounter);
	
	setEvent();
}

void Channel1::setNr2(const unsigned data) {
	if (envelopeUnit.nr2Change(data))
		disableMaster();
	else
		staticOutputTest(cycleCounter);
	
	setEvent();
}

void Channel1::setNr3(const unsigned data) {
	dutyUnit.nr3Change(data, cycleCounter);
	setEvent();
}

void Channel1::setNr4(const unsigned data) {
	lengthCounter.nr4Change(nr4, data, cycleCounter);
		
	nr4 = data;
	
	dutyUnit.nr4Change(data, cycleCounter);
	
	if (data & 0x80) { //init-bit
		nr4 &= 0x7F;
		master = !envelopeUnit.nr4Init(cycleCounter);
		sweepUnit.nr4Init(cycleCounter);
		staticOutputTest(cycleCounter);
	}
	
	setEvent();
}

void Channel1::setSo(const unsigned long soMask) {
	this->soMask = soMask;
	staticOutputTest(cycleCounter);
	setEvent();
}

void Channel1::reset() {
	cycleCounter = 0x1000 | (cycleCounter & 0xFFF); // cycleCounter >> 12 & 7 represents the frame sequencer position.

// 	lengthCounter.reset();
	dutyUnit.reset();
	envelopeUnit.reset();
	sweepUnit.reset();
	
	setEvent();
}

void Channel1::init(const bool cgb) {
	lengthCounter.init(cgb);
}

void Channel1::saveState(SaveState &state) {
	sweepUnit.saveState(state);
	dutyUnit.saveState(state.spu.ch1.duty, cycleCounter);
	envelopeUnit.saveState(state.spu.ch1.env);
	lengthCounter.saveState(state.spu.ch1.lcounter);
	
	state.spu.cycleCounter = cycleCounter;
	state.spu.ch1.nr4 = nr4;
	state.spu.ch1.master = master;
}

void Channel1::loadState(const SaveState &state) {
	sweepUnit.loadState(state);
	dutyUnit.loadState(state.spu.ch1.duty, state.mem.ioamhram.get()[0x111], state.spu.ch1.nr4, state.spu.cycleCounter);
	envelopeUnit.loadState(state.spu.ch1.env, state.mem.ioamhram.get()[0x112], state.spu.cycleCounter);
	lengthCounter.loadState(state.spu.ch1.lcounter, state.spu.cycleCounter);
	
	cycleCounter = state.spu.cycleCounter;
	nr4 = state.spu.ch1.nr4;
	master = state.spu.ch1.master;
}

void Channel1::update(Gambatte::uint_least32_t *buf, const unsigned long soBaseVol, unsigned long cycles) {
	const unsigned long outBase = envelopeUnit.dacIsOn() ? soBaseVol & soMask : 0;
	const unsigned long outLow = outBase * (0 - 15ul);
	const unsigned long endCycles = cycleCounter + cycles;
	
	for (;;) {
		const unsigned long outHigh = master ? outBase * (envelopeUnit.getVolume() * 2 - 15ul) : outLow;
		const unsigned long nextMajorEvent = nextEventUnit->getCounter() < endCycles ? nextEventUnit->getCounter() : endCycles;
		unsigned long out = dutyUnit.isHighState() ? outHigh : outLow;
		
		while (dutyUnit.getCounter() <= nextMajorEvent) {
			*buf = out - prevOut;
			prevOut = out;
			buf += dutyUnit.getCounter() - cycleCounter;
			cycleCounter = dutyUnit.getCounter();
			
			dutyUnit.event();
			out = dutyUnit.isHighState() ? outHigh : outLow;
		}
		
		if (cycleCounter < nextMajorEvent) {
			*buf = out - prevOut;
			prevOut = out;
			buf += nextMajorEvent - cycleCounter;
			cycleCounter = nextMajorEvent;
		}
		
		if (nextEventUnit->getCounter() == nextMajorEvent) {
			nextEventUnit->event();
			setEvent();
		} else
			break;
	}
	
	if (cycleCounter & SoundUnit::COUNTER_MAX) {
		dutyUnit.resetCounters(cycleCounter);
		lengthCounter.resetCounters(cycleCounter);
		envelopeUnit.resetCounters(cycleCounter);
		sweepUnit.resetCounters(cycleCounter);
		
		cycleCounter -= SoundUnit::COUNTER_MAX;
	}
}
