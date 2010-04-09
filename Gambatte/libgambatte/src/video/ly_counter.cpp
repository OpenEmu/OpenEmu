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
#include "ly_counter.h"
#include "../savestate.h"

LyCounter::LyCounter() : VideoEvent(0) {
	setDoubleSpeed(false);
	reset(0, 0);
}

void LyCounter::doEvent() {
	++ly_;
	
	if (ly_ == 154)
		ly_ = 0;
	
	setTime(time() + lineTime_);
}

unsigned long LyCounter::nextLineCycle(const unsigned lineCycle, const unsigned long cycleCounter) const {
	unsigned long tmp = time() + (lineCycle << ds);
	
	if (tmp - cycleCounter > lineTime_)
		tmp -= lineTime_;
	
	return tmp;
}

unsigned long LyCounter::nextFrameCycle(const unsigned long frameCycle, const unsigned long cycleCounter) const {
	unsigned long tmp = time() + (((153U - ly()) * 456U + frameCycle) << ds);
	
	if (tmp - cycleCounter > 70224U << ds)
		tmp -= 70224U << ds;
	
	return tmp;
}

void LyCounter::reset(const unsigned long videoCycles, const unsigned long lastUpdate) {
	ly_ = videoCycles / 456;
	setTime(lastUpdate + ((456 - (videoCycles - ly_ * 456ul)) << isDoubleSpeed()));
}

void LyCounter::setDoubleSpeed(const bool ds_in) {
	ds = ds_in;
	lineTime_ = 456U << ds_in;
}
