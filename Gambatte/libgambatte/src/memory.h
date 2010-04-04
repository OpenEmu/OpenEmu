/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
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
#ifndef MEMORY_H
#define MEMORY_H

class SaveState;

#include "int.h"
#include "video.h"
#include "sound.h"

#include "interrupter.h"
#include "rtc.h"
#include <string>

namespace Gambatte {
class InputStateGetter;
class FilterInfo;
}

class Memory {
public:
	enum { COUNTER_DISABLED = 0xFFFFFFFFu };
	
private:
	enum cartridgetype { plain, mbc1, mbc2, mbc3, mbc5 };
	enum events { HDMA_RESCHEDULE, DMA, INTERRUPTS, BLIT, UNHALT, OAM, END };
	enum irqEvents { /*MODE0, MODE1, MODE2, LYC,*/ TIMA, /*M0RESC,*/ SERIAL };
	
	unsigned char ioamhram[0x200];
	unsigned char vram[0x2000 * 2];
	unsigned char *rmem[0x10];
	unsigned char *wmem[0x10];
	
	unsigned char *memchunk;
	unsigned char *romdata[2];
	unsigned char *wramdata[2];
	unsigned char *rambankdata;
	unsigned char *rdisabled_ram;
	unsigned char *wdisabled_ram;
	unsigned char *oamDmaSrc;
	unsigned char *vrambank;
	unsigned char *rsrambankptr;
	unsigned char *wsrambankptr;
	
	Gambatte::InputStateGetter *getInput;

	unsigned long div_lastUpdate;
	unsigned long tima_lastUpdate;
	unsigned long next_timatime;
	unsigned long next_blittime;
	unsigned long nextIntTime;
	unsigned long minIntTime;
	unsigned long next_dmatime;
	unsigned long next_hdmaReschedule;
	unsigned long next_unhalttime;
	unsigned long next_endtime;
	unsigned long next_irqEventTime;
	unsigned long tmatime;
	unsigned long next_serialtime;
	unsigned long next_eventtime;
	unsigned long lastOamDmaUpdate;
	unsigned long nextOamEventTime;
	
	LCD display;
	PSG sound;
	Interrupter interrupter;
	Rtc rtc;

	events next_event;
	irqEvents next_irqEvent;
	cartridgetype romtype;
	
	std::string defaultSaveBasePath;
	std::string saveDir;
	
	unsigned short rombanks;
	unsigned short rombank;
	unsigned short dmaSource;
	unsigned short dmaDestination;
	
	unsigned char rambank;
	unsigned char rambanks;
	unsigned char oamDmaArea1Lower;
	unsigned char oamDmaArea1Width;
	unsigned char oamDmaArea2Upper;
	unsigned char oamDmaPos;

	bool cgb;
	bool doubleSpeed;
	bool IME;
	bool enable_ram;
	bool rambank_mode;
	bool battery, rtcRom;
	bool hdma_transfer;
	bool active;

	void updateInput();

	void setRombank();
	void setRambank();
	void setBanks();
	void oamDmaInitSetup();
	void setOamDmaArea();
	void updateOamDma(unsigned long cycleCounter);
	void startOamDma(unsigned long cycleCounter);
	void endOamDma(unsigned long cycleCounter);
	void setOamDmaSrc();
	
	unsigned nontrivial_ff_read(unsigned P, unsigned long cycleCounter);
	unsigned nontrivial_read(unsigned P, unsigned long cycleCounter);
	void nontrivial_ff_write(unsigned P, unsigned data, unsigned long cycleCounter);
	void mbc_write(unsigned P, unsigned data);
	void nontrivial_write(unsigned P, unsigned data, unsigned long cycleCounter);

	void set_event();
	void set_irqEvent();
	void update_irqEvents(unsigned long cc);
	void update_tima(unsigned long cycleCounter);
	
	void rescheduleIrq(unsigned long cycleCounter);
	void rescheduleHdmaReschedule();
	
	bool isDoubleSpeed() const { return doubleSpeed; }

public:
	Memory(const Interrupter &interrupter);
	~Memory();

	void setStatePtrs(SaveState &state);
	unsigned long saveState(SaveState &state, unsigned long cc);
	void loadState(const SaveState &state, unsigned long oldCc);
	void loadSavedata();
	void saveSavedata();
	const std::string saveBasePath() const;
	
	void setOsdElement(std::auto_ptr<OsdElement> osdElement) {
		display.setOsdElement(osdElement);
	}

	void speedChange(unsigned long cycleCounter);
	bool isCgb() const { return cgb; }
	bool getIME() const { return IME; }
	unsigned long getNextEventTime() const { return next_eventtime; }
	
	bool isActive() const { return active; }

	void ei(unsigned long cycleCounter);

	void di() {
		IME = 0;
		nextIntTime = COUNTER_DISABLED;
		
		if (next_event == INTERRUPTS)
			set_event();
		
// 		next_eitime=0;
// 		if(next_event==EI) set_event();
	}

	unsigned ff_read(const unsigned P, const unsigned long cycleCounter) {
		return P < 0xFF80 ? nontrivial_ff_read(P, cycleCounter) : ioamhram[P - 0xFE00];
	}

	unsigned read(const unsigned P, const unsigned long cycleCounter) {
		return rmem[P >> 12] ? rmem[P >> 12][P] : nontrivial_read(P, cycleCounter);
	}

	void write(const unsigned P, const unsigned data, const unsigned long cycleCounter) {
		if (wmem[P >> 12])
			wmem[P >> 12][P] = data;
		else
			nontrivial_write(P, data, cycleCounter);
	}
	
	void ff_write(const unsigned P, const unsigned data, const unsigned long cycleCounter) {
		if (((P + 1) & 0xFF) < 0x81)
			nontrivial_ff_write(P, data, cycleCounter);
		else
			ioamhram[P - 0xFE00] = data;
	}

	unsigned long event(unsigned long cycleCounter);
	unsigned long resetCounters(unsigned long cycleCounter);

	bool loadROM(const char* romfile, bool forceDmg);
	void set_savedir(const char *dir);

	void setInputStateGetter(Gambatte::InputStateGetter *getInput) {
		this->getInput = getInput;
	}

	void schedule_unhalt();
	void incEndtime(unsigned long inc);
	void setEndtime(unsigned long cc, unsigned long inc);
	
	void setSoundBuffer(Gambatte::uint_least32_t *const buf) { sound.setBuffer(buf); }
	unsigned fillSoundBuffer(unsigned long cc);
	void setVideoBlitter(Gambatte::VideoBlitter * vb);
	void setVideoFilter(unsigned int n);
	
	void videoBufferChange();
	
	unsigned videoWidth() const {
		return display.videoWidth();
	}
	
	unsigned videoHeight() const {
		return display.videoHeight();
	}
	
	std::vector<const Gambatte::FilterInfo*> filterInfo() const {
		return display.filterInfo();
	}
	
	void setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned long rgb32);
};

#endif
