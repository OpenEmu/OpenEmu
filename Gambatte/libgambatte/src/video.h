/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
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
#ifndef VIDEO_H
#define VIDEO_H

#include "video/ppu.h"
#include "video/lyc_irq.h"
#include "video/next_m0_time.h"
#include "interruptrequester.h"
#include "osd_element.h"
#include "minkeeper.h"
#include <memory>

namespace gambatte {

class VideoInterruptRequester {
	InterruptRequester * intreq;
	
public:
	explicit VideoInterruptRequester(InterruptRequester * intreq) : intreq(intreq) {}
	void flagHdmaReq() const { gambatte::flagHdmaReq(intreq); }
	void flagIrq(const unsigned bit) const { intreq->flagIrq(bit); }
	void setNextEventTime(const unsigned long time) const { intreq->setEventTime<VIDEO>(time); }
};

class M0Irq {
	unsigned char statReg_;
	unsigned char lycReg_;
	
public:
	M0Irq() : statReg_(0), lycReg_(0) {}
	
	void lcdReset(const unsigned statReg, const unsigned lycReg) {
		statReg_ = statReg;
		 lycReg_ =  lycReg;
	}
	
	void statRegChange(const unsigned statReg,
			const unsigned long nextM0IrqTime, const unsigned long cc, const bool cgb) {
		if (nextM0IrqTime - cc > cgb * 2U)
			statReg_ = statReg;
	}
	
	void lycRegChange(const unsigned lycReg,
			const unsigned long nextM0IrqTime, const unsigned long cc, const bool ds, const bool cgb) {
		if (nextM0IrqTime - cc > cgb * 5 + 1U - ds)
			lycReg_ = lycReg;
	}
	
	void doEvent(unsigned char *const ifreg, const unsigned ly, const unsigned statReg, const unsigned lycReg) {
		if (((statReg_ | statReg) & 0x08) && (!(statReg_ & 0x40) || ly != lycReg_))
			*ifreg |= 2;
		
		statReg_ = statReg;
		 lycReg_ =  lycReg;
	}
	
	void saveState(SaveState &state) const {
		state.ppu.m0lyc = lycReg_;
	}
	
	void loadState(const SaveState &state) {
		 lycReg_ = state.ppu.m0lyc;
		statReg_ = state.mem.ioamhram.get()[0x141];
	}
	
	unsigned statReg() const { return statReg_; }
};

class LCD {
	enum Event { MEM_EVENT, LY_COUNT }; enum { NUM_EVENTS = LY_COUNT + 1 };
	enum MemEvent { ONESHOT_LCDSTATIRQ, ONESHOT_UPDATEWY2, MODE1_IRQ, LYC_IRQ, SPRITE_MAP,
	                HDMA_REQ, MODE2_IRQ, MODE0_IRQ }; enum { NUM_MEM_EVENTS = MODE0_IRQ + 1 };
	
	class EventTimes {
		MinKeeper<NUM_EVENTS> eventMin_;
		MinKeeper<NUM_MEM_EVENTS> memEventMin_;
		VideoInterruptRequester memEventRequester_;
		
		void setMemEvent() {
			const unsigned long nmet = nextMemEventTime();
			eventMin_.setValue<MEM_EVENT>(nmet);
			memEventRequester_.setNextEventTime(nmet);
		}
		
	public:
		explicit EventTimes(const VideoInterruptRequester memEventRequester) : memEventRequester_(memEventRequester) {}
		
		Event nextEvent() const { return static_cast<Event>(eventMin_.min()); }
		unsigned long nextEventTime() const { return eventMin_.minValue(); }
		unsigned long operator()(const Event e) const { return eventMin_.value(e); }
		template<Event e> void set(const unsigned long time) { eventMin_.setValue<e>(time); }
		void set(const Event e, const unsigned long time) { eventMin_.setValue(e, time); }
		
		MemEvent nextMemEvent() const { return static_cast<MemEvent>(memEventMin_.min()); }
		unsigned long nextMemEventTime() const { return memEventMin_.minValue(); }
		unsigned long operator()(const MemEvent e) const { return memEventMin_.value(e); }
		template<MemEvent e> void setm(const unsigned long time) { memEventMin_.setValue<e>(time); setMemEvent(); }
		void set(const MemEvent e, const unsigned long time) { memEventMin_.setValue(e, time); setMemEvent(); }
		
		void flagIrq(const unsigned bit) { memEventRequester_.flagIrq(bit); }
		void flagHdmaReq() { memEventRequester_.flagHdmaReq(); }
	};
	
	PPU ppu;
	unsigned long dmgColorsRgb32[3 * 4];
	unsigned char  bgpData[8 * 8];
	unsigned char objpData[8 * 8];

	EventTimes eventTimes_;
	M0Irq m0Irq_;
	LycIrq lycIrq;
	NextM0Time nextM0Time_;

	std::auto_ptr<OsdElement> osdElement;

	unsigned char statReg;
	unsigned char m2IrqStatReg_;
	unsigned char m1IrqStatReg_;

