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

#include "ameteor/graphics/object.hpp"
#include "ameteor/graphics/screen.hpp"
#include "../debug.hpp"

namespace AMeteor
{
	namespace Graphics
	{
		Object::Object (uint16_t* pPalette, uint8_t* pChar) :
			m_attr0(0),
			m_attr1(0),
			m_attr2(0),
			m_width(1),
			m_height(1),
			m_pPalette(pPalette),
			m_pChar(pChar),
			m_charBegin(0x06010000),
			m_charEnd(0x06010020)
		{
		}

		Object::Object (const Object& obj) :
			m_attr0(obj.m_attr0),
			m_attr1(obj.m_attr1),
			m_attr2(obj.m_attr2),
			m_width(obj.m_width),
			m_height(obj.m_height),
			m_pPalette(obj.m_pPalette),
			m_pChar(obj.m_pChar),
			m_charBegin(obj.m_charBegin),
			m_charEnd(obj.m_charEnd)
		{
		}

		void Object::DrawLine (uint8_t line, uint32_t* surface, bool oneDim,
				uint8_t mosaic)
		{
			// if the object is disabled or it's an obj window
			if (m_attr0 & (0x1 << 9) || ((m_attr0 >> 10) & 0x3) == 2)
				return;

			int16_t yoff = (m_attr0 & 0xFF);
			if (yoff > Screen::HEIGHT)
				yoff -= 256;

			// if the object does not appear on the line
			if (yoff > line || yoff + m_height*8 <= line)
				return;

			uint8_t mosH;
			if (m_attr0 & (0x1 << 12))
			{
				uint8_t mosV = mosaic >> 4;
				mosH = mosaic & 0xF;

				++mosV;
				if (mosH) // let it be 0 if it's 0 (=> no mosaic)
					++mosH;

				line /= mosV;
				line *= mosV;
			}
			else
				mosH = 0;

			int16_t xoff = (m_attr1 & 0x1FF);
			if (xoff & (0x1 << 8))
				xoff |= 0xFE00; // extend sign bit

			uint32_t* ptr = surface + xoff;

			uint32_t prio = (((uint32_t)m_attr2) << 6) & (0x3 << 16);
			uint32_t mask = prio;
			// if semi transparent
			if (((m_attr0 >> 10) & 0x3) == 0x1)
				mask |= (0x1 << 18);
			uint8_t* pChar = m_pChar + (m_attr2 & 0x3FF) * 32;
			bool flipH = m_attr1 & (0x1 << 12);
			if (m_attr0 & (0x1 << 13)) // if 256 colors mode
			{
				// if the tile is vertically flipped
				if (m_attr1 & (0x1 << 13))
				{
					// go to the tile
					if (oneDim)
						pChar += m_width * (8 * 8) * (m_height -1 -((line-yoff) / 8));
					else
						pChar += (16 * 8 * 8) * (m_height -1 -((line-yoff) / 8));
					// advance to the right line
					pChar += 8 * (7 - ((line-yoff) % 8));
				}
				else
				{
					// go to the tile
					if (oneDim)
						pChar += m_width * (8 * 8) * ((line-yoff) / 8);
					else
						pChar += (16 * 8 * 8) * ((line-yoff) / 8);
					// advance to the right line
					pChar += 8 * ((line-yoff) % 8);
				}
				// go to the end of the line if the object is flipped
				if (flipH)
					pChar += (m_width-1) * (8*8) + 8 - 1;

				for (uint8_t j = 0; j < m_width*8; ++j, ++ptr)
				{
					// if we are on screen
					if (ptr - surface < Screen::WIDTH && ptr >= surface)
					{
						if (mosH && (ptr - surface) % mosH)
							*ptr = ptr[-1];
						else
						{
							// if there is something to draw
							if (*pChar)
							{
								// if we have priority or there is nothing behind
								if (prio < (*ptr & (0x3 << 16)) || !(*ptr & 0x8000))
									*ptr = m_pPalette[*pChar] | 0x8000 | mask;
							}
							// if we have nothing to draw
							else
								// if we have better priority
								if (prio < (*ptr & (0x3 << 16)))
									// we just modify priority and keep what was behind
									*ptr = (*ptr & ~(0x3 << 16)) | (mask & (0x3 << 16));
						}
					}

					if (flipH)
						if ((j % 8) == 7)
							// go to previous tile
							pChar -= ((8*8) - (8) + 1);
						else
							--pChar;
					else
						if ((j % 8) == 7)
							// go to next tile
							pChar += ((8*8) - (8) + 1);
						else
							++pChar;
				}
			}
			else
			{
				// if the tile is vertically flipped
				if (m_attr1 & (0x1 << 13))
				{
					// go to the tile
					if (oneDim)
						pChar += m_width * (8 * 8 / 2) * (m_height -1 -((line-yoff) / 8));
					else
						pChar += (32 * 8 * 8 / 2) * (m_height -1 -((line-yoff) / 8));
					// advance to the right line
					pChar += (8/2) * (7 - ((line-yoff) % 8));
				}
				else
				{
					// go to the tile
					if (oneDim)
						pChar += m_width * (8 * 8 / 2) * ((line-yoff) / 8);
					else
						pChar += (32 * 8 * 8 / 2) * ((line-yoff) / 8);
					// advance to the right line
					pChar += (8/2) * ((line-yoff) % 8);
				}
				// go to the end of the line if the object is flipped
				if (flipH)
					pChar += (m_width-1) * (8*8/2) + 8/2 - 1;

				uint16_t* pPalette = m_pPalette + 16 * (m_attr2 >> 12);
				uint8_t colorInd;
				for (uint8_t j = 0; j < m_width*8; ++j, ++ptr)
				{
					if (flipH)
						if (j % 2)
						{
							colorInd = *pChar & 0xF;

							if ((j % 8) == 7)
								// go to next tile
								// it doesn't matter if we are in one or two dimensions mapping
								// since we never go on the next line
								pChar -= ((8*8/2) - (8/2) + 1);
							else
								--pChar;
						}
						else
							colorInd = *pChar >> 4;
					else
						if (j % 2)
						{
							colorInd = *pChar >> 4;

							if ((j % 8) == 7)
								// go to next tile
								// it doesn't matter if we are in one or two dimensions mapping
								// since we never go on the next line
								pChar += ((8*8/2) - (8/2) + 1);
							else
								++pChar;
						}
						else
							colorInd = *pChar & 0xF;

					// if we are on screen
					if (ptr - surface < Screen::WIDTH && ptr >= surface)
					{
						if (mosH && (ptr - surface) % mosH)
							*ptr = ptr[-1];
						else
						{
							// if there is something to draw
							if (colorInd)
							{
								// if we have priority or there is nothing behind
								if (prio < (*ptr & (0x3 << 16)) || !(*ptr & 0x8000))
									*ptr = pPalette[colorInd] | 0x8000 | mask;
							}
							// if we have nothing to draw
							else
								// if we have better priority
								if (prio < (*ptr & (0x3 << 16)))
									// we just modify priority and keep what was behind
									*ptr = (*ptr & ~(0x3 << 16)) | (mask & (0x3 << 16));
						}
					}
				}
			}
		}

