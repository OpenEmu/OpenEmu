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

LycIrq::LycIrq(unsigned char &ifReg_in) :
	VideoEvent(1),
	ifReg(ifReg_in)
{
	setDoubleSpeed(false);
	setM2IrqEnabled(false);
	setLycReg(0);
	setSkip(false);
}

void LycIrq::doEvent() {
	if (!skip && (!m2IrqEnabled || lycReg_ > 143 || lycReg_ == 0))
		ifReg |= 0x2;
	
	skip = false;
	
	setTime(time() + frameTime);
}

unsigned long LycIrq::schedule(const unsigned statReg, const unsigned lycReg, const LyCounter &lyCounter, const unsigned long cycleCounter) {
	return ((statReg & 0x40) && lycReg < 154) ? lyCounter.nextFrameCycle(lycReg ? lycReg * 456 : 153 * 456 + 8, cycleCounter) : static_cast<unsigned long>(DISABLED_TIME);
}
