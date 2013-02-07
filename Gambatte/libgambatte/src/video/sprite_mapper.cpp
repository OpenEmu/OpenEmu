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
#include "sprite_mapper.h"
#include "counterdef.h"
#include "next_m0_time.h"
#include "../insertion_sort.h"
#include <cstring>
#include <algorithm>

namespace gambatte {

SpriteMapper::OamReader::OamReader(const LyCounter &lyCounter, const unsigned char *oamram)
: lyCounter(lyCounter), oamram(oamram), cgb_(false) {
	reset(oamram, false);
}

void SpriteMapper::OamReader::reset(const unsigned char *const oamram, const bool cgb) {
	this->oamram = oamram;
	this->cgb_   = cgb;
	setLargeSpritesSrc(false);
	lu = 0;
	lastChange = 0xFF;
	std::fill_n(szbuf, 40, largeSpritesSrc);

	unsigned pos = 0;
	unsigned distance = 80;

	while (distance--) {
		buf[pos] = oamram[((pos * 2) & ~3) | (pos & 1)];
		++pos;
	}
}

static unsigned toPosCycles(const unsigned long cc, const LyCounter &lyCounter) {
	unsigned lc = lyCounter.lineCycles(cc) + 3 - lyCounter.isDoubleSpeed() * 3u;

	if (lc >= 456)
		lc -= 456;

	return lc;
}

void SpriteMapper::OamReader::update(const unsigned long cc) {
	if (cc > lu) {
		if (changed()) {
			const unsigned lulc = toPosCycles(lu, lyCounter);

			unsigned pos = std::min(lulc, 80u);
			unsigned distance = 80;

			if ((cc - lu) >> lyCounter.isDoubleSpeed() < 456) {
				const unsigned cclc = toPosCycles(cc, lyCounter);

				distance = std::min(cclc, 80u) - pos + (cclc < lulc ? 80 : 0);
			}

			{
				const unsigned targetDistance = lastChange - pos + (lastChange <= pos ? 80 : 0);

				if (targetDistance <= distance) {
					distance = targetDistance;
					lastChange = 0xFF;
				}
			}

			while (distance--) {
				if (!(pos & 1)) {
					if (pos == 80)
						pos = 0;
					
					if (cgb_)
						szbuf[pos >> 1] = largeSpritesSrc;
					
					buf[pos    ] = oamram[pos * 2    ];
					buf[pos + 1] = oamram[pos * 2 + 1];
				} else
					szbuf[pos >> 1] = (szbuf[pos >> 1] & cgb_) | largeSpritesSrc;

				++pos;
			}
		}

		lu = cc;
	}
}

void SpriteMapper::OamReader::change(const unsigned long cc) {
	update(cc);
	lastChange = std::min(toPosCycles(lu, lyCounter), 80u);
}

void SpriteMapper::OamReader::setStatePtrs(SaveState &state) {
	state.ppu.oamReaderBuf.set(buf, sizeof buf);
	state.ppu.oamReaderSzbuf.set(szbuf, sizeof(szbuf) / sizeof(bool));
}

void SpriteMapper::OamReader::loadState(const SaveState &ss, const unsigned char *const oamram) {
	this->oamram = oamram;
	largeSpritesSrc = ss.mem.ioamhram.get()[0x140] >> 2 & 1;
	lu = ss.ppu.enableDisplayM0Time;
	change(lu);
}

void SpriteMapper::OamReader::enableDisplay(const unsigned long cc) {
	std::memset(buf, 0x00, sizeof(buf));
	std::fill(szbuf, szbuf + 40, false);
	lu = cc + (80 << lyCounter.isDoubleSpeed());
	lastChange = 80;
}

SpriteMapper::SpriteMapper(NextM0Time &nextM0Time,
                           const LyCounter &lyCounter,
                           const unsigned char *const oamram) :
	nextM0Time_(nextM0Time),
	oamReader(lyCounter, oamram)
{
	clearMap();
}

void SpriteMapper::reset(const unsigned char *const oamram, const bool cgb) {
	oamReader.reset(oamram, cgb);
	clearMap();
}

void SpriteMapper::clearMap() {
	std::memset(num, NEED_SORTING_MASK, sizeof num);
}

void SpriteMapper::mapSprites() {
	clearMap();

	for (unsigned i = 0x00; i < 0x50; i += 2) {
		const int spriteHeight = 8 << largeSprites(i >> 1);
		const unsigned bottom_pos = posbuf()[i] - (17u - spriteHeight);

		if (bottom_pos < 143u + spriteHeight) {
			const unsigned startly = static_cast<int>(bottom_pos) + 1 - spriteHeight >= 0
			                       ? static_cast<int>(bottom_pos) + 1 - spriteHeight :  0;
			unsigned char *map = spritemap + startly * 10;
			unsigned char *n   = num       + startly;
			unsigned char *const nend = num + (bottom_pos < 143 ? bottom_pos : 143) + 1;

			do {
				if (*n < NEED_SORTING_MASK + 10)
					map[(*n)++ - NEED_SORTING_MASK] = i;

				map += 10;
			} while (++n != nend);
		}
	}

	nextM0Time_.invalidatePredictedNextM0Time();
}

void SpriteMapper::sortLine(const unsigned ly) const {
	num[ly] &= ~NEED_SORTING_MASK;
	insertionSort(spritemap + ly * 10, spritemap + ly * 10 + num[ly], SpxLess(posbuf()));
}

unsigned long SpriteMapper::doEvent(const unsigned long time) {
	oamReader.update(time);
	mapSprites();
	return oamReader.changed() ? time + oamReader.lyCounter.lineTime() : static_cast<unsigned long>(DISABLED_TIME);
}

}
