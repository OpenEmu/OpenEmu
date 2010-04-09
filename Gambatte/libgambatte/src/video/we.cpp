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
#include "we.h"
#include "../savestate.h"

We::WeEnableChecker::WeEnableChecker(We &we) :
	VideoEvent(8),
	we(we)
{}

void We::WeEnableChecker::doEvent() {
	we.set(we.src_);
	
	setTime(DISABLED_TIME);
}

We::WeDisableChecker::WeDisableChecker(We &we) :
	VideoEvent(9),
	we(we)
{}

void We::WeDisableChecker::doEvent() {
	we.set(we.we_ & we.src_);
	
	setTime(DISABLED_TIME);
}

We::We(M3ExtraCycles &m3ExtraCycles) :
	m3ExtraCycles_(m3ExtraCycles),
	enableChecker_(*this),
	disableChecker_(*this)
{
	setSource(false);
	we_ = src_;
}

void We::saveState(SaveState &state) const {
	state.ppu.lcdc = (state.ppu.lcdc & ~0x20) | we_ << 5;
}

void We::loadState(const SaveState &state) {
	we_ = state.ppu.lcdc >> 5 & 1;
}
