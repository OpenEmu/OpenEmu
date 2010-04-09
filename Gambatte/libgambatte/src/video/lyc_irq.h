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
#ifndef VIDEO_LYC_IRQ_H
#define VIDEO_LYC_IRQ_H

#include "ly_counter.h"

class LycIrq : public VideoEvent {
	unsigned char &ifReg;
	unsigned long frameTime;
	unsigned char lycReg_;
	bool m2IrqEnabled;
	bool skip;
	
public:
	LycIrq(unsigned char &ifReg_in);
	
	void doEvent();
	
	unsigned lycReg() const {
		return lycReg_;
	}
	
	static unsigned long schedule(unsigned statReg, unsigned lycReg, const LyCounter &lyCounter, unsigned long cycleCounter);
	
	void setDoubleSpeed(const bool ds) {
		frameTime = 70224 << ds;
	}
	
	void setLycReg(const unsigned lycReg_in) {
		lycReg_ = lycReg_in;
	}
	
	void setM2IrqEnabled(const bool enabled) {
		m2IrqEnabled = enabled;
	}
	
	void setSkip(const bool skip) {
		this->skip = skip;
	}
	
	bool skips() const {
		return skip;
	}
	
	bool isSkipPeriod(const unsigned long cycleCounter, const bool doubleSpeed) const {
		return lycReg_ > 0 && time() - cycleCounter > 4U >> doubleSpeed && time() - cycleCounter < 9;
	}
};

#endif
