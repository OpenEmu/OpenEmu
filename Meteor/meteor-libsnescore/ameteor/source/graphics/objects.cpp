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

#include "ameteor/graphics/objects.hpp"
#include "../debug.hpp"
#include <string.h>

namespace AMeteor
{
	namespace Graphics
	{
		Objects::Objects (Memory& memory, Io& io, uint16_t* pPalette) :
			m_io(io),
			m_objs(128, Object(pPalette,
						memory.GetRealAddress(0x06010000))),
			m_pOam((uint16_t*)memory.GetRealAddress(0x07000000))
		{
		}

		void Objects::DrawLine (uint8_t line, uint32_t* surface)
		{
			bool oneDim = m_io.DRead16(Io::DISPCNT) & (0x1 << 6);
			uint8_t mosaic = m_io.DRead8(Io::MOSAIC+1);
			int16_t rotSel;
			for (Objs::iterator iter = m_objs.begin();
					iter != m_objs.end(); ++iter)
			{
				rotSel = iter->GetRotationParam();
				if (rotSel == -1)
					iter->DrawLine (line, surface, oneDim, mosaic);
				else
				{
					rotSel *= 16;
					iter->DrawLineRot (line, surface, oneDim, m_pOam[rotSel + 3],
							m_pOam[rotSel + 7], m_pOam[rotSel + 11], m_pOam[rotSel + 15],
							mosaic);
				}
			}
		}

		void Objects::DrawLineHighOnly (uint8_t line, uint32_t* surface)
		{
			bool oneDim = m_io.DRead16(Io::DISPCNT) & (0x1 << 6);
			uint8_t mosaic = m_io.DRead8(Io::MOSAIC+1);
			int16_t rotSel;
			for (Objs::iterator iter = m_objs.begin();
					iter != m_objs.end(); ++iter)
				if (iter->GetTileNum() >= 512)
				{
					rotSel = iter->GetRotationParam();
					if (rotSel == -1)
						iter->DrawLine (line, surface, oneDim, mosaic);
					else
					{
						rotSel *= 16;
						iter->DrawLineRot (line, surface, oneDim, m_pOam[rotSel + 3],
								m_pOam[rotSel + 7], m_pOam[rotSel + 11], m_pOam[rotSel + 15],
								mosaic);
					}
				}
		}

		void Objects::DrawWindow (uint8_t line, uint8_t* surface)
		{
			bool oneDim = m_io.DRead16(Io::DISPCNT) & (0x1 << 6);
			int16_t rotSel;
			uint8_t mask = (m_io.DRead16(Io::WINOUT) >> 8) & 0x3F;
			for (Objs::iterator iter = m_objs.begin(); iter != m_objs.end(); ++iter)
				if (iter->IsWindow())
				{
					rotSel = iter->GetRotationParam();
					if (rotSel == -1)
						iter->DrawWindow (line, surface, oneDim, mask);
					else
					{
						rotSel *= 16;
						iter->DrawWindowRot (line, surface, oneDim, m_pOam[rotSel + 3],
								m_pOam[rotSel + 7], m_pOam[rotSel + 11], m_pOam[rotSel + 15],
								mask);
					}
				}
		}

		void Objects::OamWrite (uint32_t begin, uint32_t end)
		{
			uint32_t objnum;
			// FIXME is this possible ?
			if (begin & 0x3)
				met_abort("OamWrite not 4 byte aligned");
			if (begin <= 0x07000000)
				objnum = 0;
			else
				objnum = (begin - 0x07000000);
			uint16_t* pOam = m_pOam + objnum/2;
			objnum /= 8;

			Objs::iterator iterStart;
			iterStart = m_objs.begin() + objnum;
			Objs::iterator iterEnd = m_objs.begin() + (end - 0x07000000 + 7)/8;
			if (iterEnd > m_objs.end())
				iterEnd = m_objs.end();

			for (Objs::iterator iter = iterStart; iter != iterEnd; ++iter, ++objnum)
			{
				iter->UpdateAttrs(pOam[0], pOam[1], pOam[2]);
				pOam += 4;
			}
		}

		void Objects::OamWrite16 (uint32_t add)
		{
			uint16_t objnum = (add - 0x07000000) / 8;
			uint16_t* pOam = m_pOam + objnum*4;
			Objs::iterator iter = m_objs.begin() + objnum;
			switch ((add - 0x07000000) % 8)
			{
				case 0:
					iter->UpdateAttr0(pOam[0]);
					break;
				case 2:
					iter->UpdateAttr1(pOam[1]);
					break;
				case 4:
					iter->UpdateAttr2(pOam[2]);
					break;
				case 6:
					break;
				default :
					met_abort("Oam access not 16 bits aligned");
					break;
			}
		}

		void Objects::OamWrite32 (uint32_t add)
		{
			add -= 0x07000000;
			uint16_t objnum = add / 8;
			uint16_t* pOam = m_pOam + objnum * 4;
			Objs::iterator iter = m_objs.begin() + objnum;
			switch (add % 8)
			{
				case 0:
					iter->UpdateAttr0(pOam[0]);
					iter->UpdateAttr1(pOam[1]);
					break;
				case 4:
					iter->UpdateAttr2(pOam[2]);
					break;
				default :
					met_abort("Oam access not 32 bits aligned");
					break;
			}
		}
	}
}
