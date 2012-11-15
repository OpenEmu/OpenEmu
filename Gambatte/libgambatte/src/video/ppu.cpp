/***************************************************************************
 *   Copyright (C) 2010 by Sindre Aamås                                    *
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
#include "ppu.h"
#include "savestate.h"
#include <algorithm>
#include <cstring>
#include <cstddef>

namespace {

using namespace gambatte;

#define PREP(u8) (((u8) << 7 & 0x80) | ((u8) << 5 & 0x40) | ((u8) << 3 & 0x20) | ((u8) << 1 & 0x10) | \
                  ((u8) >> 1 & 0x08) | ((u8) >> 3 & 0x04) | ((u8) >> 5 & 0x02) | ((u8) >> 7 & 0x01))

#define EXPAND(u8) ((PREP(u8) << 7 & 0x4000) | (PREP(u8) << 6 & 0x1000) | (PREP(u8) << 5 & 0x0400) | (PREP(u8) << 4 & 0x0100) | \
                    (PREP(u8) << 3 & 0x0040) | (PREP(u8) << 2 & 0x0010) | (PREP(u8) << 1 & 0x0004) | (PREP(u8)      & 0x0001))

#define EXPAND_ROW(n) EXPAND((n)|0x0), EXPAND((n)|0x1), EXPAND((n)|0x2), EXPAND((n)|0x3), \
                      EXPAND((n)|0x4), EXPAND((n)|0x5), EXPAND((n)|0x6), EXPAND((n)|0x7), \
                      EXPAND((n)|0x8), EXPAND((n)|0x9), EXPAND((n)|0xA), EXPAND((n)|0xB), \
                      EXPAND((n)|0xC), EXPAND((n)|0xD), EXPAND((n)|0xE), EXPAND((n)|0xF)

#define EXPAND_TABLE EXPAND_ROW(0x00), EXPAND_ROW(0x10), EXPAND_ROW(0x20), EXPAND_ROW(0x30), \
                     EXPAND_ROW(0x40), EXPAND_ROW(0x50), EXPAND_ROW(0x60), EXPAND_ROW(0x70), \
                     EXPAND_ROW(0x80), EXPAND_ROW(0x90), EXPAND_ROW(0xA0), EXPAND_ROW(0xB0), \
                     EXPAND_ROW(0xC0), EXPAND_ROW(0xD0), EXPAND_ROW(0xE0), EXPAND_ROW(0xF0)

static const unsigned short expand_lut[0x200] = {
	EXPAND_TABLE,

#undef PREP
#define PREP(u8) (u8)

	EXPAND_TABLE
};

#undef EXPAND_TABLE
#undef EXPAND_ROW
#undef EXPAND
#undef PREP

#define DECLARE_FUNC(n, id) \
	enum { ID##n = id }; \
	static void f##n (PPUPriv &); \
	static unsigned predictCyclesUntilXpos_f##n (const PPUPriv &, int targetxpos, unsigned cycles); \
	static const PPUState f##n##_ = { f##n, predictCyclesUntilXpos_f##n, ID##n }

namespace M2      {
	namespace Ly0    { DECLARE_FUNC(0, 0); }
	namespace LyNon0 { DECLARE_FUNC(0, 0); DECLARE_FUNC(1, 0); }
}

namespace M3Start { DECLARE_FUNC(0, 0); DECLARE_FUNC(1, 0); }

namespace M3Loop {
	namespace Tile            {
		DECLARE_FUNC(0, 0x80);
		DECLARE_FUNC(1, 0x81);
		DECLARE_FUNC(2, 0x82);
		DECLARE_FUNC(3, 0x83);
		DECLARE_FUNC(4, 0x84);
		DECLARE_FUNC(5, 0x85);
	}
	
	namespace LoadSprites     {
		DECLARE_FUNC(0, 0x88);
		DECLARE_FUNC(1, 0x89);
		DECLARE_FUNC(2, 0x8A);
		DECLARE_FUNC(3, 0x8B);
		DECLARE_FUNC(4, 0x8C);
		DECLARE_FUNC(5, 0x8D);
	}
	
	namespace StartWindowDraw {
		DECLARE_FUNC(0, 0x90);
		DECLARE_FUNC(1, 0x91);
		DECLARE_FUNC(2, 0x92);
		DECLARE_FUNC(3, 0x93);
		DECLARE_FUNC(4, 0x94);
		DECLARE_FUNC(5, 0x95);
	}
}

#undef DECLARE_FUNC

enum { WIN_DRAW_START = 1, WIN_DRAW_STARTED = 2 };

enum { M2_DS_OFFSET = 3 };
enum { MAX_M3START_CYCLES = 80 };

static inline unsigned weMasterCheckPriorToLyIncLineCycle(const bool cgb) { return 450 - cgb; }
static inline unsigned weMasterCheckAfterLyIncLineCycle(const bool cgb) { return 454 - cgb; }
static inline unsigned m3StartLineCycle(const bool /*cgb*/) { return 83; }

static inline void nextCall(const int cycles, const PPUState &state, PPUPriv &p) {
	const int c = p.cycles - cycles;

	if (c >= 0) {
		p.cycles = c;
		return state.f(p);
	}

	p.cycles = c;
	p.nextCallPtr = &state;
}

namespace M2 {
	namespace Ly0 {
		static void f0(PPUPriv &p) {
			p.weMaster = (p.lcdc & 0x20) && 0 == p.wy;
			p.winYPos = 0xFF;
			nextCall(m3StartLineCycle(p.cgb), M3Start::f0_, p);
		}
	}
	
	namespace LyNon0 {
		static void f0(PPUPriv &p) {
			p.weMaster |= (p.lcdc & 0x20) && p.lyCounter.ly() == p.wy;
			nextCall(weMasterCheckAfterLyIncLineCycle(p.cgb) - weMasterCheckPriorToLyIncLineCycle(p.cgb), f1_, p);
		}
		
		static void f1(PPUPriv &p) {
			p.weMaster |= (p.lcdc & 0x20) && p.lyCounter.ly() + 1 == p.wy;
			nextCall(456 - weMasterCheckAfterLyIncLineCycle(p.cgb) + m3StartLineCycle(p.cgb), M3Start::f0_, p);
		}
	}
	
	/*struct SpriteLess {
		bool operator()(const Sprite lhs, const Sprite rhs) const {
			return lhs.spx < rhs.spx;
		}
	};
	
	static void f0(PPUPriv &p) {
		std::memset(&p.spLut, 0, sizeof(p.spLut));
		p.reg0 = 0;
		p.nextSprite = 0;
		p.nextCallPtr = &f1_;
		f1(p);
	}
	
	static void f1(PPUPriv &p) {
		int cycles = p.cycles;
		unsigned oampos = p.reg0;
		unsigned nextSprite = p.nextSprite;
		const unsigned nly = (p.lyCounter.ly() + 1 == 154 ? 0 : p.lyCounter.ly() + 1) + ((p.lyCounter.time()-(p.now-p.cycles)) <= 4);
		const bool ls = p.spriteMapper.largeSpritesSource();
	
		do {
			const unsigned spy = p.spriteMapper.oamram()[oampos  ];
			const unsigned spx = p.spriteMapper.oamram()[oampos+1];
			const unsigned ydiff = spy - nly;
			
			if (ls ? ydiff < 16u : ydiff - 8u < 8u) {
				p.spriteList[nextSprite].spx = spx;
				p.spriteList[nextSprite].line = 15u - ydiff;
				p.spriteList[nextSprite].oampos = oampos;
				
				if (++nextSprite == 10) {
					cycles -= (0xA0 - 4 - oampos) >> 1;
					oampos = 0xA0 - 4;
				}
			}
			
			oampos += 4;
		} while ((cycles-=2) >= 0 && oampos != 0xA0);
		
		p.reg0 = oampos;
		p.nextSprite = nextSprite;
		p.cycles = cycles;
		
		if (oampos == 0xA0) {
			insertionSort(p.spriteList, p.spriteList + nextSprite, SpriteLess());
			p.spriteList[nextSprite].spx = 0xFF;
			p.nextSprite = 0;
			nextCall(0, M3Start::f0_, p);
		}
	}*/
}

namespace M3Start {
	static void f0(PPUPriv &p) {
		p.xpos = 0;
		
		if (p.winDrawState & p.lcdc >> 5 & WIN_DRAW_START) {
			p.winDrawState = WIN_DRAW_STARTED;
			p.wscx = 8 + (p.scx & 7);
			++p.winYPos;
		} else
			p.winDrawState = 0;
		
		p.nextCallPtr = &f1_;
		f1(p);
	}
	
