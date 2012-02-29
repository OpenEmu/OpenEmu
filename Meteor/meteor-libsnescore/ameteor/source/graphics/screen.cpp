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

#include "ameteor/graphics/screen.hpp"
#include "../globals.hpp"
#include "../debug.hpp"
#include "ameteor.hpp"

#include <cstring>

namespace AMeteor
{
	namespace Graphics
	{
		BgLayer Screen::* const Screen::BgLayers [4] =
			{ &Screen::m_bgLayer0, &Screen::m_bgLayer1,
				&Screen::m_bgLayer2, &Screen::m_bgLayer3 };

		// TODO there is no more need to pass theses references
		Screen::Screen (Memory& memory, Io& io) :
			m_io(io),
			m_surface(new uint16_t[WIDTH*HEIGHT]),
			m_renderer(m_surface),
			m_frameskip(0),
			m_curframe(0),
			m_dispcnt(0),
			m_refX2(0), m_refY2(0), m_refX3(0), m_refY3(0),
			m_pPalette((uint16_t*)memory.GetRealAddress(0x05000000)),
			m_bgLayer0(0, memory, io, m_pPalette),
			m_bgLayer1(1, memory, io, m_pPalette),
			m_bgLayer2(2, memory, io, m_pPalette),
			m_bgLayer3(3, memory, io, m_pPalette),
			m_objs(memory, io, m_pPalette + 256)
		{
		}

		Screen::~Screen ()
		{
			delete [] m_surface;
		}

