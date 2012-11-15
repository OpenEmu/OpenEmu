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
#ifndef SOUND_CHANNEL4_H
#define SOUND_CHANNEL4_H

#include "gbint.h"
#include "master_disabler.h"
#include "length_counter.h"
#include "envelope_unit.h"
#include "static_output_tester.h"

namespace gambatte {

struct SaveState;

class Channel4 {
	class Lfsr : public SoundUnit {
		unsigned long backupCounter;
		unsigned short reg;
		unsigned char nr3;
		bool master;
		
		void updateBackupCounter(unsigned long cc);
		
	public:
		Lfsr();
		void event();
		bool isHighState() const { return ~reg & 1; }
		void nr3Change(unsigned newNr3, unsigned long cc);
		void nr4Init(unsigned long cc);
		void reset(unsigned long cc);
		void saveState(SaveState &state, const unsigned long cc);
		void loadState(const SaveState &state);
		void resetCounters(unsigned long oldCc);
		void disableMaster() { killCounter(); master = false; reg = 0xFF; }
		void killCounter() { counter = COUNTER_DISABLED; }
		void reviveCounter(unsigned long cc);
	};
	
	class Ch4MasterDisabler : public MasterDisabler {
		Lfsr &lfsr;
	public:
		Ch4MasterDisabler(bool &m, Lfsr &lfsr) : MasterDisabler(m), lfsr(lfsr) {}
		void operator()() { MasterDisabler::operator()(); lfsr.disableMaster(); }
	};
	
	friend class StaticOutputTester<Channel4,Lfsr>;
	
	StaticOutputTester<Channel4,Lfsr> staticOutputTest;
	Ch4MasterDisabler disableMaster;
	LengthCounter lengthCounter;
	EnvelopeUnit envelopeUnit;
	Lfsr lfsr;
	
	SoundUnit *nextEventUnit;
	
	unsigned long cycleCounter;
	unsigned long soMask;
	unsigned long prevOut;
	
	unsigned char nr4;
	bool master;
	
	void setEvent();
	
public:
	Channel4();
	void setNr1(unsigned data);
	void setNr2(unsigned data);
	void setNr3(unsigned data) { lfsr.nr3Change(data, cycleCounter); /*setEvent();*/ }
	void setNr4(unsigned data);
	
	void setSo(unsigned long soMask);
	bool isActive() const { return master; }
	
	void update(uint_least32_t *buf, unsigned long soBaseVol, unsigned long cycles);
	
	void reset();
	void init(bool cgb);
	void saveState(SaveState &state);
	void loadState(const SaveState &state);
};

}

#endif