	static void f1(PPUPriv &p) {
		while (p.xpos < MAX_M3START_CYCLES) {
			if ((p.xpos & 7) == (p.scx & 7))
				break;
			
			switch (p.xpos & 7) {
			case 0:
				if (p.winDrawState & WIN_DRAW_STARTED) {
					p.reg1    = p.vram[(p.lcdc << 4 & 0x400) + (p.winYPos & 0xF8) * 4 + (p.wscx >> 3 & 0x1F) + 0x1800];
					p.nattrib = p.vram[(p.lcdc << 4 & 0x400) + (p.winYPos & 0xF8) * 4 + (p.wscx >> 3 & 0x1F) + 0x3800];
				} else {
					p.reg1    = p.vram[((p.lcdc << 7 | p.scx >> 3) & 0x41F) + ((p.scy + p.lyCounter.ly()) & 0xF8) * 4 + 0x1800];
					p.nattrib = p.vram[((p.lcdc << 7 | p.scx >> 3) & 0x41F) + ((p.scy + p.lyCounter.ly()) & 0xF8) * 4 + 0x3800];
				}

				break;
			case 2:
				{
					const unsigned yoffset = (p.winDrawState & WIN_DRAW_STARTED) ? p.winYPos : p.scy + p.lyCounter.ly();
					
					p.reg0 = p.vram[0x1000 + (p.nattrib << 10 & 0x2000) - ((p.reg1 * 32 | p.lcdc << 8) & 0x1000)
										+ p.reg1 * 16 + ((-(p.nattrib >> 6 & 1) ^ yoffset) & 7) * 2];
				}
				
				break;
			case 4:
				{
					const unsigned yoffset = (p.winDrawState & WIN_DRAW_STARTED) ? p.winYPos : p.scy + p.lyCounter.ly();
					const unsigned r1 = p.vram[0x1000 + (p.nattrib << 10 & 0x2000) - ((p.reg1 * 32 | p.lcdc << 8) & 0x1000)
											+ p.reg1 * 16 + ((-(p.nattrib >> 6 & 1) ^ yoffset) & 7) * 2 + 1];
						
					p.ntileword = (expand_lut + (p.nattrib << 3 & 0x100))[p.reg0] +
					              (expand_lut + (p.nattrib << 3 & 0x100))[r1    ] * 2;
				}
				
				break;
			}
			
			++p.xpos;
			
			if (--p.cycles < 0)
				return;
		}
		
		{
			const unsigned ly = p.lyCounter.ly();
			const unsigned numSprites = p.spriteMapper.numSprites(ly);
			const unsigned char *const sprites = p.spriteMapper.sprites(ly);

			for (unsigned i = 0; i < numSprites; ++i) {
				const unsigned pos = sprites[i];
				const unsigned spy = p.spriteMapper.posbuf()[pos  ];
				const unsigned spx = p.spriteMapper.posbuf()[pos+1];

				p.spriteList[i].spx    = spx;
				p.spriteList[i].line   = ly + 16u - spy;
				p.spriteList[i].oampos = pos * 2;
				p.spwordList[i] = 0;
			}
			
			p.spriteList[numSprites].spx = 0xFF;
			p.nextSprite = 0;
		}
		
		p.xpos = 0;
		p.endx = 8 - (p.scx & 7);
		
		static const PPUState *const flut[8] = {
			&M3Loop::Tile::f0_,
			&M3Loop::Tile::f1_,
			&M3Loop::Tile::f2_,
			&M3Loop::Tile::f3_,
			&M3Loop::Tile::f4_,
			&M3Loop::Tile::f5_,
			&M3Loop::Tile::f5_,
			&M3Loop::Tile::f5_
		};
		
		nextCall(1-p.cgb, *flut[p.scx & 7], p);
	}
}

namespace M3Loop {
	static void doFullTilesUnrolledDmg(PPUPriv &p, const int xend, uint_least32_t *const dbufline,
			const unsigned char *const tileMapLine, const unsigned tileline, unsigned tileMapXpos) {
		const unsigned tileIndexSign = ~p.lcdc << 3 & 0x80;
		const unsigned char *const tileDataLine = p.vram + tileIndexSign * 32 + tileline * 2;
		const unsigned twmask = (p.lcdc & 1) * 3;
		int xpos = p.xpos;
		
		do {
			int nextSprite = p.nextSprite;
			
			if (static_cast<int>(p.spriteList[nextSprite].spx) < xpos + 8) {
				int cycles = p.cycles - 8;
				
				if (p.lcdc & 2) {
					cycles -= std::max(11 - (static_cast<int>(p.spriteList[nextSprite].spx) - xpos), 6);
					
					for (unsigned i = nextSprite + 1; static_cast<int>(p.spriteList[i].spx) < xpos + 8; ++i)
						cycles -= 6;
					
					if (cycles < 0)
						break;
					
					p.cycles = cycles;
					
					do {
						unsigned reg0, reg1   = p.spriteMapper.oamram()[p.spriteList[nextSprite].oampos + 2] * 16;
						const unsigned attrib = p.spriteMapper.oamram()[p.spriteList[nextSprite].oampos + 3];
		
						{
							const unsigned spline = ((attrib & 0x40) ?
									p.spriteList[nextSprite].line ^ 15 : p.spriteList[nextSprite].line) * 2;
							reg0 = p.vram[((p.lcdc & 4) ? (reg1 & ~16) | spline : reg1 | (spline & ~16))    ];
							reg1 = p.vram[((p.lcdc & 4) ? (reg1 & ~16) | spline : reg1 | (spline & ~16)) + 1];
						}
						
						p.spwordList[nextSprite] = expand_lut[reg0 + (attrib << 3 & 0x100)]
						                         + expand_lut[reg1 + (attrib << 3 & 0x100)] * 2;
						p.spriteList[nextSprite].attrib = attrib;
						++nextSprite;
					} while (static_cast<int>(p.spriteList[nextSprite].spx) < xpos + 8);
				} else {
					if (cycles < 0)
						break;
					
					p.cycles = cycles;
					
					do {
						++nextSprite;
					} while (static_cast<int>(p.spriteList[nextSprite].spx) < xpos + 8);
				}
				
				p.nextSprite = nextSprite;
			} else if (nextSprite-1 < 0 || static_cast<int>(p.spriteList[nextSprite-1].spx) <= xpos - 8) {
				if (!(static_cast<unsigned>(p.cycles) >> 3))
					break;
				
				unsigned n = ((xend + 7 < static_cast<int>(p.spriteList[nextSprite].spx)
						? xend + 7 : static_cast<int>(p.spriteList[nextSprite].spx)) - xpos) >> 3;
				n = static_cast<unsigned>(p.cycles) >> 3 < n ? static_cast<unsigned>(p.cycles) >> 3 : n;
				
				unsigned ntileword = p.ntileword;
				uint_least32_t *dst = dbufline + xpos - 8;
				p.cycles -= n * 8;
				xpos += n * 8;
				
				do {
					dst[0] = p.bgPalette[ntileword       & twmask];
					dst[1] = p.bgPalette[ntileword >>  2 & twmask];
					dst[2] = p.bgPalette[ntileword >>  4 & twmask];
					dst[3] = p.bgPalette[ntileword >>  6 & twmask];
					dst[4] = p.bgPalette[ntileword >>  8 & twmask];
					dst[5] = p.bgPalette[ntileword >> 10 & twmask];
					dst[6] = p.bgPalette[ntileword >> 12 & twmask];
					dst[7] = p.bgPalette[ntileword >> 14 & twmask];
					
					{
						unsigned r0, r1;
						
						r1 = tileMapLine[tileMapXpos++ & 0x1F];
						r0 = (tileDataLine + r1 * 16 - (r1 & tileIndexSign) * 32)[0];
						r1 = (tileDataLine + r1 * 16 - (r1 & tileIndexSign) * 32)[1];
						
						ntileword = expand_lut[r0] + expand_lut[r1] * 2;
					}
					
					dst += 8;
				} while (--n);
				
				p.ntileword = ntileword;
				continue;
			} else {
				int cycles = p.cycles - 8;
				
				if (cycles < 0)
					break;
				
				p.cycles = cycles;
			}
			
			{
				uint_least32_t *const dst = dbufline + (xpos - 8);
				const unsigned tileword = p.ntileword;
				
				dst[0] = p.bgPalette[tileword       & twmask];
				dst[1] = p.bgPalette[tileword >>  2 & twmask];
				dst[2] = p.bgPalette[tileword >>  4 & twmask];
				dst[3] = p.bgPalette[tileword >>  6 & twmask];
				dst[4] = p.bgPalette[tileword >>  8 & twmask];
				dst[5] = p.bgPalette[tileword >> 10 & twmask];
				dst[6] = p.bgPalette[tileword >> 12 & twmask];
				dst[7] = p.bgPalette[tileword >> 14 & twmask];
				
				int i = nextSprite - 1;
			
				if (!(p.lcdc & 2)) {
					do {
						const int pos = static_cast<int>(p.spriteList[i].spx) - xpos;
						p.spwordList[i] >>= (pos >= 0 ? (8 - pos) : (pos + 8)) * 2;
						--i;
					} while (i >= 0 && static_cast<int>(p.spriteList[i].spx) > xpos - 8);
				} else {
					do {
						unsigned n;
						int pos = static_cast<int>(p.spriteList[i].spx) - xpos;
						
						if (pos < 0) {
							n = pos + 8;
							pos = 0;
						} else
							n = 8 - pos;
						
						const unsigned attrib = p.spriteList[i].attrib;
						unsigned spword       = p.spwordList[i];
						const unsigned long *const spPalette = p.spPalette + (attrib >> 2 & 4);
						
						if (!(attrib & 0x80)) {
							switch (n) {
							case 8: if (spword >> 14    ) { dst[pos+7] = spPalette[spword >> 14    ]; }
							case 7: if (spword >> 12 & 3) { dst[pos+6] = spPalette[spword >> 12 & 3]; }
							case 6: if (spword >> 10 & 3) { dst[pos+5] = spPalette[spword >> 10 & 3]; }
							case 5: if (spword >>  8 & 3) { dst[pos+4] = spPalette[spword >>  8 & 3]; }
							case 4: if (spword >>  6 & 3) { dst[pos+3] = spPalette[spword >>  6 & 3]; }
							case 3: if (spword >>  4 & 3) { dst[pos+2] = spPalette[spword >>  4 & 3]; }
							case 2: if (spword >>  2 & 3) { dst[pos+1] = spPalette[spword >>  2 & 3]; }
							case 1: if (spword       & 3) { dst[pos  ] = spPalette[spword       & 3]; }
							}
							
							spword >>= n * 2;
							
							/*do {
								if (spword & 3)
									dst[pos] = spPalette[spword & 3];
								
								spword >>= 2;
								++pos;
							} while (--n);*/
						} else {
							unsigned tw = tileword >> pos * 2;
							
							do {
								if (spword & 3) {
									dst[pos] = !(tw & twmask)
											? spPalette[spword & 3]
											: p.bgPalette[tw & twmask];
								}
								
								spword >>= 2;
								tw     >>= 2;
								++pos;
							} while (--n);
						}
						
						p.spwordList[i] = spword;
						--i;
					} while (i >= 0 && static_cast<int>(p.spriteList[i].spx) > xpos - 8);
				}
			}
			
			{
				unsigned r0, r1;
				
				r1 = tileMapLine[tileMapXpos++ & 0x1F];
				r0 = (tileDataLine + r1 * 16 - (r1 & tileIndexSign) * 32)[0];
				r1 = (tileDataLine + r1 * 16 - (r1 & tileIndexSign) * 32)[1];
				
				p.ntileword = expand_lut[r0] + expand_lut[r1] * 2;
			}
			
			xpos = xpos + 8;
		} while (xpos < xend);
		
		p.xpos = xpos;
	}
	
