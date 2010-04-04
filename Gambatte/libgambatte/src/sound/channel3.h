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
#ifndef SOUND_CHANNEL3_H
#define SOUND_CHANNEL3_H

class SaveState;

#include "int.h"

#include "master_disabler.h"
#include "length_counter.h"

class Channel3 {
	class Ch3MasterDisabler : public MasterDisabler {
		unsigned long &waveCounter;
		
	public:
		Ch3MasterDisabler(bool &m, unsigned long &wC) : MasterDisabler(m), waveCounter(wC) {}
		void operator()() { MasterDisabler::operator()(); waveCounter = SoundUnit::COUNTER_DISABLED; }
	};
	
	unsigned char waveRam[0x10];
	
	Ch3MasterDisabler disableMaster;
	LengthCounter lengthCounter;
	
	unsigned long cycleCounter;
	unsigned long soMask;
	unsigned long prevOut;
	unsigned long waveCounter;
	unsigned long lastReadTime;
	
	unsigned char nr0;
	unsigned char nr3;
	unsigned char nr4;
	unsigned char wavePos;
	unsigned char rShift;
	unsigned char sampleBuf;
	
	bool master;
	bool cgb;
	
	void updateWaveCounter(unsigned long cc);
	
public:
	Channel3();
	bool isActive() const { return master; }
	void reset();
	void init(bool cgb);
	void setStatePtrs(SaveState &state);
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state);
	void setNr0(unsigned data);
	void setNr1(unsigned data) { lengthCounter.nr1Change(data, nr4, cycleCounter); }
	void setNr2(unsigned data);
	void setNr3(unsigned data) { nr3 = data; }
	void setNr4(unsigned data);
	void setSo(unsigned long soMask);
	void update(Gambatte::uint_least32_t *buf, unsigned long soBaseVol, unsigned long cycles);
	
	unsigned waveRamRead(unsigned index) const {
		if (master) {
			if (!cgb && cycleCounter != lastReadTime)
				return 0xFF;
			
			index = wavePos >> 1;
		}
		
		return waveRam[index];
	}
	
	void waveRamWrite(unsigned index, unsigned data) {
		if (master) {
			if (!cgb && cycleCounter != lastReadTime)
				return;
			
			index = wavePos >> 1;
		}
		
		waveRam[index] = data;
	}
};

#endif
