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
#ifndef STATIC_OUTPUT_TESTER_H
#define STATIC_OUTPUT_TESTER_H

#include "envelope_unit.h"

template<class Channel, class Unit>
class StaticOutputTester : public EnvelopeUnit::VolOnOffEvent {
	const Channel &ch;
	Unit &unit;
public:
	StaticOutputTester(const Channel &ch, Unit &unit) : ch(ch), unit(unit) {}
	void operator()(unsigned long cc);
};

template<class Channel, class Unit>
void StaticOutputTester<Channel, Unit>::operator()(const unsigned long cc) {
	if (ch.soMask && ch.master && ch.envelopeUnit.getVolume())
		unit.reviveCounter(cc);
	else
		unit.killCounter();
}

#endif
