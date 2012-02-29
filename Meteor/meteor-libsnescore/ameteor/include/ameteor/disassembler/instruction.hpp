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

#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "arguments.hpp"

#include <string>
#include <stdint.h>

namespace AMeteor
{
	namespace Disassembler
	{
		class Instruction
		{
			public :
				Instruction ()
				{
				}

				explicit Instruction (uint32_t offset, uint32_t code)
				{
					ParseArm (offset, code);
				}

				explicit Instruction (uint32_t offset, uint16_t code)
				{
					ParseThumb(offset, code);
				}

				void Clear ();

				void ParseArm (uint32_t offset, uint32_t code);
				void ParseThumb (uint32_t offset, uint16_t code);

				const std::string& GetOperator () const
				{
					return m_operator;
				}

				std::string GetArguments () const
				{
					return m_args.GetString();
				}

				std::string ToString () const
				{
					return GetOperator() + ' ' + GetArguments();
				}

			private :
				std::string m_operator;
				Arguments m_args;

				void ParseArmDataProc (uint32_t code);
				void ParseArmCondition (uint32_t code);
		};
	}
}

#endif
