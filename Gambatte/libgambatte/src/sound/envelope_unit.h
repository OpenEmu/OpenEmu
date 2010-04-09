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
#ifndef ENVELOPE_UNIT_H
#define ENVELOPE_UNIT_H

#include "sound_unit.h"
#include "../savestate.h"

class EnvelopeUnit : public SoundUnit {
public:
	struct VolOnOffEvent {
		virtual ~VolOnOffEvent() {}
		virtual void operator()(unsigned long /*cc*/) {}
	};
	
private:
	static VolOnOffEvent nullEvent;
	VolOnOffEvent &volOnOffEvent;
	unsigned char nr2;
	unsigned char volume;
	
public:
	EnvelopeUnit(VolOnOffEvent &volOnOffEvent = nullEvent);
	void event();
	bool dacIsOn() const { return nr2 & 0xF8; }
	unsigned getVolume() const { return volume; }
	bool nr2Change(unsigned newNr2);
	bool nr4Init(unsigned long cycleCounter);
	void reset();
	void saveState(SaveState::SPU::Env &estate) const;
	void loadState(const SaveState::SPU::Env &estate, unsigned nr2, unsigned long cc);
};

#endif
