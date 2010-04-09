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
#ifndef SOUND_H
#define SOUND_H

class SaveState;

#include "int.h"

#include "sound/channel1.h"
#include "sound/channel2.h"
#include "sound/channel3.h"
#include "sound/channel4.h"

class PSG {
	Channel1 ch1;
	Channel2 ch2;
	Channel3 ch3;
	Channel4 ch4;
		
	Gambatte::uint_least32_t *buffer;
	
	unsigned long lastUpdate;
	unsigned long soVol;
	
	Gambatte::uint_least32_t rsum;
	
	unsigned bufferPos;
	
	bool enabled;

	void accumulate_channels(unsigned long cycles);

public:
	PSG();
	void init(bool cgb);
	void reset();
	void setStatePtrs(SaveState &state);
	void saveState(SaveState &state);
	void loadState(const SaveState &state);

	void generate_samples(unsigned long cycleCounter, unsigned doubleSpeed);
	void resetCounter(unsigned long newCc, unsigned long oldCc, unsigned doubleSpeed);
	unsigned fillBuffer();
	void setBuffer(Gambatte::uint_least32_t *const buf) { buffer = buf; bufferPos = 0; }
	
	bool isEnabled() const { return enabled; }
	void setEnabled(bool value) { enabled = value; }

	void set_nr10(unsigned data) { ch1.setNr0(data); }
	void set_nr11(unsigned data) { ch1.setNr1(data); }
	void set_nr12(unsigned data) { ch1.setNr2(data); }
	void set_nr13(unsigned data) { ch1.setNr3(data); }
	void set_nr14(unsigned data) { ch1.setNr4(data); }

	void set_nr21(unsigned data) { ch2.setNr1(data); }
	void set_nr22(unsigned data) { ch2.setNr2(data); }
	void set_nr23(unsigned data) { ch2.setNr3(data); }
	void set_nr24(unsigned data) { ch2.setNr4(data); }

	void set_nr30(unsigned data) { ch3.setNr0(data); }
	void set_nr31(unsigned data) { ch3.setNr1(data); }
	void set_nr32(unsigned data) { ch3.setNr2(data); }
	void set_nr33(unsigned data) { ch3.setNr3(data); }
	void set_nr34(unsigned data) { ch3.setNr4(data); }
	unsigned waveRamRead(unsigned index) const { return ch3.waveRamRead(index); }
	void waveRamWrite(unsigned index, unsigned data) { ch3.waveRamWrite(index, data); }

	void set_nr41(unsigned data) { ch4.setNr1(data); }
	void set_nr42(unsigned data) { ch4.setNr2(data); }
	void set_nr43(unsigned data) { ch4.setNr3(data); }
	void set_nr44(unsigned data) { ch4.setNr4(data); }

	void set_so_volume(unsigned nr50);
	void map_so(unsigned nr51);
	unsigned getStatus() const;
};

#endif