		void Screen::DrawLine (uint8_t line)
		{
			if (m_curframe < m_frameskip)
			{
				// we skip this frame
				// VBlank
				if (line == 159)
					// we don't update screen since we haven't drawn anything on it, it
					// would show up a buffer with the previous image or maybe only
					// garbage (we use double buffering)
					m_curframe = (m_curframe + 1) % FRMSKIP_TOTAL;
				return;
			}

			uint16_t* lineBg = new uint16_t[4*WIDTH];
			// layers may draw transparent pixels, so we need to clear them
			memset(lineBg, 0, 4*WIDTH*sizeof(uint16_t));
			uint32_t* lineObj = new uint32_t[WIDTH];
			for (uint32_t* p = lineObj + WIDTH - 1; p >= lineObj; --p)
				*p = 0x00030000;
			uint8_t prio[4];
			prio[0] = m_bgLayer0.GetPriority();
			prio[1] = m_bgLayer1.GetPriority();
			prio[2] = m_bgLayer2.GetPriority();
			prio[3] = m_bgLayer3.GetPriority();
			uint8_t layersOn = (m_dispcnt >> 8) & 0x1F;

			switch (m_dispcnt & 0x7)
			{
				case 0: // all in mode 0
					// if the bg is enabled draw it
					if (layersOn & (0x1     )) m_bgLayer0.DrawLine0(line, lineBg);
					if (layersOn & (0x1 << 1)) m_bgLayer1.DrawLine0(line, lineBg+WIDTH);
					if (layersOn & (0x1 << 2)) m_bgLayer2.DrawLine0(line, lineBg+2*WIDTH);
					if (layersOn & (0x1 << 3)) m_bgLayer3.DrawLine0(line, lineBg+3*WIDTH);
					// if objects are enabled draw them
					if (layersOn & (0x1 << 4)) m_objs.DrawLine(line, lineObj);
					break;
				case 1: // bg0 and bg1 in mode 0 and bg2 in mode 2, no bg3
					// disable layer 3
					layersOn &= 0xF7;
					// if the bg is enabled draw it
					if (layersOn & (0x1     )) m_bgLayer0.DrawLine0(line, lineBg);
					if (layersOn & (0x1 << 1)) m_bgLayer1.DrawLine0(line, lineBg+WIDTH);
					if (layersOn & (0x1 << 2))
						m_bgLayer2.DrawLine2(lineBg+2*WIDTH,
								m_refX2, m_refY2,
								m_io.DRead16(Io::BG2PA),
								m_io.DRead16(Io::BG2PC));
					// if objects are enabled draw them
					if (layersOn & (0x1 << 4)) m_objs.DrawLine(line, lineObj);
					break;
				case 2: // bg2 and bg3 in mode 2, no bg0 and bg1
					// disable layers 0 and 1
					layersOn &= 0xFC;
					// if the bg is enabled draw it
					if (layersOn & (0x1 << 2))
						m_bgLayer2.DrawLine2(lineBg+2*WIDTH,
								m_refX2, m_refY2,
								m_io.DRead16(Io::BG2PA),
								m_io.DRead16(Io::BG2PC));
					if (layersOn & (0x1 << 3))
						m_bgLayer3.DrawLine2(lineBg+3*WIDTH,
								m_refX3, m_refY3,
								m_io.DRead16(Io::BG3PA),
								m_io.DRead16(Io::BG3PC));
					// if objects are enabled draw them
					if (layersOn & (0x1 << 4)) m_objs.DrawLine(line, lineObj);
					break;
				// TODO (remember, HIGH ONLY for objs, don't make shitty copy paste)
				case 4: // bg2 only in mode 4 (bitmap 256)
					layersOn &= 0xF4;
					// if bg2 is enabled
					if (layersOn & (0x1 << 2))
						// draw it
						m_bgLayer2.DrawLine4(
								line,
								lineBg+2*WIDTH,
								m_refX2, m_refY2,
								m_io.DRead16(Io::BG2PA),
								m_io.DRead16(Io::BG2PB),
								m_io.DRead16(Io::BG2PC),
								m_io.DRead16(Io::BG2PD),
								m_dispcnt & (0x1 << 4));
					// if objs are enabled
					if (layersOn & (0x1 << 4))
						// all objs with the current priority
						m_objs.DrawLineHighOnly(line, lineObj);
					break;
				default :
					met_abort("not supported : " << (m_dispcnt & 0x7));
					break;
			}

			// windows
			/* I got very little information for this, it may not be accurate. All
			 * the sources don't say the same thing */
			uint8_t* window = NULL;
			if (m_dispcnt >> 13)
			{
				window = new uint8_t[WIDTH];
				// Outside window
				memset(window, m_io.DRead16(Io::WINOUT) & 0x3F, WIDTH*sizeof(uint8_t));
				// OBJ window
				if (m_dispcnt & (0x1 << 15))
					m_objs.DrawWindow(line, window);
				// Window 1
				if (m_dispcnt & (0x1 << 14))
					DrawWindow(line, window,
							m_io.DRead16(Io::WIN1V), m_io.DRead16(Io::WIN1H),
								(m_io.DRead16(Io::WININ) >> 8) & 0x3F);
				// Window 0
				if (m_dispcnt & (0x1 << 13))
					DrawWindow(line, window,
							m_io.DRead16(Io::WIN0V), m_io.DRead16(Io::WIN0H),
								m_io.DRead16(Io::WININ) & 0x3F);
			}

			// color effects
			uint16_t bldcnt = m_io.DRead16(Io::BLDCNT);
			uint8_t colorEffect = (bldcnt >> 6) & 0x3;
			uint8_t eva = std::min(m_io.DRead8(Io::BLDALPHA) & 0x1F, 16);
			uint8_t evb = std::min(m_io.DRead8(Io::BLDALPHA+1) & 0x1F, 16);
			uint8_t evy = std::min(m_io.DRead8(Io::BLDY) & 0x1F, 16);

			// blending
			uint16_t* surface = m_surface + line*WIDTH;
			uint16_t out, bout;
			// top and back are formated as follow :
			// 4 bits   | 4 bits
			// priority | layer
			uint8_t top, back;
			uint8_t curprio;
			uint32_t* pObj = lineObj;
			uint16_t* pBg = lineBg;
			uint8_t* pWin = window;
			uint8_t winmask;
			// if window are disabled, we draw everything which is enabled by
			// layersOn
			if (!window)
				winmask = 0xFF;
			for (uint8_t x = 0; x < WIDTH; ++x, ++pBg, ++pObj, ++pWin)
			{
				if (window)
					winmask = *pWin;

				// backdrop
				bout = out = m_pPalette[0];
				back = top = 0xF5;

				// for each layer
				for (uint8_t l = 0; l < 4; ++l)
					// if layer is enabled and
					if ((layersOn & (0x1 << l)) &&
							// pixel to draw is not transparent
							(pBg[l*WIDTH] & 0x8000))
					{
						curprio = ((prio[l] << 4) | l);

						if (curprio < back && curprio > top)
						{
							bout = pBg[l*WIDTH];
							back = curprio;
						}
						else if (
								// priority is lower than current top pixel and
								curprio < top &&
								// this layer should be drawn in current window
								(winmask & (0x1 << l)))
						{
							bout = out;
							out = pBg[l*WIDTH];
							back = top;
							top = curprio;
						}
					}

				// now objects
				// if objects are enabled
				if ((layersOn & (0x1 << 4)) &&
						// pixel to draw is not transparent
						(*pObj & 0x8000))
				{
					curprio = ((*pObj >> (16 - 4)) & (0x3 << 4));

					if (curprio <= (back & 0xF0) && curprio > (top & 0xF0))
					{
						bout = *pObj;
						back = curprio | 4;
					}
					else if (// priority is lower than current top pixel and
							// NOTE : objects are OVER bg with same priority
							curprio <= (top & 0xF0) &&
							// objects should be drawn in current window
							(winmask & (0x1 << 4)))
					{
						bout = out;
						out = *pObj;
						back = top;
						top = curprio | 4;
					}
				}

				// if we have an object on top and it has semi transparency
				if ((top & 0xF) == 4 && (*pObj & (0x1 << 18)))
				{
					// if second target is just behind
					if (bldcnt & ((0x1 << 8) << (back & 0xF)))
						// apply alpha blend
						out =
							std::min(((bout & 0x001F) * evb +
										(out & 0x001F) * eva) / 16, 0x001F) |
							std::min((((bout & 0x03E0) * evb +
											(out & 0x03E0) * eva) / 16) & 0xFFE0, 0x03E0) |
							// no need to take care of over flow since u16 & s32 = s32
							std::min((((bout & 0x7C00) * evb +
											(out & 0x7C00) * eva) / 16) & 0xFC00, 0x7C00);
				}
				// else if no window or window and effects are enabled in window
				// and we have a first target on top
				else if ((!window || (*pWin & (0x1 << 5)))
						&& (bldcnt & (0x1 << (top & 0xF))))
					switch (colorEffect)
					{
						case 1: // alpha blend
							// if second target is just behind
							// TODO optimization : special cases for eva = 0 or evb = 0
							if (bldcnt & ((0x1 << 8) << (back & 0xF)))
								// apply alpha blend
								out =
									std::min(((bout & 0x001F) * evb +
												(out & 0x001F) * eva) / 16, 0x001F) |
									std::min((((bout & 0x03E0) * evb +
													(out & 0x03E0) * eva) / 16) & 0xFFE0, 0x03E0) |
									// no need to take care of over flow since u16 & s32 = s32
									std::min((((bout & 0x7C00) * evb +
													(out & 0x7C00) * eva) / 16) & 0xFC00, 0x7C00);
							break;
						case 2: // brightness increase
							// we don't need saturation since the formula makes it so it never
							// goes above 0x1F
							out =
								(((out & 0x001F) +
									((0x001F - (out & 0x001F)) * evy) / 16) & 0x001F) |
								(((out & 0x03E0) +
									((0x03E0 - (out & 0x03E0)) * evy) / 16) & 0x03E0) |
								(((out & 0x7C00) +
									((0x7C00 - (out & 0x7C00)) * evy) / 16) & 0x7C00);
							break;
						case 3: // brightness decrease
							// we don't need saturation since the formula makes it so it never
							// goes below 0
							out =
								((((out & 0x001F) * (16-evy)) / 16) & 0x001F) |
								((((out & 0x03E0) * (16-evy)) / 16) & 0x03E0) |
								((((out & 0x7C00) * (16-evy)) / 16) & 0x7C00);
							break;
					}

				*surface = out;
				++surface;
			}

			m_refX2 += (int16_t)m_io.DRead16(Io::BG2PB);
			m_refY2 += (int16_t)m_io.DRead16(Io::BG2PD);
			m_refX3 += (int16_t)m_io.DRead16(Io::BG3PB);
			m_refY3 += (int16_t)m_io.DRead16(Io::BG3PD);

			if (window)
				delete [] window;
			delete [] lineBg;
			delete [] lineObj;

			// VBlank
			if (line == 159)
			{
				m_curframe = (m_curframe + 1) % FRMSKIP_TOTAL;
				m_renderer.VBlank();
			}
		}

