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
#include "rtc.h"
#include "savestate.h"

using namespace std;

Rtc::Rtc() :
activeData(NULL),
activeSet(NULL),
baseTime(0),
haltTime(0),
index(5),
dataDh(0),
dataDl(0),
dataH(0),
dataM(0),
dataS(0),
enabled(false),
lastLatchData(false) {
}

void Rtc::doLatch() {
	time_t tmp = ((dataDh & 0x40) ? haltTime : time(NULL)) - baseTime;
	
	while (tmp > 0x1FF * 86400) {
		baseTime += 0x1FF * 86400;
		tmp -= 0x1FF * 86400;
		dataDh |= 0x80;
	}
	
	dataDl = (tmp / 86400) & 0xFF;
	dataDh &= 0xFE;
	dataDh |= ((tmp / 86400) & 0x100) >> 8;
	tmp %= 86400;
	
	dataH = tmp / 3600;
	tmp %= 3600;
	
	dataM = tmp / 60;
	tmp %= 60;
	
	dataS = tmp;
}

void Rtc::doSwapActive() {
	if (!enabled || index > 4) {
		activeData = NULL;
		activeSet = NULL;
	} else switch (index) {
	case 0x00:
		activeData = &dataS;
		activeSet = &Rtc::setS;
		break;
	case 0x01:
		activeData = &dataM;
		activeSet = &Rtc::setM;
		break;
	case 0x02:
		activeData = &dataH;
		activeSet = &Rtc::setH;
		break;
	case 0x03:
		activeData = &dataDl;
		activeSet = &Rtc::setDl;
		break;
	case 0x04:
		activeData = &dataDh;
		activeSet = &Rtc::setDh;
		break;
	}
}

void Rtc::saveState(SaveState &state) const {
	state.rtc.baseTime = baseTime;
	state.rtc.haltTime = haltTime;
	state.rtc.index = index;
	state.rtc.dataDh = dataDh;
	state.rtc.dataDl = dataDl;
	state.rtc.dataH = dataH;
	state.rtc.dataM = dataM;
	state.rtc.dataS = dataS;
	state.rtc.lastLatchData = lastLatchData;
}

void Rtc::loadState(const SaveState &state, const bool enabled) {
	this->enabled = enabled;
	
	baseTime = state.rtc.baseTime;
	haltTime = state.rtc.haltTime;
	index = state.rtc.index;
	dataDh = state.rtc.dataDh;
	dataDl = state.rtc.dataDl;
	dataH = state.rtc.dataH;
	dataM = state.rtc.dataM;
	dataS = state.rtc.dataS;
	lastLatchData = state.rtc.lastLatchData;
	
	doSwapActive();
}

void Rtc::setDh(const unsigned new_dh) {
	const time_t unixtime = (dataDh & 0x40) ? haltTime : time(NULL);
	const time_t old_highdays = ((unixtime - baseTime) / 86400) & 0x100;
	baseTime += old_highdays * 86400;
	baseTime -= ((new_dh & 0x1) << 8) * 86400;
	
	if ((dataDh ^ new_dh) & 0x40) {
		if (new_dh & 0x40)
			haltTime = time(NULL);
		else
			baseTime += time(NULL) - haltTime;
	}
}

void Rtc::setDl(const unsigned new_lowdays) {
	const time_t unixtime = (dataDh & 0x40) ? haltTime : time(NULL);
	const time_t old_lowdays = ((unixtime - baseTime) / 86400) & 0xFF;
	baseTime += old_lowdays * 86400;
	baseTime -= new_lowdays * 86400;
}

void Rtc::setH(const unsigned new_hours) {
	const time_t unixtime = (dataDh & 0x40) ? haltTime : time(NULL);
	const time_t old_hours = ((unixtime - baseTime) / 3600) % 24;
	baseTime += old_hours * 3600;
	baseTime -= new_hours * 3600;
}

void Rtc::setM(const unsigned new_minutes) {
	const time_t unixtime = (dataDh & 0x40) ? haltTime : time(NULL);
	const time_t old_minutes = ((unixtime - baseTime) / 60) % 60;
	baseTime += old_minutes * 60;
	baseTime -= new_minutes * 60;
}

void Rtc::setS(const unsigned new_seconds) {
	const time_t unixtime = (dataDh & 0x40) ? haltTime : time(NULL);
	baseTime += (unixtime - baseTime) % 60;
	baseTime -= new_seconds;
}
