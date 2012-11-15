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
#include "lyc_irq.h"
#include "counterdef.h"
#include "ly_counter.h"
#include "savestate.h"
#include <algorithm>

namespace gambatte {

LycIrq::LycIrq() :
	time_(DISABLED_TIME),
	lycRegSrc_(0),
	statRegSrc_(0),
	lycReg_(0),
	statReg_(0),
	cgb_(false)
{
}

static unsigned long schedule(const unsigned statReg, const unsigned lycReg, const LyCounter &lyCounter, const unsigned long cc) {
	return (statReg & 0x40) && lycReg < 154
		? lyCounter.nextFrameCycle(lycReg ? lycReg * 456 : 153 * 456 + 8, cc)
		: static_cast<unsigned long>(DISABLED_TIME);
}

void LycIrq::regChange(const unsigned statReg, const unsigned lycReg, const LyCounter &lyCounter, const unsigned long cc) {
	const unsigned long timeSrc = schedule(statReg, lycReg, lyCounter, cc);
	statRegSrc_ = statReg;
	lycRegSrc_ = lycReg;
	time_ = std::min(time_, timeSrc);
	
	if (cgb_) {
		if (time_ - cc > 8 || (timeSrc != time_ && time_ - cc > 4U - lyCounter.isDoubleSpeed() * 4U))
			lycReg_ = lycReg;
		
		if (time_ - cc > 4U - lyCounter.isDoubleSpeed() * 4U)
			statReg_ = statReg;
	} else {
		if (time_ - cc > 4 || timeSrc != time_)
			lycReg_ = lycReg;
		
		if (time_ - cc > 4 || lycReg_ != 0)
			statReg_ = statReg;
		
		statReg_ = (statReg_ & 0x40) | (statReg & ~0x40);
	}
}

void LycIrq::doEvent(unsigned char *const ifreg, const LyCounter &lyCounter) {
	if ((statReg_ | statRegSrc_) & 0x40) {
		const unsigned cmpLy = lyCounter.time() - time_ < lyCounter.lineTime() ? 0 : lyCounter.ly();
		
		if (lycReg_ == cmpLy &&
				(lycReg_ - 1U < 144U - 1U ? !(statReg_ & 0x20) : !(statReg_ & 0x10))) {
			*ifreg |= 2;
		}
	}
	
	lycReg_ = lycRegSrc_;
	statReg_ = statRegSrc_;
	time_ = schedule(statReg_, lycReg_, lyCounter, time_);
}

void LycIrq::loadState(const SaveState &state) {
	lycRegSrc_ = state.mem.ioamhram.get()[0x145];
	statRegSrc_ = state.mem.ioamhram.get()[0x141];
	lycReg_ = state.ppu.lyc;
	statReg_ = statRegSrc_;
}

void LycIrq::saveState(SaveState &state) const {
	state.ppu.lyc = lycReg_;
}

void LycIrq::reschedule(const LyCounter & lyCounter, const unsigned long cc) {
	time_ = std::min(schedule(statReg_   , lycReg_   , lyCounter, cc),
	                 schedule(statRegSrc_, lycRegSrc_, lyCounter, cc));
}

void LycIrq::lcdReset() {
	statReg_ = statRegSrc_;
	lycReg_ = lycRegSrc_;
}

}