		void Object::DrawLineRot (uint8_t line, uint32_t* surface, bool oneDim,
				int16_t a, int16_t b, int16_t c, int16_t d, uint8_t mosaic)
		{
			// if it's an obj window
			if (((m_attr0 >> 10) & 0x3) == 2)
				return;

			int16_t yoff = (m_attr0 & 0xFF);
			if (yoff > Screen::HEIGHT)
				yoff -= 256;

			int16_t xoff = (m_attr1 & 0x1FF);
			if (xoff & (0x1 << 8))
				xoff |= 0xFE00; // extend sign bit

			uint8_t fwidth = m_width*8, fheight = m_height*8;
			if (m_attr0 & (0x1 << 9))
			{
				fwidth *= 2;
				fheight *= 2;
			}

			// if the object does not appear on the line
			if (yoff > line || yoff + fheight <= line)
				return;

			uint8_t mosH;
			if (m_attr0 & (0x1 << 12))
			{
				uint8_t mosV = mosaic >> 4;
				mosH = mosaic & 0xF;

				++mosV;
				if (mosH) // let it be 0 if it's 0 (=> no mosaic)
					++mosH;

				line /= mosV;
				line *= mosV;
			}
			else
				mosH = 0;

			int32_t curX = ((m_width*8) << 8)/2
				+ (-fwidth/2) * a + (line-yoff-fheight/2) * b;
			int32_t curY = ((m_height*8) << 8)/2
				+ (-fwidth/2) * c + (line-yoff-fheight/2) * d;
			int32_t intX, intY;

			uint32_t* ptr = surface + xoff;

			uint32_t prio = (((uint32_t)m_attr2) << 6) & (0x3 << 16);
			uint32_t mask = prio;
			// if semi transparent
			if (((m_attr0 >> 10) & 0x3) == 0x1)
				mask |= (0x1 << 18);
			uint8_t* pChar = m_pChar + (m_attr2 & 0x3FF) * 32;
			uint8_t colorInd;
			if (m_attr0 & (0x1 << 13)) // if 256 colors mode
			{
				for (uint8_t i = 0; i < fwidth; ++i)
				{
					intX = curX >> 8;
					intY = curY >> 8;

					// if we are on the object
					if (intX >= 0 && intX < m_width*8 &&
							intY >= 0 && intY < m_height*8 &&
							// and we are on screen
							ptr - surface < Screen::WIDTH && ptr >= surface)
					{
						if (mosH && (ptr - surface) % mosH)
							*ptr = ptr[-1];
						else
						{
							colorInd = pChar[
									(intY/8) * (oneDim ? m_width : 16) * 8*8
								+ (intX/8) * 8*8
								+ (intY%8) * 8
								+ (intX%8)];

							// if there is something to draw
							if (colorInd)
							{
								// if we have priority or there is nothing behind
								if (prio < (*ptr & (0x3 << 16)) || !(*ptr & 0x8000))
									*ptr = m_pPalette[colorInd] | 0x8000 | mask;
							}
							// if we have nothing to draw
							else
								// if we have better priority
								if (prio < (*ptr & (0x3 << 16)))
									// we just modify priority and keep what was behind
									*ptr = (*ptr & ~(0x3 << 16)) | (mask & (0x3 << 16));
						}
					}

					++ptr;
					curX += a;
					curY += c;
				}
			}
			else
			{
				uint16_t* pPalette = m_pPalette + 16 * (m_attr2 >> 12);
				for (uint8_t i = 0; i < fwidth; ++i)
				{
					intX = curX >> 8;
					intY = curY >> 8;

					// if we are on the object
					if (intX >= 0 && intX < m_width*8 &&
							intY >= 0 && intY < m_height*8 &&
							// and we are on screen
							ptr - surface < Screen::WIDTH && ptr >= surface)
					{
						if (mosH && (ptr - surface) % mosH)
							*ptr = ptr[-1];
						else
						{
							colorInd = pChar[(
									(intY/8) * (oneDim ? m_width : 32) * 8*8
								+ (intX/8) * 8*8
								+ (intY%8) * 8
								+ (intX%8)
								) / 2];

							if (intX % 2)
								colorInd >>= 4;
							else
								colorInd &= 0xF;

							// if there is something to draw
							if (colorInd)
							{
								// if we have priority or there is nothing behind
								if (prio < (*ptr & (0x3 << 16)) || !(*ptr & 0x8000))
									*ptr = pPalette[colorInd] | 0x8000 | mask;
							}
							// if we have nothing to draw
							else
								// if we have better priority
								if (prio < (*ptr & (0x3 << 16)))
									// we just modify priority and keep what was behind
									*ptr = (*ptr & ~(0x3 << 16)) | (mask & (0x3 << 16));
						}
					}

					++ptr;
					curX += a;
					curY += c;
				}
			}
		}

