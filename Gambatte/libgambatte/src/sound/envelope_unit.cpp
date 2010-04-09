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
#include "envelope_unit.h"
#include <algorithm>

EnvelopeUnit::VolOnOffEvent EnvelopeUnit::nullEvent;

void EnvelopeUnit::event() {
	const unsigned long period = nr2 & 7;
	
	if (period) {
		unsigned newVol = volume;
		
		if (nr2 & 8)
			++newVol;
		else
			--newVol;
		
		if (newVol < 0x10U) {
			volume = newVol;
			
			if (volume < 2)
				volOnOffEvent(counter);
			
			counter += period << 15;
		} else
			counter = COUNTER_DISABLED;
	} else
		counter += 8ul << 15;
}

bool EnvelopeUnit::nr2Change(const unsigned newNr2) {
	if (!(nr2 & 7) && counter != COUNTER_DISABLED)
		++volume;
	else if (!(nr2 & 8))
		volume += 2;
	
	if ((nr2 ^ newNr2) & 8)
		volume = 0x10 - volume;
	
	volume &= 0xF;
	
	nr2 = newNr2;
	
	return !(newNr2 & 0xF8);
}

bool EnvelopeUnit::nr4Init(const unsigned long cc) {
	{
		unsigned long period = nr2 & 7;
		
		if (!period)
			period = 8;
		
		if (!(cc & 0x7000))
			++period;
		
		counter = cc - ((cc - 0x1000) & 0x7FFF) + period * 0x8000;
	}
	
	volume = nr2 >> 4;
	
	return !(nr2 & 0xF8);
}

EnvelopeUnit::EnvelopeUnit(VolOnOffEvent &volOnOffEvent) :
volOnOffEvent(volOnOffEvent),
nr2(0),
volume(0)
{}

void EnvelopeUnit::reset() {
	counter = COUNTER_DISABLED;
}

void EnvelopeUnit::saveState(SaveState::SPU::Env &estate) const {
	estate.counter = counter;
	estate.volume = volume;
}

void EnvelopeUnit::loadState(const SaveState::SPU::Env &estate, const unsigned nr2, const unsigned long cc) {
	counter = std::max(estate.counter, cc);
	volume = estate.volume;
	this->nr2 = nr2;
}