	static void doFullTilesUnrolledCgb(PPUPriv &p, const int xend, uint_least32_t *const dbufline,
			const unsigned char *const tileMapLine, const unsigned tileline, unsigned tileMapXpos) {
		int xpos = p.xpos;
		const unsigned tileIndexSign = ~p.lcdc << 3 & 0x80;
		const unsigned char *const tileData = p.vram + tileIndexSign * 32;
		
		do {
			int nextSprite = p.nextSprite;
			
			if (static_cast<int>(p.spriteList[nextSprite].spx) < xpos + 8) {
				int cycles = p.cycles - 8;
				cycles -= std::max(11 - (static_cast<int>(p.spriteList[nextSprite].spx) - xpos), 6);
				
				for (unsigned i = nextSprite + 1; static_cast<int>(p.spriteList[i].spx) < xpos + 8; ++i)
					cycles -= 6;
				
				if (cycles < 0)
					break;
				
				p.cycles = cycles;
				
				do {
					unsigned reg0, reg1   = p.spriteMapper.oamram()[p.spriteList[nextSprite].oampos + 2] * 16;
					const unsigned attrib = p.spriteMapper.oamram()[p.spriteList[nextSprite].oampos + 3];
	
					{
						const unsigned spline = ((attrib & 0x40) ?
								p.spriteList[nextSprite].line ^ 15 : p.spriteList[nextSprite].line) * 2;
						reg0 = p.vram[(attrib << 10 & 0x2000) +
								((p.lcdc & 4) ? (reg1 & ~16) | spline : reg1 | (spline & ~16))    ];
						reg1 = p.vram[(attrib << 10 & 0x2000) +
								((p.lcdc & 4) ? (reg1 & ~16) | spline : reg1 | (spline & ~16)) + 1];
					}
					
					p.spwordList[nextSprite] = expand_lut[reg0 + (attrib << 3 & 0x100)]
					                         + expand_lut[reg1 + (attrib << 3 & 0x100)] * 2;
					p.spriteList[nextSprite].attrib = attrib;
					++nextSprite;
				} while (static_cast<int>(p.spriteList[nextSprite].spx) < xpos + 8);
				
				p.nextSprite = nextSprite;
			} else if (nextSprite-1 < 0 || static_cast<int>(p.spriteList[nextSprite-1].spx) <= xpos - 8) {
				if (!(static_cast<unsigned>(p.cycles) >> 3))
					break;
				
				unsigned n = ((xend + 7 < static_cast<int>(p.spriteList[nextSprite].spx)
						? xend + 7 : static_cast<int>(p.spriteList[nextSprite].spx)) - xpos) >> 3;
				n = static_cast<unsigned>(p.cycles) >> 3 < n ? static_cast<unsigned>(p.cycles) >> 3 : n;
				
				unsigned ntileword = p.ntileword;
				unsigned nattrib   = p.nattrib;
				uint_least32_t *dst = dbufline + xpos - 8;
				p.cycles -= n * 8;
				xpos += n * 8;
				
				do {
					dst[0] = (p.bgPalette + (nattrib & 7) * 4)[ntileword       & 3];
					dst[1] = (p.bgPalette + (nattrib & 7) * 4)[ntileword >>  2 & 3];
					dst[2] = (p.bgPalette + (nattrib & 7) * 4)[ntileword >>  4 & 3];
					dst[3] = (p.bgPalette + (nattrib & 7) * 4)[ntileword >>  6 & 3];
					dst[4] = (p.bgPalette + (nattrib & 7) * 4)[ntileword >>  8 & 3];
					dst[5] = (p.bgPalette + (nattrib & 7) * 4)[ntileword >> 10 & 3];
					dst[6] = (p.bgPalette + (nattrib & 7) * 4)[ntileword >> 12 & 3];
					dst[7] = (p.bgPalette + (nattrib & 7) * 4)[ntileword >> 14    ];
					
					{
						unsigned r0, r1;
						
						{
							const unsigned tmxpos = tileMapXpos++ & 0x1F;
							r1      = tileMapLine[tmxpos         ];
							nattrib = tileMapLine[tmxpos + 0x2000];
						}
						
						r0 = (tileData + (nattrib << 10 & 0x2000) + r1 * 16 -
								(r1 & tileIndexSign) * 32 + ((nattrib & 0x40) ? tileline ^ 7 : tileline) * 2)[0];
						r1 = (tileData + (nattrib << 10 & 0x2000) + r1 * 16 -
								(r1 & tileIndexSign) * 32 + ((nattrib & 0x40) ? tileline ^ 7 : tileline) * 2)[1];
						
						ntileword = (expand_lut + (nattrib << 3 & 0x100))[r0] + (expand_lut + (nattrib << 3 & 0x100))[r1] * 2;
					}
					
					dst += 8;
				} while (--n);
				
				p.ntileword = ntileword;
				p.nattrib   = nattrib;
				continue;
			} else {
				int cycles = p.cycles - 8;
				
				if (cycles < 0)
					break;
				
				p.cycles = cycles;
			}
			
			{
				uint_least32_t *const dst = dbufline + (xpos - 8);
				const unsigned tileword = p.ntileword;
				const unsigned attrib   = p.nattrib;
				const unsigned long *const bgPalette = p.bgPalette + (attrib & 7) * 4;
				
				dst[0] = bgPalette[tileword       & 3];
				dst[1] = bgPalette[tileword >>  2 & 3];
				dst[2] = bgPalette[tileword >>  4 & 3];
				dst[3] = bgPalette[tileword >>  6 & 3];
				dst[4] = bgPalette[tileword >>  8 & 3];
				dst[5] = bgPalette[tileword >> 10 & 3];
				dst[6] = bgPalette[tileword >> 12 & 3];
				dst[7] = bgPalette[tileword >> 14    ];
				
				int i = nextSprite - 1;
			
				if (!(p.lcdc & 2)) {
					do {
						const int pos = static_cast<int>(p.spriteList[i].spx) - xpos;
						p.spwordList[i] >>= (pos >= 0 ? (8 - pos) : (pos + 8)) * 2;
						--i;
					} while (i >= 0 && static_cast<int>(p.spriteList[i].spx) > xpos - 8);
				} else {
					unsigned char idtab[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
					const unsigned bgenmask = p.lcdc << 7 & 0x80;
					
					do {
						unsigned n;
						int pos = static_cast<int>(p.spriteList[i].spx) - xpos;
						
						if (pos < 0) {
							n = pos + 8;
							pos = 0;
						} else
							n = 8 - pos;
						
						const unsigned id      = p.spriteList[i].oampos;
						const unsigned sattrib = p.spriteList[i].attrib;
						unsigned spword        = p.spwordList[i];
						const unsigned long *const spPalette = p.spPalette + (sattrib & 7) * 4;
						
						if (!((attrib | sattrib) & bgenmask)) {
							switch (n) {
							case 8: if ((spword >> 14    ) && id < idtab[pos+7]) {
									idtab[pos+7] = id;
									  dst[pos+7] = spPalette[spword >> 14    ];
								}
							case 7: if ((spword >> 12 & 3) && id < idtab[pos+6]) {
									idtab[pos+6] = id;
									  dst[pos+6] = spPalette[spword >> 12 & 3];
								}
							case 6: if ((spword >> 10 & 3) && id < idtab[pos+5]) {
									idtab[pos+5] = id;
									  dst[pos+5] = spPalette[spword >> 10 & 3];
								}
							case 5: if ((spword >>  8 & 3) && id < idtab[pos+4]) {
									idtab[pos+4] = id;
									  dst[pos+4] = spPalette[spword >>  8 & 3];
								}
							case 4: if ((spword >>  6 & 3) && id < idtab[pos+3]) {
									idtab[pos+3] = id;
									  dst[pos+3] = spPalette[spword >>  6 & 3];
								}
							case 3: if ((spword >>  4 & 3) && id < idtab[pos+2]) {
									idtab[pos+2] = id;
									  dst[pos+2] = spPalette[spword >>  4 & 3];
								}
							case 2: if ((spword >>  2 & 3) && id < idtab[pos+1]) {
									idtab[pos+1] = id;
									  dst[pos+1] = spPalette[spword >>  2 & 3];
								}
							case 1: if ((spword       & 3) && id < idtab[pos  ]) {
									idtab[pos  ] = id;
									  dst[pos  ] = spPalette[spword       & 3];
								}
							}
							
							spword >>= n * 2;
							
							/*do {
								if ((spword & 3) && id < idtab[pos]) {
									idtab[pos] = id;
										dst[pos] = spPalette[spword & 3];
								}
								
								spword >>= 2;
								++pos;
							} while (--n);*/
						} else {							
							unsigned tw = tileword >> pos * 2;
							
							do {
								if ((spword & 3) && id < idtab[pos]) {
									idtab[pos] = id;
									  dst[pos] = !(tw & 3) ? spPalette[spword & 3] : bgPalette[tw & 3];
								}
								
								spword >>= 2;
								tw     >>= 2;
								++pos;
							} while (--n);
						}
						
						p.spwordList[i] = spword;
						--i;
					} while (i >= 0 && static_cast<int>(p.spriteList[i].spx) > xpos - 8);
				}
			}
			
			{
				unsigned r0, r1, nattrib;
				
				{
					const unsigned tmxpos = tileMapXpos++ & 0x1F;
					r1      = tileMapLine[tmxpos         ];
					nattrib = tileMapLine[tmxpos + 0x2000];
				}
				
				r0 = (tileData + (nattrib << 10 & 0x2000) + r1 * 16 -
						(r1 & tileIndexSign) * 32 + ((nattrib & 0x40) ? tileline ^ 7 : tileline) * 2)[0];
				r1 = (tileData + (nattrib << 10 & 0x2000) + r1 * 16 -
						(r1 & tileIndexSign) * 32 + ((nattrib & 0x40) ? tileline ^ 7 : tileline) * 2)[1];
				
				p.ntileword = (expand_lut + (nattrib << 3 & 0x100))[r0] + (expand_lut + (nattrib << 3 & 0x100))[r1] * 2;
				p.nattrib   = nattrib;
			}
			
			xpos = xpos + 8;
		} while (xpos < xend);
		
		p.xpos = xpos;
	}
	
	static void doFullTilesUnrolled(PPUPriv &p) {
		int xpos = p.xpos;
		const int xend = static_cast<int>(p.wx) < xpos || p.wx >= 168 ? 161 : static_cast<int>(p.wx) - 7;
		
		if (xpos >= xend)
			return;
		
		uint_least32_t *const dbufline = p.framebuf.fbline();
		const unsigned char *tileMapLine;
		unsigned tileline;
		unsigned tileMapXpos;
		
		if (p.winDrawState & WIN_DRAW_STARTED) {
			tileMapLine = p.vram + (p.lcdc << 4 & 0x400) + (p.winYPos & 0xF8) * 4 + 0x1800;
			tileMapXpos = (xpos + p.wscx) >> 3;
			tileline    = p.winYPos & 7;
		} else {
			tileMapLine = p.vram + (p.lcdc << 7 & 0x400) + ((p.scy + p.lyCounter.ly()) & 0xF8) * 4 + 0x1800;
			tileMapXpos = (p.scx + xpos + 1 - p.cgb) >> 3;
			tileline    = (p.scy + p.lyCounter.ly()) & 7;
		}
		
		if (xpos < 8) {
			uint_least32_t prebuf[16];
			
			if (p.cgb) {
				doFullTilesUnrolledCgb(p, xend < 8 ? xend : 8, prebuf + (8 - xpos), tileMapLine, tileline, tileMapXpos);
			} else
				doFullTilesUnrolledDmg(p, xend < 8 ? xend : 8, prebuf + (8 - xpos), tileMapLine, tileline, tileMapXpos);
			
			const int newxpos = p.xpos;
			
			if (newxpos > 8) {
				std::memcpy(dbufline, prebuf + (8 - xpos), (newxpos - 8) * sizeof(uint_least32_t));
			} else if (newxpos < 8)
				return;
			
			if (newxpos >= xend)
				return;
			
			tileMapXpos += (newxpos - xpos) >> 3;
		}
		
		if (p.cgb) {
			doFullTilesUnrolledCgb(p, xend, dbufline, tileMapLine, tileline, tileMapXpos);
		} else
			doFullTilesUnrolledDmg(p, xend, dbufline, tileMapLine, tileline, tileMapXpos);
	}
	
	static void plotPixel(PPUPriv &p) {
		const int xpos = p.xpos;
		const unsigned tileword = p.tileword;
		uint_least32_t *const fbline = p.framebuf.fbline();
		
		if (static_cast<int>(p.wx) == xpos && (p.weMaster || (p.wy2 == p.lyCounter.ly() && (p.lcdc & 0x20))) && xpos < 167) {
			if (p.winDrawState == 0 && (p.lcdc & 0x20)) {
				p.winDrawState = WIN_DRAW_START | WIN_DRAW_STARTED;
				++p.winYPos;
			} else if (!p.cgb && (p.winDrawState == 0 || xpos == 166))
				p.winDrawState |= WIN_DRAW_START;
		}
		
		const unsigned twdata = tileword & ((p.lcdc & 1) | p.cgb) * 3;
		unsigned long pixel = p.bgPalette[twdata + (p.attrib & 7) * 4];
		int i = static_cast<int>(p.nextSprite) - 1;
		
		if (i >= 0 && static_cast<int>(p.spriteList[i].spx) > xpos - 8) {
			unsigned spdata = 0;
			unsigned attrib = 0;
			
			if (p.cgb) {
				unsigned minId = 0xFF;
				
				do {
					if ((p.spwordList[i] & 3) && p.spriteList[i].oampos < minId) {
						spdata = p.spwordList[i] & 3;
						attrib = p.spriteList[i].attrib;
						minId  = p.spriteList[i].oampos;
					}
					
					p.spwordList[i] >>= 2;
					--i;
				} while (i >= 0 && static_cast<int>(p.spriteList[i].spx) > xpos - 8);
				
				if (spdata && (p.lcdc & 2) && (!((attrib | p.attrib) & 0x80) || !twdata || !(p.lcdc & 1)))
					pixel = p.spPalette[(attrib & 7) * 4 + spdata];
			} else {
				do {
					if (p.spwordList[i] & 3) {
						spdata = p.spwordList[i] & 3;
						attrib = p.spriteList[i].attrib;
					}
					
					p.spwordList[i] >>= 2;
					--i;
				} while (i >= 0 && static_cast<int>(p.spriteList[i].spx) > xpos - 8);
				
				if (spdata && (p.lcdc & 2) && (!(attrib & 0x80) || !twdata))
					pixel = p.spPalette[(attrib >> 2 & 4) + spdata];
			}
		}
		
		if (xpos - 8 >= 0)
			fbline[xpos - 8] = pixel;
		
		p.xpos = xpos + 1;
		p.tileword = tileword >> 2;
	}
	
	static void plotPixelIfNoSprite(PPUPriv &p) {
		if (p.spriteList[p.nextSprite].spx == p.xpos) {
			if (!((p.lcdc & 2) | p.cgb)) {
				do {
					++p.nextSprite;
				} while (p.spriteList[p.nextSprite].spx == p.xpos);
				
				plotPixel(p);
			}
		} else
			plotPixel(p);
	}
	
	static unsigned long nextM2Time(const PPUPriv &p) {
		unsigned long nextm2 = p.lyCounter.isDoubleSpeed()
				? p.lyCounter.time() + (weMasterCheckPriorToLyIncLineCycle(true ) + M2_DS_OFFSET) * 2 - 456 * 2
				: p.lyCounter.time() +  weMasterCheckPriorToLyIncLineCycle(p.cgb)                     - 456    ;

		if (p.lyCounter.ly() == 143)
			nextm2 += (456 * 10 + 456 - weMasterCheckPriorToLyIncLineCycle(p.cgb)) << p.lyCounter.isDoubleSpeed();
		
		return nextm2;
	}
	
	static void xpos168(PPUPriv &p) {
		p.lastM0Time = p.now - (p.cycles << p.lyCounter.isDoubleSpeed());
		
		const unsigned long nextm2 = nextM2Time(p);
		
		p.cycles = p.now >= nextm2
				?  (static_cast<long>(p.now - nextm2) >> p.lyCounter.isDoubleSpeed())
				: -(static_cast<long>(nextm2 - p.now) >> p.lyCounter.isDoubleSpeed());
				
		nextCall(0, p.lyCounter.ly() == 143 ? M2::Ly0::f0_ : M2::LyNon0::f0_, p);
	}
	
	static bool handleWinDrawStartReq(const PPUPriv &p, const int xpos, unsigned char &winDrawState) {
		const bool startWinDraw = (xpos < 167 || p.cgb) && (winDrawState &= WIN_DRAW_STARTED);
		
		if (!(p.lcdc & 0x20))
			winDrawState &= ~WIN_DRAW_STARTED;
		
		return startWinDraw;
	}
	
	static bool handleWinDrawStartReq(PPUPriv &p) {
		return handleWinDrawStartReq(p, p.xpos, p.winDrawState);
	}
	
	namespace StartWindowDraw {
		static void inc(const PPUState &nextf, PPUPriv &p) {
			if (!(p.lcdc & 0x20) && p.cgb) {
				plotPixelIfNoSprite(p);
				
				if (p.xpos == p.endx) {
					if (p.xpos < 168) {
						nextCall(1,Tile::f0_,p);
					} else
						xpos168(p);
					
					return;
				}
			}
			
			nextCall(1,nextf,p);
		}
		
		static void f0(PPUPriv &p) {
			if (p.xpos == p.endx) {
				p.tileword = p.ntileword;
				p.attrib   = p.nattrib;
				p.endx = p.xpos < 160 ? p.xpos + 8 : 168;
			}
			
			p.wscx = 8 - p.xpos;
			
			if (p.winDrawState & WIN_DRAW_STARTED) {
				p.reg1    = p.vram[(p.lcdc << 4 & 0x400) + (p.winYPos & 0xF8) * 4 + 0x1800];
				p.nattrib = p.vram[(p.lcdc << 4 & 0x400) + (p.winYPos & 0xF8) * 4 + 0x3800];
			} else {
				p.reg1    = p.vram[(p.lcdc << 7 & 0x400) + ((p.scy + p.lyCounter.ly()) & 0xF8) * 4 + 0x1800];
				p.nattrib = p.vram[(p.lcdc << 7 & 0x400) + ((p.scy + p.lyCounter.ly()) & 0xF8) * 4 + 0x3800];
			}
			
			inc(f1_,p);
		}
		
		static void f1(PPUPriv &p) {
			inc(f2_,p);
		}
		
		static void f2(PPUPriv &p) {
			const unsigned yoffset = (p.winDrawState & WIN_DRAW_STARTED) ? p.winYPos : p.scy + p.lyCounter.ly();
			
			p.reg0 = p.vram[0x1000 + (p.nattrib << 10 & 0x2000)
					- ((p.reg1 * 32 | p.lcdc << 8) & 0x1000)
					+ p.reg1 * 16 + ((-(p.nattrib >> 6 & 1) ^ yoffset) & 7) * 2];

			inc(f3_,p);
		}
		
		static void f3(PPUPriv &p) {
			inc(f4_,p);
		}
		
		static void f4(PPUPriv &p) {
			const unsigned yoffset = (p.winDrawState & WIN_DRAW_STARTED) ? p.winYPos : p.scy + p.lyCounter.ly();
			const unsigned r1 = p.vram[0x1000 + (p.nattrib << 10 & 0x2000)
						- ((p.reg1 * 32 | p.lcdc << 8) & 0x1000)
						+ p.reg1 * 16 + ((-(p.nattrib >> 6 & 1) ^ yoffset) & 7) * 2 + 1];
				
			p.ntileword = (expand_lut + (p.nattrib << 3 & 0x100))[p.reg0] +
			              (expand_lut + (p.nattrib << 3 & 0x100))[r1    ] * 2;
			
			inc(f5_,p);
		}
		
		static void f5(PPUPriv &p) {
			inc(Tile::f0_,p);
		}
	};

	namespace LoadSprites {
		static void inc(const PPUState &nextf, PPUPriv &p) {
			plotPixelIfNoSprite(p);
			
			if (p.xpos == p.endx) {
				if (p.xpos < 168) {
					nextCall(1,Tile::f0_,p);
				} else
					xpos168(p);
			} else
				nextCall(1,nextf,p);
		}
		
		static void f0(PPUPriv &p) {
			p.reg1 = p.spriteMapper.oamram()[p.spriteList[p.currentSprite].oampos + 2];
			nextCall(1,f1_,p);
		}
		
		static void f1(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			p.spriteList[p.currentSprite].attrib = p.spriteMapper.oamram()[p.spriteList[p.currentSprite].oampos + 3];
			inc(f2_,p);
		}
		
		static void f2(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			const unsigned spline = ((p.spriteList[p.currentSprite].attrib & 0x40) ?
					p.spriteList[p.currentSprite].line ^ 15 : p.spriteList[p.currentSprite].line) * 2;
			p.reg0 = p.vram[(p.spriteList[p.currentSprite].attrib << 10 & p.cgb * 0x2000) +
					((p.lcdc & 4) ? (p.reg1 * 16 & ~16) | spline : p.reg1 * 16 | (spline & ~16))];
			inc(f3_,p);
		}
		
		static void f3(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			inc(f4_,p);
		}
		
		static void f4(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			const unsigned spline = ((p.spriteList[p.currentSprite].attrib & 0x40) ?
					p.spriteList[p.currentSprite].line ^ 15 : p.spriteList[p.currentSprite].line) * 2;
			p.reg1 = p.vram[(p.spriteList[p.currentSprite].attrib << 10 & p.cgb * 0x2000) +
					((p.lcdc & 4) ? (p.reg1 * 16 & ~16) | spline : p.reg1 * 16 | (spline & ~16)) + 1];
			inc(f5_,p);
		}
		
		static void f5(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			plotPixelIfNoSprite(p);
			
			unsigned entry = p.currentSprite;
			
			if (entry == p.nextSprite) {
				++p.nextSprite;
			} else {
				entry = p.nextSprite - 1;
				p.spriteList[entry] = p.spriteList[p.currentSprite];
			}
			
			p.spwordList[entry] = expand_lut[p.reg0 + (p.spriteList[entry].attrib << 3 & 0x100)] +
			                      expand_lut[p.reg1 + (p.spriteList[entry].attrib << 3 & 0x100)] * 2;
			p.spriteList[entry].spx = p.xpos;
			
			if (p.xpos == p.endx) {
				if (p.xpos < 168) {
					nextCall(1,Tile::f0_,p);
				} else
					xpos168(p);
			} else {
				p.nextCallPtr = &Tile::f5_;
				nextCall(1,Tile::f5_,p);
			}
		}
	};
	
	namespace Tile {
		static void inc(const PPUState &nextf, PPUPriv &p) {
			plotPixelIfNoSprite(p);
			
			if (p.xpos == 168) {
				xpos168(p);
			} else
				nextCall(1,nextf,p);
		}
		
		static void f0(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
 			doFullTilesUnrolled(p);
			
			if (p.xpos == 168) {
				++p.cycles;
				return xpos168(p);
			}
			
			p.tileword = p.ntileword;
			p.attrib   = p.nattrib;
			p.endx = p.xpos < 160 ? p.xpos + 8 : 168;
			
			if (p.winDrawState & WIN_DRAW_STARTED) {
				p.reg1    = p.vram[(p.lcdc << 4 & 0x400) + (p.winYPos & 0xF8) * 4
							+ ((p.xpos + p.wscx) >> 3 & 0x1F) + 0x1800];
				p.nattrib = p.vram[(p.lcdc << 4 & 0x400) + (p.winYPos & 0xF8) * 4
							+ ((p.xpos + p.wscx) >> 3 & 0x1F) + 0x3800];
			} else {
				p.reg1    = p.vram[((p.lcdc << 7 | (p.scx + p.xpos + 1 - p.cgb) >> 3) & 0x41F)
								+ ((p.scy + p.lyCounter.ly()) & 0xF8) * 4 + 0x1800];
				p.nattrib = p.vram[((p.lcdc << 7 | (p.scx + p.xpos + 1 - p.cgb) >> 3) & 0x41F)
								+ ((p.scy + p.lyCounter.ly()) & 0xF8) * 4 + 0x3800];
			}
			
			inc(f1_,p);
		}
		
		static void f1(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			inc(f2_,p);
		}
		
		static void f2(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			const unsigned yoffset = (p.winDrawState & WIN_DRAW_STARTED) ? p.winYPos : p.scy + p.lyCounter.ly();
			
			p.reg0 = p.vram[0x1000 + (p.nattrib << 10 & 0x2000)
					- ((p.reg1 * 32 | p.lcdc << 8) & 0x1000)
					+ p.reg1 * 16 + ((-(p.nattrib >> 6 & 1) ^ yoffset) & 7) * 2];

			inc(f3_,p);
		}
		
		static void f3(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			inc(f4_,p);
		}
		
		static void f4(PPUPriv &p) {
			if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
				return StartWindowDraw::f0(p);
			
			const unsigned yoffset = (p.winDrawState & WIN_DRAW_STARTED) ? p.winYPos : p.scy + p.lyCounter.ly();
			const unsigned r1 = p.vram[0x1000 + (p.nattrib << 10 & 0x2000)
						- ((p.reg1 * 32 | p.lcdc << 8) & 0x1000)
						+ p.reg1 * 16 + ((-(p.nattrib >> 6 & 1) ^ yoffset) & 7) * 2 + 1];
				
			p.ntileword = (expand_lut + (p.nattrib << 3 & 0x100))[p.reg0] +
			              (expand_lut + (p.nattrib << 3 & 0x100))[r1    ] * 2;
			
			plotPixelIfNoSprite(p);
			
			if (p.xpos == 168) {
				xpos168(p);
			} else
				nextCall(1,f5_,p);
		}
		
		static void f5(PPUPriv &p) {
			int endx = p.endx;
			p.nextCallPtr = &f5_;
		
			do {
				if ((p.winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p))
					return StartWindowDraw::f0(p);
				
				if (p.spriteList[p.nextSprite].spx == p.xpos) {
					if ((p.lcdc & 2) | p.cgb) {
						p.currentSprite = p.nextSprite;
						return LoadSprites::f0(p);
					}
					
					do {
						++p.nextSprite;
					} while (p.spriteList[p.nextSprite].spx == p.xpos);
				}
				
				plotPixel(p);
				
				if (p.xpos == endx) {
					if (endx < 168) {
						nextCall(1,f0_,p);
					} else
						xpos168(p);
					
					return;
				}
			} while (--p.cycles >= 0);
		}
	};
};


namespace M2 {
	namespace Ly0 {
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, unsigned winDrawState, int targetxpos, unsigned cycles);
	}
	
