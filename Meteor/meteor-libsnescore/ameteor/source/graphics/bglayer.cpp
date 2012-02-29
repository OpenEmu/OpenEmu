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

#include "ameteor/graphics/bglayer.hpp"
#include "ameteor/graphics/screen.hpp"
#include "../debug.hpp"

namespace AMeteor
{
	namespace Graphics
	{
		BgLayer::BgLayer (int8_t num, Memory& memory, Io& io, uint16_t* pPalette) :
			m_memory(memory),
			m_io(io),
			m_num(num),
			m_priority(0),
			m_cnt(0),
			m_xoff(0),
			m_yoff(0),
			m_tWidth(32),
			m_tHeight(32),
			m_rWidth(16),
			m_rHeight(16),
			m_mapAdd(0x06000000),
			m_charAdd(0x06000000),
			m_pPalette(pPalette)
		{
		}

		BgLayer::~BgLayer ()
		{
		}

		void BgLayer::DrawLine0 (uint8_t line, uint16_t* ptr)
		{
			uint8_t mosH;
			if (m_cnt & (0x1 << 6))
			{
				mosH = m_io.DRead8(Io::MOSAIC);
				uint8_t mosV = mosH >> 4;
				mosH &= 0x0F;

				++mosV;
				if (mosH) // let it be 0 if it's 0 (=> no mosaic)
					++mosH;

				line /= mosV;
				line *= mosV;
			}
			else
				mosH = 0;

			uint16_t* pMap = (uint16_t*)m_memory.GetRealAddress(m_mapAdd), *pTile;
			uint8_t* pChar = m_memory.GetRealAddress(m_charAdd), *tpChar;
			uint8_t i = 0;

			// theses are the coordinates on the layer from which we will draw a line
			uint16_t xoff = m_xoff % (m_tWidth * 8);
			uint16_t yoff = (m_yoff + line) % (m_tHeight * 8);

			// theses are the coordinates on the tile of the pixels we are drawing
			// NOTE : if the tile is horizontally flipped tileX = 8 - tileX, thus
			// when we draw it is ALWAYS incremented no matter how the tile is
			// flipped
			// NOTE : tileY is NOT redefined if the tile is flipped vertically
			int8_t tileX = xoff % 8, tileY;
			bool flipH;

			// sets pTile to the tile we must draw first
			// +---+---+
			// | 1 | 2 |
			// +---+---+
			// | 3 | 4 |
			// +---+---+
			if (yoff >= 256)
				if (m_tWidth == 64)
					if (xoff >= 256)
						// zone 4
						pTile = pMap + 32*32 + 32*32 + 32*32
							+ 32 * ((yoff-256)/8) + (xoff-256)/8;
					else
						// zone 3 (large map)
						pTile = pMap + 32*32 + 32*32 + 32 * ((yoff-256)/8) + xoff/8;
				else
					// zone 3 (no large map)
					pTile = pMap + 32*32 + 32 * ((yoff-256)/8) + xoff/8;
			else
				if (xoff >= 256)
					// zone 2
					pTile = pMap + 32*32 + 32 * (yoff/8) + (xoff-256)/8;
				else
					// zone 1
					pTile = pMap + 32 * (yoff/8) + xoff/8;

			if (m_hicolor)
			{
				while (i < 240)
				{
					flipH = (bool)(*pTile & (0x1 << 10));

					// if the tile is vertically fliped
					if (*pTile & (0x1 << 11))
						tileY = 7 - (yoff % 8);
					else
						tileY = yoff % 8;

					if (flipH)
						tpChar = pChar + (*pTile & 0x3FF)*8*8 + tileY*8 + 7 - tileX;
					else
						tpChar = pChar + (*pTile & 0x3FF)*8*8 + tileY*8 + tileX;

					while (tileX < 8)
					{
						if (mosH && i % mosH)
							*ptr = ptr[-1];
						else
						{
							if (*tpChar)
								*ptr = m_pPalette[*tpChar] | 0x8000;
							else
								*ptr = 0x0;

							if (flipH)
								--tpChar;
							else
								++tpChar;
						}

						++ptr;
						++tileX;
						++i;

						if (i == 240)
							return;
					}
					tileX = 0;
					++pTile;

					// support for big maps and wrap around
					if (!((pTile - pMap) % 32))
						if (m_tWidth == 32)
							pTile -= 32;
						else
							if (yoff >= 256 && pTile - pMap > 32*32 * 3
									|| yoff < 256 && pTile - pMap > 32*32)
								pTile -= 32*33;
							else
								pTile += 32*31;
				}
			}
			else
			{
				uint16_t* pPalette;
				uint8_t colorInd;

				// for each pixel of the line we are drawing
				while (i < 240)
				{
					pPalette = m_pPalette + ((*pTile >> 12) & 0xF) * 16;
					flipH = (bool)(*pTile & (0x1 << 10));

					// if the tile is vertically fliped
					if (*pTile & (0x1 << 11))
						tileY = 7 - (yoff % 8);
					else
						tileY = yoff % 8;

					if (flipH)
						tpChar = pChar + ((*pTile & 0x3FF)*8*8 + tileY*8 + 7 - tileX)/2;
					else
						tpChar = pChar + ((*pTile & 0x3FF)*8*8 + tileY*8 + tileX)/2;

					// we draw until the end of the tile or the line
					while (tileX < 8)
					{
						if (mosH && i % mosH)
							*ptr = ptr[-1];
						else
						{
							if (flipH)
								if (tileX % 2)
								{
									colorInd = *tpChar & 0xF;
									--tpChar;
								}
								else
									colorInd = *tpChar >> 4;
							else
								if (tileX % 2)
								{
									colorInd = *tpChar >> 4;
									++tpChar;
								}
								else
									colorInd = *tpChar & 0xF;

							if (colorInd)
								*ptr = pPalette[colorInd] | 0x8000;
							else
								*ptr = 0x0;
						}

						++ptr;
						++tileX;
						++i;

						// if this was the last pixel of the line
						if (i == 240)
							return;
					}

					// we have finished drawing a tile, so we go to the next tile
					tileX = 0;
					++pTile;

					// support for big maps and wrap around
					if (!((pTile - pMap) % 32))
						if (m_tWidth == 32)
							pTile -= 32;
						else
							if (yoff >= 256 && pTile - pMap > 32*32 * 3
									|| yoff < 256 && pTile - pMap > 32*32)
								pTile -= 32*33;
							else
								pTile += 32*31;
				}
			}
		}

