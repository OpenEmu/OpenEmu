/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
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
#include "sprite_mapper.h"
#include "m3_extra_cycles.h"
#include "../insertion_sort.h"
#include <cstring>

#include <algorithm>

SpriteMapper::OamReader::OamReader(const LyCounter &lyCounter, const unsigned char *oamram)
: lyCounter(lyCounter), oamram(oamram) {
	reset(oamram);
}

void SpriteMapper::OamReader::reset(const unsigned char *const oamram) {
	this->oamram = oamram;
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
	unsigned lc = lyCounter.lineCycles(cc) + 4 - lyCounter.isDoubleSpeed() * 3u;

	if (lc >= 456)
		lc -= 456;

	return lc >> 1;
}

void SpriteMapper::OamReader::update(const unsigned long cc) {
	if (cc > lu) {
		if (changed()) {
			const unsigned lulc = toPosCycles(lu, lyCounter);

			unsigned pos = std::min(lulc, 40u);
			unsigned distance = 40;

			if ((cc - lu) >> lyCounter.isDoubleSpeed() < 456) {
				const unsigned cclc = toPosCycles(cc, lyCounter);

				distance = std::min(cclc, 40u) - pos + (cclc < lulc ? 40 : 0);
			}

			{
				const unsigned targetDistance = lastChange - pos + (lastChange <= pos ? 40 : 0);

				if (targetDistance <= distance) {
					distance = targetDistance;
					lastChange = 0xFF;
				}
			}

			while (distance--) {
				if (pos >= 40)
					pos = 0;

				szbuf[pos] = largeSpritesSrc;
				buf[pos * 2] = oamram[pos * 4];
				buf[pos * 2 + 1] = oamram[pos * 4 + 1];

				++pos;
			}
		}

		lu = cc;
	}
}

void SpriteMapper::OamReader::change(const unsigned long cc) {
	update(cc);
	lastChange = std::min(toPosCycles(lu, lyCounter), 40u);
}

void SpriteMapper::OamReader::setStatePtrs(SaveState &state) {
	state.ppu.oamReaderBuf.set(buf, sizeof buf);
	state.ppu.oamReaderSzbuf.set(szbuf, sizeof(szbuf) / sizeof(bool));
}

void SpriteMapper::OamReader::enableDisplay(const unsigned long cc) {
	std::memset(buf, 0x00, sizeof(buf));
	std::fill(szbuf, szbuf + 40, false);
	lu = cc + 160;
	lastChange = 40;
}

bool SpriteMapper::OamReader::oamAccessible(const unsigned long cycleCounter, const M3ExtraCycles &m3ExtraCycles) const {
	unsigned ly = lyCounter.ly();
	unsigned lc = lyCounter.lineCycles(cycleCounter) + 4 - lyCounter.isDoubleSpeed() * 3u;

	if (lc >= 456) {
		lc -= 456;
		++ly;
	}

	return cycleCounter < lu || ly >= 144 || lc >= 80 + 173 + m3ExtraCycles(ly);
}

SpriteMapper::SpriteMapper(M3ExtraCycles &m3ExtraCycles,
                           const LyCounter &lyCounter,
                           const unsigned char *const oamram) :
	VideoEvent(2),
	m3ExtraCycles(m3ExtraCycles),
	oamReader(lyCounter, oamram),
	cgb(false)
{
	clearMap();
}

void SpriteMapper::reset(const unsigned char *const oamram, const bool cgb_in) {
	oamReader.reset(oamram);
	cgb = cgb_in;
	clearMap();
}

void SpriteMapper::clearMap() {
	std::memset(num, cgb ? 0 : NEED_SORTING_MASK, sizeof(num));
}

void SpriteMapper::mapSprites() {
	clearMap();

	for (unsigned i = 0x00; i < 0x50; i += 2) {
		const unsigned spriteHeight = 8u << largeSprites(i >> 1);
		const unsigned bottom_pos = posbuf()[i] - (17u - spriteHeight);

		if (bottom_pos >= 143 + spriteHeight)
			continue;

		unsigned char *map = spritemap;
		unsigned char *n = num;

		if (bottom_pos >= spriteHeight) {
			const unsigned startly = bottom_pos + 1 - spriteHeight;
			n += startly;
			map += startly * 10;
		}

		unsigned char *const end = num + (bottom_pos >= 143 ? 143 : bottom_pos);

		do {
			if ((*n & ~NEED_SORTING_MASK) < 10)
				map[(*n)++ & ~NEED_SORTING_MASK] = i;

			map += 10;
			++n;
		} while (n <= end);
	}

	m3ExtraCycles.invalidateCache();
}

void SpriteMapper::sortLine(const unsigned ly) const {
	num[ly] &= ~NEED_SORTING_MASK;
	insertionSort(spritemap + ly * 10, spritemap + ly * 10 + num[ly], SpxLess(posbuf()));
}

void SpriteMapper::doEvent() {
	oamReader.update(time());
	mapSprites();
	setTime(oamReader.changed() ? time() + oamReader.lyCounter.lineTime() : static_cast<unsigned long>(DISABLED_TIME));
}
