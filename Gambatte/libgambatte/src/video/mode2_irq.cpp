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
#include "mode2_irq.h"

#include "ly_counter.h"
#include "lyc_irq.h"

Mode2Irq::Mode2Irq(const LyCounter &lyCounter_in, const LycIrq &lycIrq_in,
                   unsigned char &ifReg_in) :
	VideoEvent(0),
	lyCounter(lyCounter_in),
	lycIrq(lycIrq_in),
	ifReg(ifReg_in)
{
}

void Mode2Irq::doEvent() {
	const unsigned ly = lyCounter.time() - time() < 8 ? (lyCounter.ly() == 153 ? 0 : lyCounter.ly() + 1) : lyCounter.ly();
	
	if (lycIrq.time() == DISABLED_TIME || (lycIrq.lycReg() != 0 && ly != (lycIrq.lycReg() + 1U)) || (lycIrq.lycReg() == 0 && ly > 1))
		ifReg |= 2;
	
	setTime(time() + lyCounter.lineTime());
	
	if (ly == 0)
		setTime(time() - 4);
	else if (ly == 143)
		setTime(time() + lyCounter.lineTime() * 10 + 4);
}

unsigned long Mode2Irq::schedule(const unsigned statReg, const LyCounter &lyCounter, const unsigned long cycleCounter) {
	if ((statReg & 0x28) != 0x20)
		return DISABLED_TIME;
	
	unsigned next = lyCounter.time() - cycleCounter;
	
	if (lyCounter.ly() >= 143 || (lyCounter.ly() == 142 && next <= 4)) {
		next += (153u - lyCounter.ly()) * lyCounter.lineTime();
	} else {
		if (next <= 4)
			next += lyCounter.lineTime();
		
		next -= 4;
	}
	
	return cycleCounter + next;
}
