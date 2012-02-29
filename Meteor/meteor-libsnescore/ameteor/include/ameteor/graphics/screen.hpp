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

#ifndef __GRAPHICS_SCREEN_H__
#define __GRAPHICS_SCREEN_H__

#include "bglayer.hpp"
#include "objects.hpp"
#include "renderer.hpp"

#include <stdint.h>
#include <istream>
#include <ostream>

namespace AMeteor
{
	namespace Graphics
	{
		class Screen
		{
			public :
				static const uint8_t WIDTH = 240;
				static const uint8_t HEIGHT = 160;

				// we skip x frames every FRMSKIP_TOTAL frames
				static const uint8_t FRMSKIP_TOTAL = 10;

				Screen (Memory& memory, Io& io);
				~Screen ();

				Renderer& GetRenderer()
				{
					return m_renderer;
				}
				const Renderer& GetRenderer() const
				{
					return m_renderer;
				}

				const uint16_t* GetSurface () const
				{
					return m_surface;
				}

				void SetFrameskip (uint8_t skip)
				{
					m_frameskip = skip;
					m_curframe = 0;
				}

				void DrawLine (uint8_t line);

				void UpdateDispCnt (uint16_t dispcnt)
				{
					m_dispcnt = dispcnt;
				}

#define UPDATE_BG_CNT(num) \
	void UpdateBg##num##Cnt (uint16_t cnt) \
	{ \
		m_bgLayer##num.UpdateCnt(cnt); \
	}
				UPDATE_BG_CNT(0)
				UPDATE_BG_CNT(1)
				UPDATE_BG_CNT(2)
				UPDATE_BG_CNT(3)
#undef UPDATE_BG_CNT

#define UPDATE_BG_OFF(num, coord) \
	void UpdateBg##num##coord##Off (uint16_t off) \
	{ \
		m_bgLayer##num.Update##coord##Off(off); \
	}
				UPDATE_BG_OFF(0, X)
				UPDATE_BG_OFF(0, Y)
				UPDATE_BG_OFF(1, X)
				UPDATE_BG_OFF(1, Y)
				UPDATE_BG_OFF(2, X)
				UPDATE_BG_OFF(2, Y)
				UPDATE_BG_OFF(3, X)
				UPDATE_BG_OFF(3, Y)
#undef UPDATE_BG_OFF

#define UPDATE_BG_REF(num, coord) \
	void UpdateBg##num##Ref##coord(int32_t x) \
	{ \
		m_ref##coord##num = (x & (0x1 << 27)) ? x | 0xF0000000 : x & 0x07FFFFFF; \
	}
				UPDATE_BG_REF(2, X)
				UPDATE_BG_REF(2, Y)
				UPDATE_BG_REF(3, X)
				UPDATE_BG_REF(3, Y)
#undef UPDATE_BG_REF

				void OamWrite (uint32_t begin, uint32_t end)
				{
					m_objs.OamWrite (begin, end);
				}
				void OamWrite16 (uint32_t add)
				{
					m_objs.OamWrite16 (add);
				}
				void OamWrite32 (uint32_t add)
				{
					m_objs.OamWrite32 (add);
				}

				bool SaveState (std::ostream& stream);
				bool LoadState (std::istream& stream);

			private :
				Io& m_io;

				uint16_t* m_surface;

				Renderer m_renderer;

				uint8_t m_frameskip, m_curframe;

				uint16_t m_dispcnt;
				int32_t m_refX2, m_refY2, m_refX3, m_refY3;
				uint16_t* m_pPalette;

				// FIXME is this REALLY useful ?
				static BgLayer Screen::* const BgLayers [4];
				BgLayer m_bgLayer0, m_bgLayer1, m_bgLayer2, m_bgLayer3;
				Objects m_objs;

				void DrawWindow (uint8_t line, uint8_t* surface,
						uint16_t win0v, uint16_t win0h, uint8_t mask);
		};
	}
}

#endif
