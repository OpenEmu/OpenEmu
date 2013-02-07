/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
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

namespace gambatte {

Interrupter::Interrupter(unsigned short &SP_in, unsigned short &PC_in) :
	SP(SP_in),
	PC(PC_in)
{}

unsigned long Interrupter::interrupt(const unsigned address, unsigned long cycleCounter, Memory &memory) {
	cycleCounter += 8;
	SP = (SP - 1) & 0xFFFF;
	memory.write(SP, PC >> 8, cycleCounter);
	cycleCounter += 4;
	SP = (SP - 1) & 0xFFFF;
	memory.write(SP, PC & 0xFF, cycleCounter);
	PC = address;
	cycleCounter += 8;
	
	if (address == 0x40 && !gsCodes.empty())
		applyVblankCheats(cycleCounter, memory);
	
	return cycleCounter;
}

static int asHex(const char c) {
	return c >= 'A' ? c - 'A' + 0xA : c - '0';
}

void Interrupter::setGameShark(const std::string &codes) {
	std::string code;
	gsCodes.clear();
	
	for (std::size_t pos = 0; pos < codes.length() && (code = codes.substr(pos, codes.find(';', pos) - pos), true); pos += code.length() + 1) {
		if (code.length() >= 8) {
			GsCode gs;
			gs.type  =  asHex(code[0]) << 4 | asHex(code[1]);
			gs.value = (asHex(code[2]) << 4 | asHex(code[3])) & 0xFF;
			gs.address = (asHex(code[4]) << 4 | asHex(code[5]) | asHex(code[6]) << 12 | asHex(code[7]) << 8) & 0xFFFF;
			gsCodes.push_back(gs);
		}
	}
}

void Interrupter::applyVblankCheats(const unsigned long cycleCounter, Memory &memory) {
	for (std::size_t i = 0, size = gsCodes.size(); i < size; ++i) {
		if (gsCodes[i].type == 0x01)
			memory.write(gsCodes[i].address, gsCodes[i].value, cycleCounter);
	}
}

}
