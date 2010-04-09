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
#include "break_event.h"

BreakEvent::BreakEvent(unsigned char &drawStartCycle_in, unsigned char &scReadOffset_in) :
	VideoEvent(3),
	drawStartCycle(drawStartCycle_in),
	scReadOffset(scReadOffset_in)
{
	setDoubleSpeed(false);
	setScxSource(0);
}

void BreakEvent::doEvent() {
	scReadOffset = baseCycle;
	drawStartCycle = baseCycle + (scxSrc & 7);
	
	setTime(DISABLED_TIME);
}