		void Object::DrawWindow (uint8_t line, uint8_t* surface, bool oneDim,
				uint8_t mask)
		{
			// if the object is disabled or it's not an obj window
			if (m_attr0 & (0x1 << 9) || ((m_attr0 >> 10) & 0x3) != 2)
				return;

			int16_t yoff = (m_attr0 & 0xFF);
			if (yoff > Screen::HEIGHT)
				yoff -= 256;

			// if the object does not appear on the line
			if (yoff > line || yoff + m_height*8 <= line)
				return;

			int16_t xoff = (m_attr1 & 0x1FF);
			if (xoff & (0x1 << 8))
				xoff |= 0xFE00; // extend sign bit

			bool flipH = m_attr1 & (0x1 << 12);
			uint8_t* ptr = surface + xoff;
			if (flipH)
				ptr += m_width * 8 - 1;
			uint8_t* pChar = m_pChar + (m_attr2 & 0x3FF) * 32;
			if (m_attr0 & (0x1 << 13)) // if 256 colors mode
			{
				// TODO 2d map, vert flips
				// we set pChar on the tile
				if (oneDim)
					pChar += m_width * (8 * 8) * ((line-yoff) / 8);
				else
					pChar += (16 * 8 * 8) * ((line-yoff) / 8);
				// and we go to the first pixel we need to draw
				pChar += 8 * ((line-yoff) % 8);
				for (uint8_t j = 0; j < m_width*8; ++j)
				{
					if (ptr - surface < Screen::WIDTH && ptr >= surface && *pChar)
						*ptr = mask;

					if (flipH)
						--ptr;
					else
						++ptr;

					if ((j % 8) == 7)
						// go to next tile
						pChar += ((8*8) - (8) + 1);
					else
						++pChar;
				}
			}
			else
			{
				// TODO verts flips
				// we set pChar on the tile
				if (oneDim)
					pChar += m_width * (8 * 8 / 2) * ((line-yoff) / 8);
				else
					pChar += (32 * 8 * 8 / 2) * ((line-yoff) / 8);
				// and we go to the first pixel we need to draw
				pChar += (8/2) * ((line-yoff) % 8);
				uint8_t colorInd;
				for (uint8_t j = 0; j < m_width*8; ++j)
				{
					if (j % 2)
					{
						colorInd = *pChar >> 4;

						if ((j % 8) == 7)
							// go to next tile
							// it doesn't matter if we are in one or two dimensions mapping
							// since we never go on the next line
							pChar += ((8*8/2) - (8/2) + 1);
						else
							++pChar;
					}
					else
						colorInd = *pChar & 0xF;

					// if we are not offscreen and there is a color
					if (ptr - surface < Screen::WIDTH && ptr >= surface && colorInd)
						*ptr = mask;

					if (flipH)
						--ptr;
					else
						++ptr;
				}
			}
		}