	namespace LyNon0 {
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, unsigned winDrawState, int targetxpos, unsigned cycles);
	}
}

namespace M3Loop {
	static unsigned predictCyclesUntilXposNextLine(const PPUPriv &p, unsigned winDrawState, const int targetx) {
		if (p.wx == 166 && !p.cgb && p.xpos < 167 && (p.weMaster || (p.wy2 == p.lyCounter.ly() && (p.lcdc & 0x20))))
			winDrawState = WIN_DRAW_START | (WIN_DRAW_STARTED & p.lcdc >> 4);
		
		const unsigned cycles = (nextM2Time(p) - p.now) >> p.lyCounter.isDoubleSpeed();

		return p.lyCounter.ly() == 143
			?    M2::Ly0::predictCyclesUntilXpos_f0(p, winDrawState, targetx, cycles)
			: M2::LyNon0::predictCyclesUntilXpos_f0(p, winDrawState, targetx, cycles);
	}
	
	namespace StartWindowDraw {
		static unsigned predictCyclesUntilXpos_fn(const PPUPriv &p, int xpos, int endx, unsigned ly,
				unsigned nextSprite, bool weMaster, unsigned winDrawState, int fno, int targetx, unsigned cycles);
	}
	
	namespace Tile {
		static const unsigned char* addSpriteCycles(const unsigned char *nextSprite,
				const unsigned char *spriteEnd, const unsigned char *const spxOf, const unsigned maxSpx,
				const unsigned firstTileXpos, unsigned prevSpriteTileNo, unsigned *const cyclesAccumulator) {
			unsigned sum = 0;

			while (nextSprite < spriteEnd && spxOf[*nextSprite] <= maxSpx) {
				unsigned cycles = 6;
				const unsigned distanceFromTileStart = (spxOf[*nextSprite] - firstTileXpos) &  7;
				const unsigned tileNo                = (spxOf[*nextSprite] - firstTileXpos) & ~7;

				if (distanceFromTileStart < 5 && tileNo != prevSpriteTileNo)
					cycles = 11 - distanceFromTileStart;

				prevSpriteTileNo = tileNo;
				sum += cycles;
				++nextSprite;
			}

			*cyclesAccumulator += sum;

			return nextSprite;
		}
		
