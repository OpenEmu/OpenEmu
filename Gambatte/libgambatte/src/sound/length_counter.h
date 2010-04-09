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
#ifndef LENGTH_COUNTER_H
#define LENGTH_COUNTER_H

#include "sound_unit.h"
#include "../savestate.h"

class MasterDisabler;

class LengthCounter : public SoundUnit {
	MasterDisabler &disableMaster;
	unsigned short lengthCounter;
	const unsigned char lengthMask;
	bool cgb;

public:
	LengthCounter(MasterDisabler &disabler, unsigned lengthMask);
	void event();
	void nr1Change(unsigned newNr1, unsigned nr4, unsigned long cc);
	void nr4Change(unsigned oldNr4, unsigned newNr4, unsigned long cc);
// 	void reset();
	void init(bool cgb);
	void saveState(SaveState::SPU::LCounter &lstate) const;
	void loadState(const SaveState::SPU::LCounter &lstate, unsigned long cc);
};

#endif