		void Object::DrawWindowRot (uint8_t line, uint8_t* surface,
				bool oneDim, int16_t a, int16_t b, int16_t c, int16_t d, uint8_t mask)
		{
			// if it's not an obj window
			if (((m_attr0 >> 10) & 0x3) != 2)
				return;

			int16_t yoff = (m_attr0 & 0xFF);
			if (yoff > Screen::HEIGHT)
				yoff -= 256;

			int16_t xoff = (m_attr1 & 0x1FF);
			if (xoff & (0x1 << 8))
				xoff |= 0xFE00; // extend sign bit

			uint8_t fwidth = m_width*8, fheight = m_height*8;
			if (m_attr0 & (0x1 << 9))
			{
				fwidth *= 2;
				fheight *= 2;
			}

			// if the object does not appear on the line
			if (yoff > line || yoff + fheight <= line)
				return;

			int32_t curX = ((m_width*8) << 8)/2
				+ (-fwidth/2) * a + (line-yoff-fheight/2) * b;
			int32_t curY = ((m_height*8) << 8)/2
				+ (-fwidth/2) * c + (line-yoff-fheight/2) * d;
			int32_t intX, intY;

			uint8_t* ptr = surface + xoff;
			uint8_t* pChar = m_pChar + (m_attr2 & 0x3FF) * 32;
			uint8_t colorInd;
			if (m_attr0 & (0x1 << 13)) // if 256 colors mode
			{
				for (uint8_t i = 0; i < fwidth; ++i)
				{
					intX = curX >> 8;
					intY = curY >> 8;

					if (intX >= 0 && intX < m_width*8 &&
							intY >= 0 && intY < m_height*8)
					{
						colorInd = pChar[
								(intY/8) * (oneDim ? m_width : 32) * 8*8
							+ (intX/8) * 8*8
							+ (intY%8) * 8
							+ (intX%8)];

						if (ptr - surface < Screen::WIDTH && ptr >= surface && colorInd)
							*ptr = mask;
					}

					++ptr;
					curX += a;
					curY += c;
				}
			}
			else
			{
				for (uint8_t i = 0; i < fwidth; ++i)
				{
					intX = curX >> 8;
					intY = curY >> 8;

					if (intX >= 0 && intX < m_width*8 &&
							intY >= 0 && intY < m_height*8)
					{
						colorInd = pChar[(
								(intY/8) * (oneDim ? m_width : 32) * 8*8
							+ (intX/8) * 8*8
							+ (intY%8) * 8
							+ (intX%8)
							) / 2];

						if (intX % 2)
							colorInd >>= 4;
						else
							colorInd &= 0xF;

						if (ptr - surface < Screen::WIDTH && ptr >= surface && colorInd)
							*ptr = mask;
					}

					++ptr;
					curX += a;
					curY += c;
				}
			}
		}


