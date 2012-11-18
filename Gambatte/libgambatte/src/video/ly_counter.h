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
#ifndef LY_COUNTER_H
#define LY_COUNTER_H

namespace gambatte {

struct SaveState;

class LyCounter {
	unsigned long time_;
	unsigned short lineTime_;
	unsigned char ly_;
	bool ds;
	
public:
	LyCounter();
	void doEvent();
	bool isDoubleSpeed() const { return ds; }
	
	unsigned long frameCycles(const unsigned long cc) const {
		return ly_ * 456ul + lineCycles(cc);
	}
	
	unsigned lineCycles(const unsigned long cc) const {
		return 456u - ((time_ - cc) >> isDoubleSpeed());
	}
	
	unsigned lineTime() const { return lineTime_; }
	unsigned ly() const { return ly_; }
	unsigned long nextLineCycle(unsigned lineCycle, unsigned long cycleCounter) const;
	unsigned long nextFrameCycle(unsigned long frameCycle, unsigned long cycleCounter) const;
	void reset(unsigned long videoCycles, unsigned long lastUpdate);
	void setDoubleSpeed(bool ds_in);
	unsigned long time() const { return time_; }
};

}

#endif
