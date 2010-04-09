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
#include "mode0_irq.h"

#include "ly_counter.h"
#include "lyc_irq.h"
#include "m3_extra_cycles.h"

Mode0Irq::Mode0Irq(const LyCounter &lyCounter_in, const LycIrq &lycIrq_in, 
                   const M3ExtraCycles &m3ExtraCycles_in, unsigned char &ifReg_in) :
	VideoEvent(0),
	lyCounter(lyCounter_in),
	lycIrq(lycIrq_in),
	m3ExtraCycles(m3ExtraCycles_in),
	ifReg(ifReg_in)
{
}

static unsigned baseCycle(const bool ds) {
	return 80 + 169 + ds * 3 + 1 - ds;
}

void Mode0Irq::doEvent() {
	if (lycIrq.time() == DISABLED_TIME || lyCounter.ly() != lycIrq.lycReg())
		ifReg |= 2;
	
	unsigned long nextTime = lyCounter.time();
	unsigned nextLy = lyCounter.ly() + 1;
	
	if (nextLy == 144) {
		nextLy = 0;
		nextTime += lyCounter.lineTime() * 10;
	}
	
	nextTime += (baseCycle(lyCounter.isDoubleSpeed()) + m3ExtraCycles(nextLy)) << lyCounter.isDoubleSpeed();
	
	setTime(nextTime);
}

void Mode0Irq::mode3CyclesChange() {
	unsigned long nextTime = lyCounter.time() - lyCounter.lineTime();
	unsigned nextLy = lyCounter.ly();
	
	if (time() > lyCounter.time()) {
		nextTime += lyCounter.lineTime();
		++nextLy;
		
		if (nextLy > 143) {
			nextTime += lyCounter.lineTime() * (154 - nextLy);
			nextLy = 0;
		}
	}
	
	nextTime += (baseCycle(lyCounter.isDoubleSpeed()) + m3ExtraCycles(nextLy)) << lyCounter.isDoubleSpeed();
	
	setTime(nextTime);
}

unsigned long Mode0Irq::schedule(const unsigned statReg, const M3ExtraCycles &m3ExtraCycles, const LyCounter &lyCounter, const unsigned long cycleCounter) {
	if (!(statReg & 0x08))
		return DISABLED_TIME;
	
	unsigned line = lyCounter.ly();
	int next = static_cast<int>(baseCycle(lyCounter.isDoubleSpeed())) - static_cast<int>(lyCounter.lineCycles(cycleCounter));
	
	if (line < 144 && next + static_cast<int>(m3ExtraCycles(line)) <= 0) {
		next += 456;
		++line;
	}
	
	if (line > 143) {
		next += (154 - line) * 456;
		line = 0;
	}
	
	next += m3ExtraCycles(line);
	
	return cycleCounter + (static_cast<unsigned>(next) << lyCounter.isDoubleSpeed());
}