		static unsigned predictCyclesUntilXpos_fn(const PPUPriv &p, const int xpos,
				const int endx, const unsigned ly, const unsigned nextSprite,
				const bool weMaster, unsigned char winDrawState, const int fno, const int targetx, unsigned cycles) {
			if ((winDrawState & WIN_DRAW_START) && handleWinDrawStartReq(p, xpos, winDrawState)) {
				return StartWindowDraw::predictCyclesUntilXpos_fn(p, xpos, endx,
						ly, nextSprite, weMaster, WIN_DRAW_STARTED & p.lcdc >> 4, 0, targetx, cycles);
			}
			
			if (xpos > targetx)
				return predictCyclesUntilXposNextLine(p, winDrawState, targetx);
			
			enum { NO_TILE_NUMBER = 1 }; // low bit set, so it will never be equal to an actual tile number.
			
			int nwx = 0xFF;
			cycles += targetx - xpos;

			if (p.wx - static_cast<unsigned>(xpos) < targetx - static_cast<unsigned>(xpos) && (p.lcdc & 0x20)
					&& (weMaster || p.wy2 == ly) && !(winDrawState & WIN_DRAW_STARTED) && (p.cgb || p.wx != 166)) {
				nwx = p.wx;
				cycles += 6;
			}

			if ((p.lcdc & 2) | p.cgb) {
				const unsigned char *sprite = p.spriteMapper.sprites(ly);
				const unsigned char *const spriteEnd = sprite + p.spriteMapper.numSprites(ly);
				sprite += nextSprite;
				
				if (sprite < spriteEnd) {
					const int spx = p.spriteMapper.posbuf()[*sprite + 1];
					unsigned firstTileXpos = static_cast<unsigned>(endx) & 7; // ok even if endx is capped at 168, because fno will be used.
					unsigned prevSpriteTileNo = (xpos - firstTileXpos) & ~7; // this tile. all sprites on this tile will now add 6 cycles.

					// except this one
					if (fno + spx - xpos < 5 && spx <= nwx) {
						cycles += 11 - (fno + spx - xpos);
						sprite += 1;
					}

					if (nwx < targetx) {
						sprite = addSpriteCycles(sprite, spriteEnd, p.spriteMapper.posbuf() + 1,
										nwx, firstTileXpos, prevSpriteTileNo, &cycles);
						firstTileXpos = nwx + 1;
						prevSpriteTileNo = NO_TILE_NUMBER;
					}

					addSpriteCycles(sprite, spriteEnd, p.spriteMapper.posbuf() + 1,
							targetx, firstTileXpos, prevSpriteTileNo, &cycles);
				}
			}
			
			return cycles;
		}
		
