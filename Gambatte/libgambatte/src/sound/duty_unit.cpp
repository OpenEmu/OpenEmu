/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
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
#include "duty_unit.h"
#include <algorithm>

static inline bool toOutState(const unsigned duty, const unsigned pos) {
	static const unsigned char duties[4] = { 0x80, 0x81, 0xE1, 0x7E };
	
	return duties[duty] >> pos & 1;
}

static inline unsigned toPeriod(const unsigned freq) {
	return (2048 - freq) << 1;
}

void DutyUnit::updatePos(const unsigned long cc) {
	if (cc >= nextPosUpdate) {
		const unsigned long inc = (cc - nextPosUpdate) / period + 1;
		nextPosUpdate += period * inc;
		pos += inc;
		pos &= 7;
	}
}

void DutyUnit::setDuty(const unsigned nr1) {
	duty = nr1 >> 6;
	high = toOutState(duty, pos);
}

void DutyUnit::setCounter() {
	static const unsigned char nextStateDistance[4 * 8] = {
		6, 5, 4, 3, 2, 1, 0, 0,
		0, 5, 4, 3, 2, 1, 0, 1,
		0, 3, 2, 1, 0, 3, 2, 1,
		0, 5, 4, 3, 2, 1, 0, 1
	};
	
	if (enableEvents && nextPosUpdate != COUNTER_DISABLED)
		counter = nextPosUpdate + period * nextStateDistance[(duty * 8) | pos];
	else
		counter = COUNTER_DISABLED;
}

void DutyUnit::setFreq(const unsigned newFreq, const unsigned long cc) {
	updatePos(cc);
	period = toPeriod(newFreq);
	setCounter();
}

void DutyUnit::event() {
	unsigned inc = period << duty;
	
	if (duty == 3)
		inc -= period * 2;
	
	if (!(high ^= true))
		inc = period * 8 - inc;
	
	counter += inc;
}

void DutyUnit::nr1Change(const unsigned newNr1, const unsigned long cc) {
	updatePos(cc);
	setDuty(newNr1);
	setCounter();
}

void DutyUnit::nr3Change(const unsigned newNr3, const unsigned long cc) {
	setFreq((getFreq() & 0x700) | newNr3, cc);
}

void DutyUnit::nr4Change(const unsigned newNr4, const unsigned long cc) {
	setFreq((newNr4 << 8 & 0x700) | (getFreq() & 0xFF), cc);
	
	if (newNr4 & 0x80) {
		nextPosUpdate = (cc & ~1) + period;
		setCounter();
	}
}

DutyUnit::DutyUnit() :
nextPosUpdate(COUNTER_DISABLED),
period(4096),
pos(0),
duty(0),
high(false),
enableEvents(true)
{}

void DutyUnit::reset() {
	pos = 0;
	high = toOutState(duty, pos);
	nextPosUpdate = COUNTER_DISABLED;
	setCounter();
}

void DutyUnit::saveState(SaveState::SPU::Duty &dstate, const unsigned long cc) {
	updatePos(cc);
	dstate.nextPosUpdate = nextPosUpdate;
	dstate.nr3 = getFreq() & 0xFF;
	dstate.pos = pos;
}

void DutyUnit::loadState(const SaveState::SPU::Duty &dstate, const unsigned nr1, const unsigned nr4, const unsigned long cc) {
	nextPosUpdate = std::max(dstate.nextPosUpdate, cc);
	pos = dstate.pos & 7;
	setDuty(nr1);
	period = toPeriod((nr4 << 8 & 0x700) | dstate.nr3);
	enableEvents = true;
	setCounter();
}

void DutyUnit::resetCounters(const unsigned long oldCc) {
	if (nextPosUpdate == COUNTER_DISABLED)
		return;
	
	updatePos(oldCc);
	nextPosUpdate -= COUNTER_MAX;
	SoundUnit::resetCounters(oldCc);
}

void DutyUnit::killCounter() {
	enableEvents = false;
	setCounter();
}

void DutyUnit::reviveCounter(const unsigned long cc) {
	updatePos(cc);
	high = toOutState(duty, pos);
	enableEvents = true;
	setCounter();
}
