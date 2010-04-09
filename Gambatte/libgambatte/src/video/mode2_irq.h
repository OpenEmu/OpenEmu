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
#ifndef VIDEO_MODE2_IRQ_H
#define VIDEO_MODE2_IRQ_H

class LycIrq;

#include "ly_counter.h"
#include "basic_add_event.h"

class Mode2Irq : public VideoEvent {
	const LyCounter &lyCounter;
	const LycIrq &lycIrq;
	unsigned char &ifReg;
	
public:
	Mode2Irq(const LyCounter &lyCounter_in, const LycIrq &lycIrq_in,
	         unsigned char &ifReg_in);
	
	void doEvent();
	static unsigned long schedule(unsigned statReg, const LyCounter &lyCounter, unsigned long cycleCounter);
};

#endif
