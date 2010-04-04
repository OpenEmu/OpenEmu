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
#ifndef SAVESTATE_H
#define SAVESTATE_H

#include "int.h"

struct SaveState {
	template<typename T>
	class Ptr {
		T *ptr;
		unsigned long sz;
		
	public:
		Ptr() : ptr(0), sz(0) {}
		const T* get() const { return ptr; }
		unsigned long getSz() const { return sz; }
		void set(T *ptr, const unsigned long sz) { this->ptr = ptr; this->sz = sz; }
		
		friend class SaverList;
		friend void setInitState(SaveState &, bool);
	};

	struct CPU {
		unsigned long cycleCounter;
		unsigned short PC;
		unsigned short SP;
		unsigned char A;
		unsigned char B;
		unsigned char C;
		unsigned char D;
		unsigned char E;
		unsigned char F;
		unsigned char H;
		unsigned char L;
		bool skip;
		bool halted;
	} cpu;
	
	struct Mem {
		Ptr<unsigned char> vram;
		Ptr<unsigned char> sram;
		Ptr<unsigned char> wram;
		Ptr<unsigned char> ioamhram;
		unsigned long div_lastUpdate;
		unsigned long tima_lastUpdate;
		unsigned long tmatime;
		unsigned long next_serialtime;
		unsigned long lastOamDmaUpdate;
		unsigned long minIntTime;
		unsigned short rombank;
		unsigned short dmaSource;
		unsigned short dmaDestination;
		unsigned char rambank;
		unsigned char oamDmaPos;
		bool IME;
		bool enable_ram;
		bool rambank_mode;
		bool hdma_transfer;
	} mem;
	
	struct PPU {
		Ptr<Gambatte::uint_least32_t> drawBuffer;
		Ptr<unsigned char> bgpData;
		Ptr<unsigned char> objpData;
		//SpriteMapper::OamReader
		Ptr<unsigned char> oamReaderBuf;
		Ptr<bool> oamReaderSzbuf;
		
		unsigned long videoCycles;
		unsigned long enableDisplayM0Time;
		unsigned char winYPos;
		unsigned char drawStartCycle;
		unsigned char scReadOffset;
		unsigned char lcdc;
		//ScReader
		unsigned char scx[2];
		unsigned char scy[2];
		//ScxReader
		unsigned char scxAnd7;
		//WeMasterChecker
		bool weMaster;
		//WxReader
		unsigned char wx;
		//Wy
		unsigned char wy;
		bool lycIrqSkip;
	} ppu;
	
	struct SPU {
		struct Duty {
			unsigned long nextPosUpdate;
			unsigned char nr3;
			unsigned char pos;
		};
		
		struct Env {
			unsigned long counter;
			unsigned char volume;
		};
		
		struct LCounter {
			unsigned long counter;
			unsigned short lengthCounter;
		};
		
		struct {
			struct {
				unsigned long counter;
				unsigned short shadow;
				unsigned char nr0;
				bool negging;
			} sweep;
			Duty duty;
			Env env;
			LCounter lcounter;
			unsigned char nr4;
			bool master;
		} ch1;
		
		struct {
			Duty duty;
			Env env;
			LCounter lcounter;
			unsigned char nr4;
			bool master;
		} ch2;
		
		struct {
			Ptr<unsigned char> waveRam;
			LCounter lcounter;
			unsigned long waveCounter;
			unsigned long lastReadTime;
			unsigned char nr3;
			unsigned char nr4;
			unsigned char wavePos;
			unsigned char sampleBuf;
			bool master;
		} ch3;
		
		struct {
			struct {
				unsigned long counter;
				unsigned short reg;
			} lfsr;
			Env env;
			LCounter lcounter;
			unsigned char nr4;
			bool master;
		} ch4;
		
		unsigned long cycleCounter;
	} spu;
	
	struct RTC {
		unsigned long baseTime;
		unsigned long haltTime;
		unsigned char index;
		unsigned char dataDh;
		unsigned char dataDl;
		unsigned char dataH;
		unsigned char dataM;
		unsigned char dataS;
		bool lastLatchData;
	} rtc;
};

#endif