		static unsigned predictCyclesUntilXpos_fn(const PPUPriv &p,
				const int endx, const int fno, const int targetx, const unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.xpos, endx, p.lyCounter.ly(),
					p.nextSprite, p.weMaster, p.winDrawState, fno, targetx, cycles);
		}
		
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.xpos < 160 ? p.xpos + 8 : 168, 0, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f1(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 1, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f2(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 2, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f3(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 3, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f4(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 4, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f5(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 5, targetx, cycles);
		}
	}
	
	namespace StartWindowDraw {
		static unsigned predictCyclesUntilXpos_fn(const PPUPriv &p, int xpos,
				const int endx, const unsigned ly, const unsigned nextSprite, const bool weMaster,
				const unsigned winDrawState, const int fno, const int targetx, unsigned cycles) {
			if (xpos > targetx)
				return predictCyclesUntilXposNextLine(p, winDrawState, targetx);
			
			unsigned cinc = 6 - fno;

			if (!(p.lcdc & 0x20) && p.cgb) {
				unsigned xinc = std::min<int>(cinc, std::min(endx, targetx + 1) - xpos);

				if (((p.lcdc & 2) | p.cgb) && p.spriteList[nextSprite].spx < xpos + xinc) {
					xpos = p.spriteList[nextSprite].spx;
				} else {
					cinc = xinc;
					xpos += xinc;
				}
			}
			
			cycles += cinc;

			if (xpos <= targetx) {
				return Tile::predictCyclesUntilXpos_fn(p, xpos, xpos < 160 ? xpos + 8 : 168,
							ly, nextSprite, weMaster, winDrawState, 0, targetx, cycles);
			}
			
			return cycles - 1;
		}
		
		static unsigned predictCyclesUntilXpos_fn(const PPUPriv &p, const int endx,
					const int fno, const int targetx, const unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.xpos, endx,
					p.lyCounter.ly(), p.nextSprite, p.weMaster, p.winDrawState, fno, targetx, cycles);
		}
		
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, const int targetx, const unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.xpos == p.endx ? (p.xpos < 160 ? p.xpos + 8 : 168) : p.endx, 0, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f1(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 1, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f2(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 2, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f3(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 3, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f4(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 4, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f5(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, p.endx, 5, targetx, cycles);
		}
	}

	namespace LoadSprites {
		static unsigned predictCyclesUntilXpos_fn(const PPUPriv &p, const int fno, const int targetx, unsigned cycles) {
			unsigned nextSprite = p.nextSprite;
			
			if ((p.lcdc & 2) | p.cgb) {
				cycles += 6 - fno;
				nextSprite += 1;
			}
			
			return Tile::predictCyclesUntilXpos_fn(p, p.xpos, p.endx, p.lyCounter.ly(),
					nextSprite, p.weMaster, p.winDrawState, 5, targetx, cycles);
		}
		
 		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, 0, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f1(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, 1, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f2(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, 2, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f3(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, 3, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f4(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, 4, targetx, cycles);
		}
		static unsigned predictCyclesUntilXpos_f5(const PPUPriv &p, int targetx, unsigned cycles) {
			return predictCyclesUntilXpos_fn(p, 5, targetx, cycles);
		}
	}
}

namespace M3Start {
	static unsigned predictCyclesUntilXpos_f1(const PPUPriv &p, const unsigned xpos, const unsigned ly,
				const bool weMaster, const unsigned winDrawState, const int targetx, unsigned cycles) {
		cycles += std::min((static_cast<unsigned>(p.scx) - static_cast<unsigned>(xpos)) & 7, MAX_M3START_CYCLES - xpos) + 1 - p.cgb;
		return M3Loop::Tile::predictCyclesUntilXpos_fn(p, 0, 8 - (p.scx & 7), ly, 0,
				weMaster, winDrawState, std::min(p.scx & 7, 5), targetx, cycles);
	}
	
	static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, const unsigned ly,
			const bool weMaster, unsigned winDrawState, const int targetx, const unsigned cycles) {
		winDrawState = (winDrawState & p.lcdc >> 5 & WIN_DRAW_START) ? WIN_DRAW_STARTED : 0;
		return predictCyclesUntilXpos_f1(p, 0, ly, weMaster, winDrawState, targetx, cycles);
	}
	
	static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, const int targetx, const unsigned cycles) {
		const unsigned ly = p.lyCounter.ly() + (p.lyCounter.time() - p.now < 16);
		return predictCyclesUntilXpos_f0(p, ly, p.weMaster, p.winDrawState, targetx, cycles);
	}
	
	static unsigned predictCyclesUntilXpos_f1(const PPUPriv &p, const int targetx, const unsigned cycles) {
		return predictCyclesUntilXpos_f1(p, p.xpos, p.lyCounter.ly(), p.weMaster, p.winDrawState, targetx, cycles);
	}
}

namespace M2 {
	namespace Ly0 {
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p,
				const unsigned winDrawState, const int targetx, const unsigned cycles) {
			const bool weMaster = (p.lcdc & 0x20) && 0 == p.wy;
			const unsigned ly = 0;
			
			return M3Start::predictCyclesUntilXpos_f0(p, ly, weMaster,
					winDrawState, targetx, cycles + m3StartLineCycle(p.cgb));

		}
		
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, const int targetx, const unsigned cycles) {
			return predictCyclesUntilXpos_f0(p, p.winDrawState, targetx, cycles);
		}
	}
	
	namespace LyNon0 {
		static unsigned predictCyclesUntilXpos_f1(const PPUPriv &p, bool weMaster,
				const unsigned winDrawState, const int targetx, const unsigned cycles) {
			const unsigned ly = p.lyCounter.ly() + 1;
			
			weMaster |= (p.lcdc & 0x20) && ly == p.wy;
			
			return M3Start::predictCyclesUntilXpos_f0(p, ly, weMaster, winDrawState, targetx,
					cycles + 456 - weMasterCheckAfterLyIncLineCycle(p.cgb) + m3StartLineCycle(p.cgb));
		}
		
		static unsigned predictCyclesUntilXpos_f1(const PPUPriv &p, const int targetx, const unsigned cycles) {
			return predictCyclesUntilXpos_f1(p, p.weMaster, p.winDrawState, targetx, cycles);
		}
		
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p,
				const unsigned winDrawState, const int targetx, const unsigned cycles) {
			const bool weMaster = p.weMaster || ((p.lcdc & 0x20) && p.lyCounter.ly() == p.wy);
			
			return predictCyclesUntilXpos_f1(p, weMaster, winDrawState, targetx,
					cycles + weMasterCheckAfterLyIncLineCycle(p.cgb) - weMasterCheckPriorToLyIncLineCycle(p.cgb));
		}
		
		static unsigned predictCyclesUntilXpos_f0(const PPUPriv &p, const int targetx, const unsigned cycles) {
			return predictCyclesUntilXpos_f0(p, p.winDrawState, targetx, cycles);
		}
	}
}

} // anon namespace

