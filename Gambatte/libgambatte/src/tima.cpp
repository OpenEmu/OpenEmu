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
#include "tima.h"
#include "savestate.h"

static const unsigned char timaClock[4] = { 10, 4, 6, 8 };

namespace gambatte {

Tima::Tima() :
lastUpdate_(0),
tmatime_(DISABLED_TIME),
tima_(0),
tma_(0),
tac_(0)
{}

void Tima::saveState(SaveState &state) const {
	state.mem.timaLastUpdate = lastUpdate_;
	state.mem.tmatime = tmatime_;
}

void Tima::loadState(const SaveState &state, const TimaInterruptRequester timaIrq) {
	lastUpdate_ = state.mem.timaLastUpdate;
	tmatime_ = state.mem.tmatime;
	
	tima_ = state.mem.ioamhram.get()[0x105];
	tma_  = state.mem.ioamhram.get()[0x106];
	tac_  = state.mem.ioamhram.get()[0x107];
	
	timaIrq.setNextIrqEventTime((tac_ & 4)
		?
			(tmatime_ != DISABLED_TIME && tmatime_ > state.cpu.cycleCounter
			          ? tmatime_
			          : lastUpdate_ + ((256u - tima_) << timaClock[tac_ & 3]) + 3)
		:
			static_cast<unsigned long>(DISABLED_TIME)
	);
}

void Tima::resetCc(const unsigned long oldCc, const unsigned long newCc, const TimaInterruptRequester timaIrq) {
	const unsigned long dec = oldCc - newCc;
	
	if (tac_ & 0x04) {
		updateIrq(oldCc, timaIrq);
		updateTima(oldCc);
		
		lastUpdate_ -= dec;
		timaIrq.setNextIrqEventTime(timaIrq.nextIrqEventTime() - dec);
		
		if (tmatime_ != DISABLED_TIME)
			tmatime_ -= dec;
	}
}

void Tima::updateTima(const unsigned long cycleCounter) {
	const unsigned long ticks = (cycleCounter - lastUpdate_) >> timaClock[tac_ & 3];

	lastUpdate_ += ticks << timaClock[tac_ & 3];

	if (cycleCounter >= tmatime_) {
		if (cycleCounter >= tmatime_ + 4)
			tmatime_ = DISABLED_TIME;

		tima_ = tma_;
	}

	unsigned long tmp = tima_ + ticks;

	while (tmp > 0x100)
		tmp -= 0x100 - tma_;

	if (tmp == 0x100) {
		tmp = 0;
		tmatime_ = lastUpdate_ + 3;

		if (cycleCounter >= tmatime_) {
			if (cycleCounter >= tmatime_ + 4)
				tmatime_ = DISABLED_TIME;

			tmp = tma_;
		}
	}

	tima_ = tmp;
}

void Tima::setTima(const unsigned data, const unsigned long cycleCounter, const TimaInterruptRequester timaIrq) {
	if (tac_ & 0x04) {
		updateIrq(cycleCounter, timaIrq);
		updateTima(cycleCounter);

		if (tmatime_ - cycleCounter < 4)
			tmatime_ = DISABLED_TIME;

		timaIrq.setNextIrqEventTime(lastUpdate_ + ((256u - data) << timaClock[tac_ & 3]) + 3);
	}
	
	tima_ = data;
}

void Tima::setTma(const unsigned data, const unsigned long cycleCounter, const TimaInterruptRequester timaIrq) {
	if (tac_ & 0x04) {
		updateIrq(cycleCounter, timaIrq);
		updateTima(cycleCounter);
	}
	
	tma_ = data;
}

void Tima::setTac(const unsigned data, const unsigned long cycleCounter, const TimaInterruptRequester timaIrq) {
	if (tac_ ^ data) {
		unsigned long nextIrqEventTime = timaIrq.nextIrqEventTime();
		
		if (tac_ & 0x04) {
			updateIrq(cycleCounter, timaIrq);
			updateTima(cycleCounter);

			lastUpdate_ -= (1u << (timaClock[tac_ & 3] - 1)) + 3;
			tmatime_ -= (1u << (timaClock[tac_ & 3] - 1)) + 3;
			nextIrqEventTime -= (1u << (timaClock[tac_ & 3] - 1)) + 3;
			
			if (cycleCounter >= nextIrqEventTime)
				timaIrq.flagIrq();
			
			updateTima(cycleCounter);

			tmatime_ = DISABLED_TIME;
			nextIrqEventTime = DISABLED_TIME;
		}

		if (data & 4) {
			lastUpdate_ = (cycleCounter >> timaClock[data & 3]) << timaClock[data & 3];
			nextIrqEventTime = lastUpdate_ + ((256u - tima_) << timaClock[data & 3]) + 3;
		}
		
		timaIrq.setNextIrqEventTime(nextIrqEventTime);
	}
	
	tac_ = data;
}

unsigned Tima::tima(unsigned long cycleCounter) {
	if (tac_ & 0x04)
		updateTima(cycleCounter);

	return tima_;
}

void Tima::doIrqEvent(const TimaInterruptRequester timaIrq) {
	timaIrq.flagIrq();
	timaIrq.setNextIrqEventTime(timaIrq.nextIrqEventTime() + ((256u - tma_) << timaClock[tac_ & 3]));
}

}
