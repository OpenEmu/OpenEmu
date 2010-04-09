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
#include "interrupter.h"

#include "memory.h"

Interrupter::Interrupter(unsigned short &SP_in, unsigned short &PC_in, bool &halted_in) :
	SP(SP_in),
	PC(PC_in),
	halted(halted_in)
{}

unsigned long Interrupter::interrupt(const unsigned address, unsigned long cycleCounter, Memory &memory) {
	if (halted && memory.isCgb())
		cycleCounter += 4;
	
	halted = false;
	cycleCounter += 8;
	SP = (SP - 1) & 0xFFFF;
	memory.write(SP, PC >> 8, cycleCounter);
	cycleCounter += 4;
	SP = (SP - 1) & 0xFFFF;
	memory.write(SP, PC & 0xFF, cycleCounter);
	PC = address;
	cycleCounter += 8;
	
	return cycleCounter;
}