		void BgLayer::DrawLine2 (uint16_t* ptr,
				int32_t curX, int32_t curY,
				int16_t dx, int16_t dy)
		{
			if (!m_hicolor)
			{
				//FIXME is this possible ??
				//this seems to be impossible, but we should not crash since some games
				//do it
				//puts("rotated layer with 16 colors");
				//abort();
				// XXX
				//it seems now that we should not care about this flag, we draw in 256
				//colors, that's all
				//return;
			}
			int32_t intX, intY;

			uint16_t colorInd;

			uint8_t* pMap = (uint8_t*)m_memory.GetRealAddress(m_mapAdd);
			uint8_t* pChar = m_memory.GetRealAddress(m_charAdd);

			for (uint8_t x = 0; x < 240; ++x, ++ptr, curX += dx, curY += dy)
			{
				intX = curX >> 8;
				intY = curY >> 8;

				// if we are off layer
				if (intX < 0 || intX >= m_rWidth*8)
					if (m_cnt & (0x1 << 13))
					{
						// NOTE : in C++, the modulus can be negative, this is because in
						// x86, the IDIV instruction gives a remainder of the same sign of
						// the dividend
						curX %= m_rWidth*8 << 8;
						if (curX < 0)
							curX += m_rWidth*8 << 8;
						intX = curX >> 8;
					}
					else
						continue;
				if (intY < 0 || intY >= m_rHeight*8)
					if (m_cnt & (0x1 << 13))
					{
						curY %= m_rHeight*8 << 8;
						if (curY < 0)
							curY += m_rHeight*8 << 8;
						intY = curY >> 8;
					}
					else
						continue;

				colorInd = pChar[pMap[intY / 8 * m_rWidth + intX / 8] * 8 * 8
					+ (intY % 8) * 8 + intX % 8];

				if (colorInd)
					*ptr = m_pPalette[colorInd] | 0x8000;
				else
					*ptr = 0x0;
			}
		}

