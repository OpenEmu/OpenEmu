/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
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
#ifndef RTC_H
#define RTC_H

#include <ctime>

namespace gambatte {

struct SaveState;

class Rtc {
private:
	unsigned char *activeData;
	void (Rtc::*activeSet)(unsigned);
	std::time_t baseTime;
	std::time_t haltTime;
	unsigned char index;
	unsigned char dataDh;
	unsigned char dataDl;
	unsigned char dataH;
	unsigned char dataM;
	unsigned char dataS;
	bool enabled;
	bool lastLatchData;
	
	void doLatch();
	void doSwapActive();
	void setDh(unsigned new_dh);
	void setDl(unsigned new_lowdays);
	void setH(unsigned new_hours);
	void setM(unsigned new_minutes);
	void setS(unsigned new_seconds);
	
public:
	Rtc();
	
	const unsigned char* getActive() const { return activeData; }
	std::time_t& getBaseTime() { return baseTime; }
	
	void setBaseTime(const std::time_t baseTime) {
		this->baseTime = baseTime;
// 		doLatch();
	}
	
	void latch(const unsigned data) {
		if (!lastLatchData && data == 1)
			doLatch();
		
		lastLatchData = data;
	}
	
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state);
	
	void set(const bool enabled, unsigned bank) {
		bank &= 0xF;
		bank -= 8;
		
		this->enabled = enabled;
		this->index = bank;
		
		doSwapActive();
	}
	
	void write(const unsigned data) {
// 		if (activeSet)
		(this->*activeSet)(data);
		*activeData = data;
	}
};

}

#endif
