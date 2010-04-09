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
#ifndef VIDEO_M3_EXTRA_CYCLES_H
#define VIDEO_M3_EXTRA_CYCLES_H

class ScxReader;
class Window;
class SpriteMapper;

#include <cstring>

class M3ExtraCycles {
	enum { CYCLES_INVALID = 0xFF };
	
	mutable unsigned char cycles[144];
	
	const SpriteMapper &spriteMapper;
	const ScxReader &scxReader;
	const Window &win;
	
	void updateLine(unsigned ly) const;
	
public:
	M3ExtraCycles(const SpriteMapper &spriteMapper,
	              const ScxReader &scxReader_in,
	              const Window &win);
	
	void invalidateCache() {
		std::memset(cycles, CYCLES_INVALID, sizeof(cycles));
	}
	
	unsigned operator()(const unsigned ly) const {
		if (cycles[ly] == CYCLES_INVALID)
			updateLine(ly);
		
		return cycles[ly];
	}
};

#endif
