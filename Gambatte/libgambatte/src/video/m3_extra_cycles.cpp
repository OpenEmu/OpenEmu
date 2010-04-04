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
#include "m3_extra_cycles.h"
#include "scx_reader.h"
#include "window.h"
#include "sprite_mapper.h"
#include "../insertion_sort.h"

M3ExtraCycles::M3ExtraCycles(const SpriteMapper &spriteMapper,
                             const ScxReader &scxReader,
                             const Window &win) :
	spriteMapper(spriteMapper),
	scxReader(scxReader),
	win(win)
{
	invalidateCache();
}

static const unsigned char* addLineCycles(const unsigned char *const start, const unsigned char *const end,
		const unsigned maxSpx, const unsigned scwxAnd7, const unsigned char *const posbuf_plus1, unsigned char *cycles_out) {
	unsigned sum = 0;
	
	const unsigned char *a = start; 
	
	for (; a < end; ++a) {
		const unsigned spx = posbuf_plus1[*a];
		
		if (spx > maxSpx)
			break;
		
		unsigned cycles = 6;
		const unsigned posAnd7 = (scwxAnd7 + spx) & 7;
		
		if (posAnd7 < 5) {
			cycles = 11 - posAnd7;
			
			for (const unsigned char *b = a; b > start;) {
				const unsigned bSpx = posbuf_plus1[*--b];
				
				if (spx - bSpx > 4U)
					break;
				
				if (((scwxAnd7 + bSpx) & 7) < 4 || spx == bSpx) {
					cycles = 6;
					break;
				}
			}
		}
		
		sum += cycles;
	}
	
	*cycles_out += sum;
	
	return a;
}

void M3ExtraCycles::updateLine(const unsigned ly) const {
	const bool windowEnabled = win.enabled(ly);
	
	cycles[ly] = windowEnabled ? scxReader.scxAnd7() + 6 : scxReader.scxAnd7();
	
	const unsigned numSprites = spriteMapper.numSprites(ly);
	
	if (numSprites == 0)
		return;
	
	unsigned char sortBuf[10];
	const unsigned char *tmp = spriteMapper.sprites(ly);
	
	if (spriteMapper.isCgb()) {
		std::memcpy(sortBuf, tmp, sizeof(sortBuf));
		insertionSort(sortBuf, sortBuf + numSprites, SpriteMapper::SpxLess(spriteMapper.posbuf()));
		tmp = sortBuf;
	}
	
	const unsigned char *const tmpend = tmp + numSprites;
	const unsigned char *const posbuf_plus1 = spriteMapper.posbuf() + 1;
	
	if (windowEnabled) {
		addLineCycles(addLineCycles(tmp, tmpend, win.wxReader.wx(), scxReader.scxAnd7(), posbuf_plus1, cycles + ly),
				tmpend, 167, 7 - win.wxReader.wx(), posbuf_plus1, cycles + ly);
	} else
		addLineCycles(tmp, tmpend, 167, scxReader.scxAnd7(), posbuf_plus1, cycles + ly);
}
