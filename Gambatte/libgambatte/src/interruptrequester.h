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
#ifndef INTERRUPT_REQUESTER_H
#define INTERRUPT_REQUESTER_H

#include "counterdef.h"
#include "minkeeper.h"

namespace gambatte {
struct SaveState;
enum MemEventId { UNHALT, END, BLIT, SERIAL, OAM, DMA, TIMA, VIDEO, INTERRUPTS };

class InterruptRequester {
	MinKeeper<INTERRUPTS + 1> eventTimes;
	unsigned long minIntTime;
	unsigned ifreg_;
	unsigned iereg_;
	
	class IntFlags {
		unsigned char flags_;
		enum { IME_MASK = 1, HALTED_MASK = 2 };
		
	public:
		IntFlags() : flags_(0) {}
		
		bool ime() const { return flags_ & IME_MASK; }
		bool halted() const { return flags_ & HALTED_MASK; }
		bool imeOrHalted() const { return flags_; }
		
		void setIme() { flags_ |= IME_MASK; }
		void unsetIme() { flags_ &= ~IME_MASK; }
		
		void setHalted() { flags_ |= HALTED_MASK; }
		void unsetHalted() { flags_ &= ~HALTED_MASK; }
		
		void set(const bool ime, const bool halted) { flags_ = halted * HALTED_MASK + ime * IME_MASK; }
	} intFlags;
	
public:
	InterruptRequester();
	
	void saveState(SaveState &) const;
	void loadState(const SaveState &);
	
	void resetCc(unsigned long oldCc, unsigned long newCc);
	
	unsigned ifreg() const { return ifreg_; }
	unsigned pendingIrqs() const { return ifreg_ & iereg_; }
	bool ime() const { return intFlags.ime(); }
	bool halted() const { return intFlags.halted(); }
	
	void ei(unsigned long cc);
	void di();
	void halt();
	void unhalt();
	void flagIrq(unsigned bit);
	void ackIrq(unsigned bit);
	void setIereg(unsigned iereg);
	void setIfreg(unsigned ifreg);
	
	MemEventId minEventId() const { return static_cast<MemEventId>(eventTimes.min()); }
	unsigned long minEventTime() const { return eventTimes.minValue(); }
	template<MemEventId id> void setEventTime(unsigned long value) { eventTimes.setValue<id>(value); }
	void setEventTime(const MemEventId id, unsigned long value) { eventTimes.setValue(id, value); }
	unsigned long eventTime(MemEventId id) const { return eventTimes.value(id); }
};

inline void flagHdmaReq(InterruptRequester *const intreq) { intreq->setEventTime<DMA>(0); }
inline void flagGdmaReq(InterruptRequester *const intreq) { intreq->setEventTime<DMA>(1); }
inline void ackDmaReq(InterruptRequester *const intreq) { intreq->setEventTime<DMA>(DISABLED_TIME); }
inline bool hdmaReqFlagged(const InterruptRequester &intreq) { return intreq.eventTime(DMA) == 0; }
inline bool gdmaReqFlagged(const InterruptRequester &intreq) { return intreq.eventTime(DMA) == 1; }

}

#endif
