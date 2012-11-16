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
#include "interruptrequester.h"
#include "savestate.h"

namespace gambatte {

InterruptRequester::InterruptRequester() : minIntTime(0), ifreg_(0), iereg_(0) {}

void InterruptRequester::saveState(SaveState &state) const {
	state.mem.minIntTime = minIntTime;
	state.mem.IME = ime();
	state.mem.halted = halted();
}

void InterruptRequester::loadState(const SaveState &state) {
	minIntTime = state.mem.minIntTime;
	ifreg_ = state.mem.ioamhram.get()[0x10F];
	iereg_ = state.mem.ioamhram.get()[0x1FF] & 0x1F;
	intFlags.set(state.mem.IME, state.mem.halted);
	
	eventTimes.setValue<INTERRUPTS>(intFlags.imeOrHalted() && pendingIrqs() ? minIntTime : static_cast<unsigned long>(DISABLED_TIME));
}

void InterruptRequester::resetCc(const unsigned long oldCc, const unsigned long newCc) {
	minIntTime = minIntTime < oldCc ? 0 : minIntTime - (oldCc - newCc);
	
	if (eventTimes.value(INTERRUPTS) != DISABLED_TIME)
		eventTimes.setValue<INTERRUPTS>(minIntTime);
}

void InterruptRequester::ei(const unsigned long cc) {
	intFlags.setIme();
	minIntTime = cc + 1;
	
	if (pendingIrqs())
		eventTimes.setValue<INTERRUPTS>(minIntTime);
}

void InterruptRequester::di() {
	intFlags.unsetIme();
	
	if (!intFlags.imeOrHalted())
		eventTimes.setValue<INTERRUPTS>(DISABLED_TIME);
}

void InterruptRequester::halt() {
	intFlags.setHalted();
	
	if (pendingIrqs())
		eventTimes.setValue<INTERRUPTS>(minIntTime);
}

void InterruptRequester::unhalt() {
	intFlags.unsetHalted();
	
	if (!intFlags.imeOrHalted())
		eventTimes.setValue<INTERRUPTS>(DISABLED_TIME);
}

void InterruptRequester::flagIrq(const unsigned bit) {
	ifreg_ |= bit;
	
	if (intFlags.imeOrHalted() && pendingIrqs())
		eventTimes.setValue<INTERRUPTS>(minIntTime);
}

void InterruptRequester::ackIrq(const unsigned bit) {
	ifreg_ ^= bit;
	di();
}

void InterruptRequester::setIereg(const unsigned iereg) {
	iereg_ = iereg & 0x1F;
	
	if (intFlags.imeOrHalted())
		eventTimes.setValue<INTERRUPTS>(pendingIrqs() ? minIntTime : static_cast<unsigned long>(DISABLED_TIME));
}

void InterruptRequester::setIfreg(const unsigned ifreg) {
	ifreg_ = ifreg;
	
	if (intFlags.imeOrHalted())
		eventTimes.setValue<INTERRUPTS>(pendingIrqs() ? minIntTime : static_cast<unsigned long>(DISABLED_TIME));
}

}