		void BgLayer::DrawLine3 (uint16_t* ptr,
				int32_t curX, int32_t curY,
				int16_t dx, int16_t dy)
		{
			int32_t intX, intY;

			uint8_t* pChar = m_memory.GetRealAddress(0x06000000);

			for (uint8_t x = 0; x < 240; ++x, ++ptr, curX += dx, curY += dy)
			{
				intX = curX >> 8;
				intY = curY >> 8;

				// if we are off layer
				if (intX < 0 || intX >= 240)
					if (m_cnt & (0x1 << 13))
					{
						// NOTE : in C++, the modulus can be negative
						intX %= 240;
						if (intX < 0)
							intX += 240;
					}
					else
						continue;
				if (intY < 0 || intY >= 160)
					if (m_cnt & (0x1 << 13))
					{
						intY %= 160;
						if (intY < 0)
							intY += 160;
					}
					else
						continue;

				*ptr = pChar[intY * 240 * 2 + intX * 2] | 0x8000;
			}
		}

		void BgLayer::DrawLine4 (uint8_t line, uint16_t* ptr,
				int32_t curX, int32_t curY,
				int16_t dx, int16_t dmx, int16_t dy, int16_t dmy, bool frame1)
		{
			uint8_t mosH;
			if (m_cnt & (0x1 << 6))
			{
				// TODO horizontal mosaic not implemented
				mosH = m_io.DRead8(Io::MOSAIC);
				uint8_t mosV = mosH >> 4;
				mosH &= 0x0F;

				++mosV;
				if (mosH) // let it be 0 if it's 0 (=> no mosaic)
					++mosH;

				uint8_t back = line % mosV;

				curX -= back*dmx;
				curY -= back*dmy;
			}
			else
				mosH = 0;

			int32_t intX, intY;

			uint16_t colorInd;

			uint8_t* pChar =
				m_memory.GetRealAddress(frame1 ? 0x0600A000 : 0x06000000);

			for (uint8_t x = 0; x < 240; ++x, ++ptr, curX += dx, curY += dy)
			{
				if (mosH && x % mosH)
				{
					*ptr = ptr[-1];
					continue;
				}

				intX = curX >> 8;
				intY = curY >> 8;

				// if we are off layer
				if (intX < 0 || intX >= 240)
					if (m_cnt & (0x1 << 13))
					{
						// NOTE : in C++, the modulus can be negative
						intX %= 240;
						if (intX < 0)
							intX += 240;
					}
					else
						continue;
				if (intY < 0 || intY >= 160)
					if (m_cnt & (0x1 << 13))
					{
						intY %= 160;
						if (intY < 0)
							intY += 160;
					}
					else
						continue;

				colorInd = pChar[intY * 240 + intX];

				if (colorInd)
					*ptr = m_pPalette[colorInd] | 0x8000;
				else
					*ptr = 0x0;
			}
		}

		void BgLayer::UpdateCnt (uint16_t cnt)
		{
			if (m_cnt == cnt)
				return;

			switch (cnt >> 14)
			{
				case 0:
					m_tWidth = m_tHeight = 32;
					m_rWidth = m_rHeight = 16;
					break;
				case 1:
					m_tWidth = 64;
					m_tHeight = 32;
					m_rWidth = m_rHeight = 32;
					break;
				case 2:
					m_tWidth = 32;
					m_tHeight = 64;
					m_rWidth = m_rHeight = 64;
					break;
				case 3:
					m_tWidth = m_tHeight = 64;
					m_rWidth = m_rHeight = 128;
					break;
			}

			m_priority = (cnt & 0x3);
			m_charAdd = 0x06000000 + ((cnt >> 2) & 0x3) * 0x4000;
			m_hicolor = cnt & (0x1 << 7);
			m_mapAdd = 0x06000000 + ((cnt >> 8) & 0x1F) * 0x0800;

			m_cnt = cnt;
		}
	}
}
