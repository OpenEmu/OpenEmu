// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __ARG_MUL_REGISTERS_H__
#define __ARG_MUL_REGISTERS_H__

#include "argument.hpp"

#include <stdint.h>
#include <vector>

namespace AMeteor
{
	namespace Disassembler
	{
		enum SpecialRegister
		{
			SPREG_NONE = 0,
			SPREG_LR = 1,
			SPREG_PC = 2
		};

		class ArgMulRegisters : public Argument
		{
			public :
				ArgMulRegisters (bool forceuser) :
					m_lastreg(SPREG_NONE),
					m_forceuser(forceuser)
				{ }

				Argument* Clone () const;

				void AddRegister(uint8_t reg)
				{
					m_regs.push_back(reg);
				}
				void AddLastRegister(SpecialRegister reg)
				{
					m_lastreg = reg;
				}

				std::string GetString () const;

			private :
				typedef std::vector<uint8_t> Registers;

				Registers m_regs;
				SpecialRegister m_lastreg;
				bool m_forceuser;
		};
	}
}

#endif
