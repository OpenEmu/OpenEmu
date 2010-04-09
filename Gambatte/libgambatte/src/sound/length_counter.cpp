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
#include "length_counter.h"
#include "master_disabler.h"
#include <algorithm>

LengthCounter::LengthCounter(MasterDisabler &disabler, const unsigned mask) :
	disableMaster(disabler),
	lengthMask(mask)
{
	init(false);
	nr1Change(0, 0, 0);
}

void LengthCounter::event() {
	counter = COUNTER_DISABLED;
	lengthCounter = 0;
	disableMaster();
}

void LengthCounter::nr1Change(const unsigned newNr1, const unsigned nr4, const unsigned long cycleCounter) {
	lengthCounter = (~newNr1 & lengthMask) + 1;
	counter = (nr4 & 0x40) ?( (cycleCounter >> 13) + lengthCounter) << 13 : static_cast<unsigned long>(COUNTER_DISABLED);
}

void LengthCounter::nr4Change(const unsigned oldNr4, const unsigned newNr4, const unsigned long cycleCounter) {
	if (counter != COUNTER_DISABLED)
		lengthCounter = (counter >> 13) - (cycleCounter >> 13);
	
	{
		unsigned dec = 0;
		
		if (newNr4 & 0x40) {
			dec = ~cycleCounter >> 12 & 1;
			
			if (!(oldNr4 & 0x40) && lengthCounter) {
				if (!(lengthCounter -= dec))
					disableMaster();
			}
		}
		
		if ((newNr4 & 0x80) && !lengthCounter)
			lengthCounter = lengthMask + 1 - dec;
	}
	
	if ((newNr4 & 0x40) && lengthCounter)
		counter = ((cycleCounter >> 13) + lengthCounter) << 13;
	else
		counter = COUNTER_DISABLED;
}

/*void LengthCounter::reset() {
	counter = COUNTER_DISABLED;
	
	if (cgb)
		lengthCounter = lengthMask + 1;
}*/

void LengthCounter::init(const bool cgb) {
	this->cgb = cgb;
}

void LengthCounter::saveState(SaveState::SPU::LCounter &lstate) const {
	lstate.counter = counter;
	lstate.lengthCounter = lengthCounter;
}

void LengthCounter::loadState(const SaveState::SPU::LCounter &lstate, const unsigned long cc) {
	counter = std::max(lstate.counter, cc);
	lengthCounter = lstate.lengthCounter;
}
