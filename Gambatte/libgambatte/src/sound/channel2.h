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
#ifndef SOUND_CHANNEL2_H
#define SOUND_CHANNEL2_H

class SaveState;

#include "int.h"

#include "length_counter.h"
#include "duty_unit.h"
#include "envelope_unit.h"
#include "static_output_tester.h"

class Channel2 {
	friend class StaticOutputTester<Channel2,DutyUnit>;
	
	StaticOutputTester<Channel2,DutyUnit> staticOutputTest;
	DutyMasterDisabler disableMaster;
	LengthCounter lengthCounter;
	DutyUnit dutyUnit;
	EnvelopeUnit envelopeUnit;
	
	SoundUnit *nextEventUnit;
	
	unsigned long cycleCounter;
	unsigned long soMask;
	unsigned long prevOut;
	
	unsigned char nr4;
	bool master;
	
	void setEvent();
	
public:
	Channel2();
	void setNr1(unsigned data);
	void setNr2(unsigned data);
	void setNr3(unsigned data);
	void setNr4(unsigned data);
	
	void setSo(unsigned long soMask);
	// void deactivate() { disableMaster(); setEvent(); }
	bool isActive() const { return master; }
	
	void update(Gambatte::uint_least32_t *buf, unsigned long soBaseVol, unsigned long cycles);
	
	void reset();
	void init(bool cgb);
	void saveState(SaveState &state);
	void loadState(const SaveState &state);
};

#endif
