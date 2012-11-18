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
#ifndef TIMA_H
#define TIMA_H

#include "interruptrequester.h"

namespace gambatte {

class TimaInterruptRequester {
	InterruptRequester &intreq;
	
public:
	explicit TimaInterruptRequester(InterruptRequester &intreq) : intreq(intreq) {}
	void flagIrq() const { intreq.flagIrq(4); }
	unsigned long nextIrqEventTime() const { return intreq.eventTime(TIMA); }
	void setNextIrqEventTime(const unsigned long time) const { intreq.setEventTime<TIMA>(time); }
};

class Tima {
	unsigned long lastUpdate_;
	unsigned long tmatime_;
	
	unsigned char tima_;
	unsigned char tma_;
	unsigned char tac_;
	
	void updateIrq(const unsigned long cc, const TimaInterruptRequester timaIrq) {
		while (cc >= timaIrq.nextIrqEventTime())
			doIrqEvent(timaIrq);
	}
	
	void updateTima(unsigned long cc);
	
public:
	Tima();
	void saveState(SaveState &) const;
	void loadState(const SaveState &, TimaInterruptRequester timaIrq);
	void resetCc(unsigned long oldCc, unsigned long newCc, TimaInterruptRequester timaIrq);
	
	void setTima(unsigned tima, unsigned long cc, TimaInterruptRequester timaIrq);
	void setTma(unsigned tma, unsigned long cc, TimaInterruptRequester timaIrq);
	void setTac(unsigned tac, unsigned long cc, TimaInterruptRequester timaIrq);
	unsigned tima(unsigned long cc);
	
	void doIrqEvent(TimaInterruptRequester timaIrq);
};

}

#endif
