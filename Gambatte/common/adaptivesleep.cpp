/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
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
#include "adaptivesleep.h"

usec_t AdaptiveSleep::sleepUntil(usec_t base, usec_t inc) {
	usec_t now = getusecs();
	usec_t diff = now - base;
	
	if (diff >= inc)
		return diff - inc;
	
	diff = inc - diff;
	
	if (diff > oversleep + oversleepVar) {
		diff -= oversleep + oversleepVar;
		usecsleep(diff);
		const usec_t ideal = now + diff;
		now = getusecs();
		
		{
			usec_t curOversleep = now - ideal;
				
			if (negate(curOversleep) < curOversleep)
				curOversleep = 0;
			
			oversleepVar = (oversleepVar * 15 + (curOversleep < oversleep ? oversleep - curOversleep : curOversleep - oversleep)) >> 4;
			oversleep = (oversleep * 15 + curOversleep) >> 4;
		}
		
		noSleep = 60;
	} else if (--noSleep == 0) {
		noSleep = 60;
		oversleep = oversleepVar = 0;
	}
	
	while (now - base < inc)
		now = getusecs();
	
	return 0;
}