		void Screen::DrawWindow (uint8_t line, uint8_t* surface,
				uint16_t win0v, uint16_t win0h, uint8_t mask)
		{
			// the variables are called win0, but this function works also for win1
			uint8_t win0t = win0v >> 8, win0b = win0v & 0xFF;
			// VBA says that if t == b and they are greater than 228, we are in the
			// window
			// This is from Tonc documentation
			if (win0t >= 227)
				return;
			else if (win0b > win0t && line >= win0t && line < win0b
					// the above is the normal behaviour
					|| win0b < win0t && (line >= win0t || line < win0b)
					// the above is the "inverted" behaviour
					)
			{
				uint8_t win0l, win0r;
				uint8_t* ptr;
				win0l = win0h >> 8;
				win0r = win0h & 0xFF;
				// this seems wrong
				//if (win0l > 240)
				//	win0l = 240;
				//if (win0r > 240)
				//	win0r = 240;

				// if this is the normal behaviour
				if (win0l <= win0r)
				{
					ptr = surface + win0l;
					for (uint8_t i = win0l; i < win0r && i < 240; ++i, ++ptr)
						*ptr = mask;
				}
				// else, this is the inverted behaviour
				else
				{
					ptr = surface;
					for (uint8_t i = 0; i < win0r && i < 240; ++i, ++ptr)
						*ptr = mask;
					ptr = surface + win0l;
					for (uint8_t i = win0l; i < 240; ++i, ++ptr)
						*ptr = mask;
				}
			}
		}

		bool Screen::SaveState (std::ostream& stream)
		{
			SS_WRITE_VAR(m_refX2);
			SS_WRITE_VAR(m_refY2);
			SS_WRITE_VAR(m_refX3);
			SS_WRITE_VAR(m_refY3);

			return true;
		}

		bool Screen::LoadState (std::istream& stream)
		{
			SS_READ_VAR(m_refX2);
			SS_READ_VAR(m_refY2);
			SS_READ_VAR(m_refX3);
			SS_READ_VAR(m_refY3);

			return true;
		}
	}
}