	static void setDmgPalette(unsigned long *palette, const unsigned long *dmgColors, unsigned data);
	void setDmgPaletteColor(unsigned index, unsigned long rgb32);

	void refreshPalettes();
	void setDBuffer();

	void doMode2IrqEvent();
	void event();

	unsigned long m0TimeOfCurrentLine(unsigned long cc);
	bool cgbpAccessible(unsigned long cycleCounter);
	
	void mode3CyclesChange();
	void doCgbBgColorChange(unsigned index, unsigned data, unsigned long cycleCounter);
	void doCgbSpColorChange(unsigned index, unsigned data, unsigned long cycleCounter);

public:
	LCD(const unsigned char *oamram, const unsigned char *vram_in, VideoInterruptRequester memEventRequester);
	void reset(const unsigned char *oamram, const unsigned char *vram, bool cgb);
	void setStatePtrs(SaveState &state);
	void saveState(SaveState &state) const;
	void loadState(const SaveState &state, const unsigned char *oamram);
	void setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned long rgb32);
	void setVideoBuffer(uint_least32_t *videoBuf, int pitch);

	void setOsdElement(std::auto_ptr<OsdElement> osdElement) { this->osdElement = osdElement; }

	void dmgBgPaletteChange(const unsigned data, const unsigned long cycleCounter) {
		update(cycleCounter);
		bgpData[0] = data;
		setDmgPalette(ppu.bgPalette(), dmgColorsRgb32, data);
	}

	void dmgSpPalette1Change(const unsigned data, const unsigned long cycleCounter) {
		update(cycleCounter);
		objpData[0] = data;
		setDmgPalette(ppu.spPalette(), dmgColorsRgb32 + 4, data);
	}

	void dmgSpPalette2Change(const unsigned data, const unsigned long cycleCounter) {
		update(cycleCounter);
		objpData[1] = data;
		setDmgPalette(ppu.spPalette() + 4, dmgColorsRgb32 + 8, data);
	}

	void cgbBgColorChange(unsigned index, const unsigned data, const unsigned long cycleCounter) {
		if (bgpData[index] != data)
			doCgbBgColorChange(index, data, cycleCounter);
	}

	void cgbSpColorChange(unsigned index, const unsigned data, const unsigned long cycleCounter) {
		if (objpData[index] != data)
			doCgbSpColorChange(index, data, cycleCounter);
	}

	unsigned cgbBgColorRead(const unsigned index, const unsigned long cycleCounter) {
		return ppu.cgb() & cgbpAccessible(cycleCounter) ? bgpData[index] : 0xFF;
	}

	unsigned cgbSpColorRead(const unsigned index, const unsigned long cycleCounter) {
		return ppu.cgb() & cgbpAccessible(cycleCounter) ? objpData[index] : 0xFF;
	}

	void updateScreen(bool blanklcd, unsigned long cc);
	void resetCc(unsigned long oldCC, unsigned long newCc);
	void speedChange(unsigned long cycleCounter);
	bool vramAccessible(unsigned long cycleCounter);
	bool oamReadable(unsigned long cycleCounter);
	bool oamWritable(unsigned long cycleCounter);
	void wxChange(unsigned newValue, unsigned long cycleCounter);
	void wyChange(unsigned newValue, unsigned long cycleCounter);
	void oamChange(unsigned long cycleCounter);
	void oamChange(const unsigned char *oamram, unsigned long cycleCounter);
	void scxChange(unsigned newScx, unsigned long cycleCounter);
	void scyChange(unsigned newValue, unsigned long cycleCounter);

	void vramChange(const unsigned long cycleCounter) { update(cycleCounter); }

	unsigned getStat(unsigned lycReg, unsigned long cycleCounter);

	unsigned getLyReg(const unsigned long cycleCounter) {
		unsigned lyReg = 0;

		if (ppu.lcdc() & 0x80) {
			if (cycleCounter >= ppu.lyCounter().time())
				update(cycleCounter);

			lyReg = ppu.lyCounter().ly();
			
			if (lyReg == 153) {
				if (isDoubleSpeed()) {
					if (ppu.lyCounter().time() - cycleCounter <= 456 * 2 - 8)
						lyReg = 0;
				} else
					lyReg = 0;
			} else if (ppu.lyCounter().time() - cycleCounter <= 4)
				++lyReg;
		}

		return lyReg;
	}

	unsigned long nextMode1IrqTime() const { return eventTimes_(MODE1_IRQ); }

	void lcdcChange(unsigned data, unsigned long cycleCounter);
	void lcdstatChange(unsigned data, unsigned long cycleCounter);
	void lycRegChange(unsigned data, unsigned long cycleCounter);
	
	void enableHdma(unsigned long cycleCounter);
	void disableHdma(unsigned long cycleCounter);
	bool hdmaIsEnabled() const { return eventTimes_(HDMA_REQ) != DISABLED_TIME; }
	
	void update(unsigned long cycleCounter);
	
	bool isCgb() const { return ppu.cgb(); }
	bool isDoubleSpeed() const { return ppu.lyCounter().isDoubleSpeed(); }
};

}

#endif
