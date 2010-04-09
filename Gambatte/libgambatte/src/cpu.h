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
#ifndef CPU_H
#define CPU_H

class SaveState;

#include "int.h"
#include "memory.h"

class CPU {
	Memory memory;
	
	unsigned long cycleCounter_;

	unsigned short PC_;
	unsigned short SP;
	
	unsigned HF1, HF2, ZF, CF;

	unsigned char A_, B, C, D, E, /*F,*/ H, L;

	bool skip;
	bool halted;
	
	void process(unsigned long cycles);
	
public:
	
	CPU();
// 	void halt();

// 	unsigned interrupt(unsigned address, unsigned cycleCounter);
	
	void runFor(unsigned long cycles);
	void setStatePtrs(SaveState &state);
	void saveState(SaveState &state);
	void loadState(const SaveState &state);
	
	void loadSavedata() { memory.loadSavedata(); }
	void saveSavedata() { memory.saveSavedata(); }
	
	void setVideoBlitter(Gambatte::VideoBlitter *vb) {
		memory.setVideoBlitter(vb);
	}
	
	void videoBufferChange() {
		memory.videoBufferChange();
	}
	
	unsigned int videoWidth() const {
		return memory.videoWidth();
	}
	
	unsigned int videoHeight() const {
		return memory.videoHeight();
	}
	
	void setVideoFilter(const unsigned int n) {
		memory.setVideoFilter(n);
	}
	
	std::vector<const Gambatte::FilterInfo*> filterInfo() const {
		return memory.filterInfo();
	}
	
	void setInputStateGetter(Gambatte::InputStateGetter *getInput) {
		memory.setInputStateGetter(getInput);
	}
	
	void set_savedir(const char *sdir) {
		memory.set_savedir(sdir);
	}
	
	const std::string saveBasePath() const {
		return memory.saveBasePath();
	}
	
	void setOsdElement(std::auto_ptr<OsdElement> osdElement) {
		memory.setOsdElement(osdElement);
	}
	
	bool load(const char* romfile, bool forceDmg);
	
	void setSoundBuffer(Gambatte::uint_least32_t *const buf) { memory.setSoundBuffer(buf); }
	unsigned fillSoundBuffer() { return memory.fillSoundBuffer(cycleCounter_); }
	
	bool isCgb() const { return memory.isCgb(); }
	
	void setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned rgb32) {
		memory.setDmgPaletteColor(palNum, colorNum, rgb32);
	}
};

#endif
