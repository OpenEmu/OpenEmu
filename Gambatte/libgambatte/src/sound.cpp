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
#include "sound.h"

#include "savestate.h"
#include <cstring>
#include <algorithm>

/*
	Frame Sequencer

	Step    Length Ctr  Vol Env     Sweep
	- - - - - - - - - - - - - - - - - - - -
	0       Clock       -           Clock
S	1       -           Clock       -
	2       Clock       -           -
	3       -           -           -
	4       Clock       -           Clock
	5       -           -           -
	6       Clock       -           -
	7       -           -           -
	- - - - - - - - - - - - - - - - - - - -
	Rate    256 Hz      64 Hz       128 Hz

S) start step on sound power on.
*/

// static const unsigned bufferSize = 35112 + 16 + 2048; //FIXME: DMA can prevent process from returning for up to 4096 cycles.

PSG::PSG() :
buffer(NULL),
lastUpdate(0),
soVol(0),
rsum(0x8000), // initialize to 0x8000 to prevent borrows from high word, xor away later
bufferPos(0),
enabled(false)
{}

void PSG::init(const bool cgb) {
	ch1.init(cgb);
	ch2.init(cgb);
	ch3.init(cgb);
	ch4.init(cgb);
}

void PSG::reset() {
	ch1.reset();
	ch2.reset();
	ch3.reset();
	ch4.reset();
}

void PSG::setStatePtrs(SaveState &state) {
	ch3.setStatePtrs(state);
}

void PSG::saveState(SaveState &state) {
	ch1.saveState(state);
	ch2.saveState(state);
	ch3.saveState(state);
	ch4.saveState(state);
}

void PSG::loadState(const SaveState &state) {
	ch1.loadState(state);
	ch2.loadState(state);
	ch3.loadState(state);
	ch4.loadState(state);
	
	lastUpdate = state.cpu.cycleCounter;
	set_so_volume(state.mem.ioamhram.get()[0x124]);
	map_so(state.mem.ioamhram.get()[0x125]);
	enabled = state.mem.ioamhram.get()[0x126] >> 7 & 1;
}

void PSG::accumulate_channels(const unsigned long cycles) {
	Gambatte::uint_least32_t *const buf = buffer + bufferPos;
	
	std::memset(buf, 0, cycles * sizeof(Gambatte::uint_least32_t));
	ch1.update(buf, soVol, cycles);
	ch2.update(buf, soVol, cycles);
	ch3.update(buf, soVol, cycles);
	ch4.update(buf, soVol, cycles);
}

void PSG::generate_samples(const unsigned long cycleCounter, const unsigned doubleSpeed) {
	const unsigned long cycles = (cycleCounter - lastUpdate) >> (1 + doubleSpeed);
	lastUpdate += cycles << (1 + doubleSpeed);

	if (cycles)
		accumulate_channels(cycles);
	
	bufferPos += cycles;
}

void PSG::resetCounter(const unsigned long newCc, const unsigned long oldCc, const unsigned doubleSpeed) {
	generate_samples(oldCc, doubleSpeed);
	lastUpdate = newCc - (oldCc - lastUpdate);
}

unsigned PSG::fillBuffer() {
	Gambatte::uint_least32_t sum = rsum;
	Gambatte::uint_least32_t *b = buffer;
	unsigned n = bufferPos;
	
	while (n--) {
		sum += *b;
		*b++ = sum ^ 0x8000; // xor away the initial rsum value of 0x8000 (which prevents borrows from the high word) from the low word
	}
	
	rsum = sum;
	
	return bufferPos;
}

#ifdef WORDS_BIGENDIAN
static const unsigned long so1Mul = 0x00000001;
static const unsigned long so2Mul = 0x00010000;
#else
static const unsigned long so1Mul = 0x00010000;
static const unsigned long so2Mul = 0x00000001;
#endif

void PSG::set_so_volume(const unsigned nr50) {
	soVol = (((nr50 & 0x7) + 1) * so1Mul + ((nr50 >> 4 & 0x7) + 1) * so2Mul) * 64;
}

void PSG::map_so(const unsigned nr51) {
	const unsigned long tmp = nr51 * so1Mul + (nr51 >> 4) * so2Mul;
	
	ch1.setSo((tmp      & 0x00010001) * 0xFFFF);
	ch2.setSo((tmp >> 1 & 0x00010001) * 0xFFFF);
	ch3.setSo((tmp >> 2 & 0x00010001) * 0xFFFF);
	ch4.setSo((tmp >> 3 & 0x00010001) * 0xFFFF);
}

unsigned PSG::getStatus() const {
	return ch1.isActive() | (ch2.isActive() << 1) | (ch3.isActive() << 2) | (ch4.isActive() << 3);
}