namespace gambatte {

PPUPriv::PPUPriv(NextM0Time &nextM0Time, const unsigned char *const oamram, const unsigned char *const vram) :
	vram(vram),
	nextCallPtr(&M2::Ly0::f0_),
	now(0),
	lastM0Time(0),
	cycles(-4396),
	tileword(0),
	ntileword(0),
	spriteMapper(nextM0Time, lyCounter, oamram),
	lcdc(0),
	scy(0),
	scx(0),
	wy(0),
	wy2(0),
	wx(0),
	winDrawState(0),
	wscx(0),
	winYPos(0),
	reg0(0),
	reg1(0),
	attrib(0),
	nattrib(0),
	nextSprite(0),
	currentSprite(0xFF),
	xpos(0),
	endx(0),
	cgb(false),
	weMaster(false)
{
	std::memset(spriteList, 0, sizeof spriteList);
	std::memset(spwordList, 0, sizeof spwordList);
}

static void saveSpriteList(const PPUPriv &p, SaveState &ss) {
	for (unsigned i = 0; i < 10; ++i) {
		ss.ppu.spAttribList[i] = p.spriteList[i].attrib;
		ss.ppu.spByte0List[i] = p.spwordList[i] & 0xFF;
		ss.ppu.spByte1List[i] = p.spwordList[i] >> 8;
	}
	
	ss.ppu.nextSprite    = p.nextSprite;
	ss.ppu.currentSprite = p.currentSprite;
}

void PPU::saveState(SaveState &ss) const {
	p_.spriteMapper.saveState(ss);
	ss.ppu.videoCycles = p_.lcdc & 0x80 ? p_.lyCounter.frameCycles(p_.now) : 0;
	ss.ppu.xpos = p_.xpos;
	ss.ppu.endx = p_.endx;
	ss.ppu.reg0 = p_.reg0;
	ss.ppu.reg1 = p_.reg1;
	ss.ppu.tileword = p_.tileword;
	ss.ppu.ntileword = p_.ntileword;
	ss.ppu.attrib = p_.attrib;
	ss.ppu.nattrib = p_.nattrib;
	ss.ppu.winDrawState = p_.winDrawState;
	ss.ppu.winYPos = p_.winYPos;
	ss.ppu.oldWy = p_.wy2;
	ss.ppu.wscx = p_.wscx;
	ss.ppu.weMaster = p_.weMaster;
	saveSpriteList(p_, ss);
	ss.ppu.state = p_.nextCallPtr->id;
	ss.ppu.lastM0Time = p_.now - p_.lastM0Time;
}

namespace {

template<class T, class K, std::size_t start, std::size_t len>
struct BSearch {
	static std::size_t upperBound(const T a[], const K e) {
		if (e < a[start + len / 2])
			return BSearch<T, K, start, len / 2>::upperBound(a, e);
		
		return BSearch<T, K, start + len / 2 + 1, len - (len / 2 + 1)>::upperBound(a, e);
	}
};

template<class T, class K, std::size_t start>
struct BSearch<T, K, start, 0> {
	static std::size_t upperBound(const T[], const K) {
		return start;
	}
};

template<std::size_t len, class T, class K>
std::size_t upperBound(const T a[], const K e) {
	return BSearch<T, K, 0, len>::upperBound(a, e);
}

struct CycleState {
	const PPUState *state;
	long cycle;
	operator long() const { return cycle; }
};

static const PPUState * decodeM3LoopState(const unsigned state) {
	switch (state) {
	case M3Loop::Tile::ID0: return &M3Loop::Tile::f0_;
	case M3Loop::Tile::ID1: return &M3Loop::Tile::f1_;
	case M3Loop::Tile::ID2: return &M3Loop::Tile::f2_;
	case M3Loop::Tile::ID3: return &M3Loop::Tile::f3_;
	case M3Loop::Tile::ID4: return &M3Loop::Tile::f4_;
	case M3Loop::Tile::ID5: return &M3Loop::Tile::f5_;
	
	case M3Loop::LoadSprites::ID0: return &M3Loop::LoadSprites::f0_;
	case M3Loop::LoadSprites::ID1: return &M3Loop::LoadSprites::f1_;
	case M3Loop::LoadSprites::ID2: return &M3Loop::LoadSprites::f2_;
	case M3Loop::LoadSprites::ID3: return &M3Loop::LoadSprites::f3_;
	case M3Loop::LoadSprites::ID4: return &M3Loop::LoadSprites::f4_;
	case M3Loop::LoadSprites::ID5: return &M3Loop::LoadSprites::f5_;
	
	case M3Loop::StartWindowDraw::ID0: return &M3Loop::StartWindowDraw::f0_;
	case M3Loop::StartWindowDraw::ID1: return &M3Loop::StartWindowDraw::f1_;
	case M3Loop::StartWindowDraw::ID2: return &M3Loop::StartWindowDraw::f2_;
	case M3Loop::StartWindowDraw::ID3: return &M3Loop::StartWindowDraw::f3_;
	case M3Loop::StartWindowDraw::ID4: return &M3Loop::StartWindowDraw::f4_;
	case M3Loop::StartWindowDraw::ID5: return &M3Loop::StartWindowDraw::f5_;
	}

	return 0;
}

static long cyclesUntilM0Upperbound(const PPUPriv &p) {
	long cycles = 168 - p.xpos + 6;
	
	for (unsigned i = p.nextSprite; i < 10 && p.spriteList[i].spx < 168; ++i)
		cycles += 11;
	
	return cycles;
}

static void loadSpriteList(PPUPriv &p, const SaveState &ss) {
	if (ss.ppu.videoCycles < 144 * 456UL && ss.ppu.xpos < 168) {
		const unsigned ly = ss.ppu.videoCycles / 456;
		const unsigned numSprites = p.spriteMapper.numSprites(ly);
		const unsigned char *const sprites = p.spriteMapper.sprites(ly);

		for (unsigned i = 0; i < numSprites; ++i) {
			const unsigned pos = sprites[i];
			const unsigned spy = p.spriteMapper.posbuf()[pos  ];
			const unsigned spx = p.spriteMapper.posbuf()[pos+1];

			p.spriteList[i].spx    = spx;
			p.spriteList[i].line   = ly + 16u - spy;
			p.spriteList[i].oampos = pos * 2;
			p.spriteList[i].attrib = ss.ppu.spAttribList[i] & 0xFF;
			p.spwordList[i] = (ss.ppu.spByte1List[i] * 0x100 + ss.ppu.spByte0List[i]) & 0xFFFF;
		}
		
		p.spriteList[numSprites].spx = 0xFF;
		p.nextSprite = std::min<unsigned>(ss.ppu.nextSprite, numSprites);
		
		while (p.spriteList[p.nextSprite].spx < ss.ppu.xpos)
			++p.nextSprite;
		
		p.currentSprite = std::min<unsigned>(p.nextSprite, ss.ppu.currentSprite);
	}
}

}

void PPU::loadState(const SaveState &ss, const unsigned char *const oamram) {
	const PPUState *const m3loopState = decodeM3LoopState(ss.ppu.state);
	const long videoCycles = std::min(ss.ppu.videoCycles, 70223UL);
	const bool ds = p_.cgb & ss.mem.ioamhram.get()[0x14D] >> 7;
	const long vcycs = videoCycles - ds * M2_DS_OFFSET < 0
	                 ? videoCycles - ds * M2_DS_OFFSET + 70224
	                 : videoCycles - ds * M2_DS_OFFSET;
	const long lineCycles = static_cast<unsigned long>(vcycs) % 456;
	
	p_.now = ss.cpu.cycleCounter;
	p_.lcdc = ss.mem.ioamhram.get()[0x140];
	p_.lyCounter.setDoubleSpeed(ds);
	p_.lyCounter.reset(std::min(ss.ppu.videoCycles, 70223ul), ss.cpu.cycleCounter);
	p_.spriteMapper.loadState(ss, oamram);
	p_.winYPos = ss.ppu.winYPos;
	p_.scy = ss.mem.ioamhram.get()[0x142];
	p_.scx = ss.mem.ioamhram.get()[0x143];
	p_.wy = ss.mem.ioamhram.get()[0x14A];
	p_.wy2 = ss.ppu.oldWy;
	p_.wx = ss.mem.ioamhram.get()[0x14B];
	p_.xpos = std::min<int>(ss.ppu.xpos, 168);
	p_.endx = (p_.xpos & ~7) + (ss.ppu.endx & 7);
	p_.endx = std::min(p_.endx <= p_.xpos ? p_.endx + 8 : p_.endx, 168);
	p_.reg0 = ss.ppu.reg0 & 0xFF;
	p_.reg1 = ss.ppu.reg1 & 0xFF;
	p_.tileword = ss.ppu.tileword & 0xFFFF;
	p_.ntileword = ss.ppu.ntileword & 0xFFFF;
	p_.attrib = ss.ppu.attrib & 0xFF;
	p_.nattrib = ss.ppu.nattrib & 0xFF;
	p_.wscx = ss.ppu.wscx;
	p_.weMaster = ss.ppu.weMaster;
	p_.winDrawState = ss.ppu.winDrawState & (WIN_DRAW_START | WIN_DRAW_STARTED);
	p_.lastM0Time = p_.now - ss.ppu.lastM0Time;
	loadSpriteList(p_, ss);
	
	if (m3loopState && videoCycles < 144 * 456L && p_.xpos < 168
			&& lineCycles + cyclesUntilM0Upperbound(p_) < static_cast<long>(weMasterCheckPriorToLyIncLineCycle(p_.cgb))) {
		p_.nextCallPtr = m3loopState;
		p_.cycles = -1;
	} else if (vcycs < 143 * 456L + static_cast<long>(m3StartLineCycle(p_.cgb)) + MAX_M3START_CYCLES) {
		const struct CycleState lineCycleStates[] = {
			{    &M3Start::f0_, m3StartLineCycle(p_.cgb) },
			{    &M3Start::f1_, m3StartLineCycle(p_.cgb) + MAX_M3START_CYCLES },
			{ &M2::LyNon0::f0_, weMasterCheckPriorToLyIncLineCycle(p_.cgb) },
			{ &M2::LyNon0::f1_, weMasterCheckAfterLyIncLineCycle(p_.cgb) },
			{    &M3Start::f0_, m3StartLineCycle(p_.cgb) + 456 }
		};
		
		const std::size_t pos =
			upperBound<sizeof(lineCycleStates) / sizeof(lineCycleStates[0]) - 1>(lineCycleStates, lineCycles);
		
		p_.cycles = lineCycles - lineCycleStates[pos].cycle;
		p_.nextCallPtr = lineCycleStates[pos].state;
		
		if (&M3Start::f1_ == lineCycleStates[pos].state) {
			p_.xpos   = lineCycles - m3StartLineCycle(p_.cgb) + 1;
			p_.cycles = -1;
		}
	} else {
		p_.cycles = vcycs - 70224;
		p_.nextCallPtr = &M2::Ly0::f0_;
	}
}

void PPU::reset(const unsigned char *const oamram, const bool cgb) {
	p_.cgb = cgb;
	p_.spriteMapper.reset(oamram, cgb);
}

void PPU::resetCc(const unsigned long oldCc, const unsigned long newCc) {
	const unsigned long dec = oldCc - newCc;
	const unsigned long videoCycles = p_.lcdc & 0x80 ? p_.lyCounter.frameCycles(p_.now) : 0;
	
	p_.now -= dec;
	p_.lastM0Time = p_.lastM0Time ? p_.lastM0Time - dec : p_.lastM0Time;
	p_.lyCounter.reset(videoCycles, p_.now);
	p_.spriteMapper.resetCycleCounter(oldCc, newCc);
}

void PPU::speedChange(const unsigned long cycleCounter) {
	const unsigned long videoCycles = p_.lcdc & 0x80 ? p_.lyCounter.frameCycles(p_.now) : 0;
	
	p_.spriteMapper.preSpeedChange(cycleCounter);
	p_.lyCounter.setDoubleSpeed(!p_.lyCounter.isDoubleSpeed());
	p_.lyCounter.reset(videoCycles, p_.now);
	p_.spriteMapper.postSpeedChange(cycleCounter);
	
	if (&M2::Ly0::f0_ == p_.nextCallPtr || &M2::LyNon0::f0_ == p_.nextCallPtr) {
		if (p_.lyCounter.isDoubleSpeed()) {
			p_.cycles -= M2_DS_OFFSET;
		} else
			p_.cycles += M2_DS_OFFSET;
	}
}

unsigned long PPU::predictedNextXposTime(const unsigned xpos) const {
	return p_.now + (p_.nextCallPtr->predictCyclesUntilXpos_f(p_, xpos, -p_.cycles) << p_.lyCounter.isDoubleSpeed());
}

void PPU::setLcdc(const unsigned lcdc, const unsigned long cc) {
	if ((p_.lcdc ^ lcdc) & lcdc & 0x80) {
		p_.now = cc;
		p_.lastM0Time = 0;
		p_.lyCounter.reset(0, p_.now);
		p_.spriteMapper.enableDisplay(cc);
		p_.weMaster = (lcdc & 0x20) && 0 == p_.wy;
		p_.winDrawState = 0;
		p_.nextCallPtr = &M3Start::f0_;
		p_.cycles = -static_cast<int>(m3StartLineCycle(p_.cgb) + M2_DS_OFFSET * p_.lyCounter.isDoubleSpeed());
	} else if ((p_.lcdc ^ lcdc) & 0x20) {
		if (!(lcdc & 0x20)) {
			if (p_.winDrawState == WIN_DRAW_STARTED || p_.xpos == 168)
				p_.winDrawState &= ~WIN_DRAW_STARTED;
		} else if (p_.winDrawState == WIN_DRAW_START) {
			p_.winDrawState |= WIN_DRAW_STARTED;
			++p_.winYPos;
		}
	}
	
	if ((p_.lcdc ^ lcdc) & 0x04) {
		if (p_.lcdc & lcdc & 0x80)
			p_.spriteMapper.oamChange(cc);
		
		p_.spriteMapper.setLargeSpritesSource(lcdc & 0x04);
	}
	
	p_.lcdc = lcdc;
}

void PPU::update(const unsigned long cc) {
	const int cycles = (cc - p_.now) >> p_.lyCounter.isDoubleSpeed();
	
	p_.now += cycles << p_.lyCounter.isDoubleSpeed();
	p_.cycles += cycles;
	
	if (p_.cycles >= 0) {
		p_.framebuf.setFbline(p_.lyCounter.ly());
		p_.nextCallPtr->f(p_);
	}
}

}