		void Object::UpdateAttrs (uint16_t attr0, uint16_t attr1, uint16_t attr2)
		{
			bool setsize = false, reload = false;
			if ((m_attr0 & 0xFF00) != (attr0 & 0xFF00))
			{
				reload = true;
				setsize = true;
			}
			m_attr0 = attr0;

			if ((m_attr1 & 0xF000) != (attr1 & 0xF000))
			{
				reload = true;
				setsize = true;
			}
			m_attr1 = attr1;

			if ((m_attr2 & 0xF1FF) != (attr2 & 0xF1FF))
				reload = true;
			m_attr2 = attr2;

			if (setsize)
			{
				SetSize();

				if (reload)
				{
					m_charBegin = 0x06010000 + (m_attr2 & 0x3FF)*32;
					m_charEnd = m_charBegin + m_width*m_height*8*
						((m_attr0 & (0x1 << 13)) ? 8 : 4);
				}
			}
		}

		void Object::UpdateAttr0 (uint16_t attr0)
		{
			// FIXME : we can do a more restrictive condition
			if ((m_attr0 & 0xFF00) != (attr0 & 0xFF00))
			{
				m_attr0 = attr0;
				SetSize();
				m_charEnd = m_charBegin + m_width*m_height*8*
					((m_attr0 & (0x1 << 13)) ? 8 : 4);
			}
			else
				m_attr0 = attr0;
		}

		void Object::UpdateAttr1 (uint16_t attr1)
		{
			// if the size has changed
			if ((m_attr1 & 0xC000) != (attr1 & 0xC000))
			{
				m_attr1 = attr1;
				SetSize();
				m_charEnd = m_charBegin + m_width*m_height*8*
					((m_attr0 & (0x1 << 13)) ? 8 : 4);
			}
			else
				m_attr1 = attr1;
		}

		void Object::UpdateAttr2 (uint16_t attr2)
		{
			if ((m_attr2 & 0xF1FF) != (attr2 & 0xF1FF))
			{
				m_attr2 = attr2;
				m_charBegin = 0x06010000 + (m_attr2 & 0x3FF)*32;
				m_charEnd = m_charBegin + m_width*m_height*8*
					((m_attr0 & (0x1 << 13)) ? 8 : 4);
			}
			else
				m_attr2 = attr2;
		}

		inline void Object::SetSize ()
		{
			static const uint8_t Width[3][4] = {
				{1, 2, 4, 8}, // Square
				{2, 4, 4, 8}, // Horizontal
				{1, 1, 2, 4}  // Vertical
			};
			static const uint8_t Height[3][4] = {
				{1, 2, 4, 8}, // Square
				{1, 1, 2, 4}, // Horizontal
				{2, 4, 4, 8}  // Vertical
			};

			m_width = Width[m_attr0 >> 14][m_attr1 >> 14];
			m_height = Height[m_attr0 >> 14][m_attr1 >> 14];
		}
	}
}
