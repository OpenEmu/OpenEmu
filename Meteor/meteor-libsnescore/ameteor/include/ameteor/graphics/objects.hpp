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

#ifndef __GRAPHICS_OBJECTS_H__
#define __GRAPHICS_OBJECTS_H__

#include "object.hpp"
#include "memory.hpp"
#include "io.hpp"

#include <vector>

namespace AMeteor
{
	namespace Graphics
	{
		class Objects
		{
			public :
				Objects (Memory& memory, Io& io, uint16_t* pPalette);

				void DrawLine (uint8_t line, uint32_t* surface);
				void DrawLineHighOnly (uint8_t line, uint32_t* surface);
				void DrawWindow (uint8_t line, uint8_t* surface);

				void OamWrite (uint32_t begin, uint32_t end);
				void OamWrite16 (uint32_t add);
				void OamWrite32 (uint32_t add);

			private :
				typedef std::vector<Object> Objs;

				Io& m_io;
				Objs m_objs;
				uint16_t* m_pOam;
		};
	}
}

#endif
