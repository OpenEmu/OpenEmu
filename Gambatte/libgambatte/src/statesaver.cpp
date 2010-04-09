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
#include "statesaver.h"
#include "savestate.h"
#include "array.h"
#include <vector>
#include <cstring>
#include <algorithm>
#include <fstream>

enum AsciiChar {
	NUL, SOH, STX, ETX, EOT, ENQ, ACK, BEL,  BS, TAB,  LF,  VT,  FF,  CR,  SO,  SI,
	DLE, DC1, DC2, DC3, DC4, NAK, SYN, ETB, CAN,  EM, SUB, ESC,  FS,  GS,  RS,  US,
	 SP, XCL, QOT, HSH, DLR, PRC, AMP, APO, LPA, RPA, AST, PLU, COM, HYP, STP, DIV,
	NO0, NO1, NO2, NO3, NO4, NO5, NO6, NO7, NO8, NO9, CLN, SCL,  LT, EQL,  GT, QTN,
	 AT,   A,   B,   C,   D,   E,   F,   G,   H,   I,   J,   K,   L,   M,   N,   O,
	  P,   Q,   R,   S,   T,   U,   V,   W,   X,   Y,   Z, LBX, BSL, RBX, CAT, UND,
	ACN,   a,   b,   c,   d,   e,   f,   g,   h,   i,   j,   k,   l,   m,   n,   o,
	  p,   q,   r,   s,   t,   u,   v,   w,   x,   y,   z, LBR, BAR, RBR, TLD, DEL
};

struct Saver {
	const char *label;
	void (*save)(std::ofstream &file, const SaveState &state);
	void (*load)(std::ifstream &file, SaveState &state);
	unsigned char labelsize;
};

static inline bool operator<(const Saver &l, const Saver &r) {
	return std::strcmp(l.label, r.label) < 0;
}

static void put24(std::ofstream &file, const unsigned long data) {
	file.put(data >> 16 & 0xFF);
	file.put(data >> 8 & 0xFF);
	file.put(data & 0xFF);
}

static void put32(std::ofstream &file, const unsigned long data) {
	file.put(data >> 24 & 0xFF);
	file.put(data >> 16 & 0xFF);
	file.put(data >> 8 & 0xFF);
	file.put(data & 0xFF);
}

static void write(std::ofstream &file, const unsigned char data) {
	static const char inf[] = { 0x00, 0x00, 0x01 };
	
	file.write(inf, sizeof(inf));
	file.put(data & 0xFF);
}

static void write(std::ofstream &file, const unsigned short data) {
	static const char inf[] = { 0x00, 0x00, 0x02 };
	
	file.write(inf, sizeof(inf));
	file.put(data >> 8 & 0xFF);
	file.put(data & 0xFF);
}

static void write(std::ofstream &file, const unsigned long data) {
	static const char inf[] = { 0x00, 0x00, 0x04 };
	
	file.write(inf, sizeof(inf));
	put32(file, data);
}

static inline void write(std::ofstream &file, const bool data) {
	write(file, static_cast<unsigned char>(data));
}

static void write(std::ofstream &file, const unsigned char *data, const unsigned long sz) {
	put24(file, sz);
	file.write(reinterpret_cast<const char*>(data), sz);
}

static void write(std::ofstream &file, const bool *data, const unsigned long sz) {
	put24(file, sz);
	
	for (unsigned long i = 0; i < sz; ++i)
		file.put(data[i]);
}

static unsigned long get24(std::ifstream &file) {
	unsigned long tmp = file.get() & 0xFF;
	
	tmp = tmp << 8 | (file.get() & 0xFF);
	
	return tmp << 8 | (file.get() & 0xFF);
}

static unsigned long read(std::ifstream &file) {
	unsigned long size = get24(file);
	
	if (size > 4) {
		file.ignore(size - 4);
		size = 4;
	}
	
	unsigned long out = 0;
	
	switch (size) {
	case 4: out = (out | (file.get() & 0xFF)) << 8;
	case 3: out = (out | (file.get() & 0xFF)) << 8;
	case 2: out = (out | (file.get() & 0xFF)) << 8;
	case 1: out = out | (file.get() & 0xFF);
	}
	
	return out;
}

static inline void read(std::ifstream &file, unsigned char &data) {
	data = read(file) & 0xFF;
}

static inline void read(std::ifstream &file, unsigned short &data) {
	data = read(file) & 0xFFFF;
}

static inline void read(std::ifstream &file, unsigned long &data) {
	data = read(file);
}

static inline void read(std::ifstream &file, bool &data) {
	data = read(file);
}

static void read(std::ifstream &file, unsigned char *data, unsigned long sz) {
	const unsigned long size = get24(file);
	
	if (size < sz)
		sz = size;
	
	file.read(reinterpret_cast<char*>(data), sz);
	file.ignore(size - sz);
	
	if (static_cast<unsigned char>(0x100)) {
		for (unsigned long i = 0; i < sz; ++i)
			data[i] &= 0xFF;
	}
}

static void read(std::ifstream &file, bool *data, unsigned long sz) {
	const unsigned long size = get24(file);
	
	if (size < sz)
		sz = size;
	
	for (unsigned long i = 0; i < sz; ++i)
		data[i] = file.get();
	
	file.ignore(size - sz);
}

class SaverList {
public:
	typedef std::vector<Saver> list_t;
	typedef list_t::const_iterator const_iterator;
	
private:
	list_t list;
	unsigned char maxLabelsize_;
	
public:
	SaverList();
	const_iterator begin() const { return list.begin(); }
	const_iterator end() const { return list.end(); }
	unsigned maxLabelsize() const { return maxLabelsize_; }
};

SaverList::SaverList() {
#define ADD(arg) do { \
	struct Func { \
		static void save(std::ofstream &file, const SaveState &state) { write(file, state.arg); } \
		static void load(std::ifstream &file, SaveState &state) { read(file, state.arg); } \
	}; \
	\
	Saver saver = { label, Func::save, Func::load, sizeof label }; \
	list.push_back(saver); \
} while (0)

#define ADDPTR(arg) do { \
	struct Func { \
		static void save(std::ofstream &file, const SaveState &state) { write(file, state.arg.get(), state.arg.getSz()); } \
		static void load(std::ifstream &file, SaveState &state) { read(file, state.arg.ptr, state.arg.getSz()); } \
	}; \
	\
	Saver saver = { label, Func::save, Func::load, sizeof label }; \
	list.push_back(saver); \
} while (0)
	
	{ static const char label[] = { c,c,           NUL }; ADD(cpu.cycleCounter); }
	{ static const char label[] = { p,c,           NUL }; ADD(cpu.PC); }
	{ static const char label[] = { s,p,           NUL }; ADD(cpu.SP); }
	{ static const char label[] = { a,             NUL }; ADD(cpu.A); }
	{ static const char label[] = { b,             NUL }; ADD(cpu.B); }
	{ static const char label[] = { c,             NUL }; ADD(cpu.C); }
	{ static const char label[] = { d,             NUL }; ADD(cpu.D); }
	{ static const char label[] = { e,             NUL }; ADD(cpu.E); }
	{ static const char label[] = { f,             NUL }; ADD(cpu.F); }
	{ static const char label[] = { h,             NUL }; ADD(cpu.H); }
	{ static const char label[] = { l,             NUL }; ADD(cpu.L); }
	{ static const char label[] = { s,k,i,p,       NUL }; ADD(cpu.skip); }
	{ static const char label[] = { h,a,l,t,       NUL }; ADD(cpu.halted); }
	{ static const char label[] = { v,r,a,m,       NUL }; ADDPTR(mem.vram); }
	{ static const char label[] = { s,r,a,m,       NUL }; ADDPTR(mem.sram); }
	{ static const char label[] = { w,r,a,m,       NUL }; ADDPTR(mem.wram); }
	{ static const char label[] = { h,r,a,m,       NUL }; ADDPTR(mem.ioamhram); }
	{ static const char label[] = { l,d,i,v,u,p,   NUL }; ADD(mem.div_lastUpdate); }
	{ static const char label[] = { l,t,i,m,a,u,p, NUL }; ADD(mem.tima_lastUpdate); }
	{ static const char label[] = { t,m,a,t,i,m,e, NUL }; ADD(mem.tmatime); }
	{ static const char label[] = { s,e,r,i,a,l,t, NUL }; ADD(mem.next_serialtime); }
	{ static const char label[] = { l,o,d,m,a,u,p, NUL }; ADD(mem.lastOamDmaUpdate); }
	{ static const char label[] = { m,i,n,i,n,t,t, NUL }; ADD(mem.minIntTime); }
	{ static const char label[] = { r,o,m,b,a,n,k, NUL }; ADD(mem.rombank); }
	{ static const char label[] = { d,m,a,s,r,c,   NUL }; ADD(mem.dmaSource); }
	{ static const char label[] = { d,m,a,d,s,t,   NUL }; ADD(mem.dmaDestination); }
	{ static const char label[] = { r,a,m,b,a,n,k, NUL }; ADD(mem.rambank); }
	{ static const char label[] = { o,d,m,a,p,o,s, NUL }; ADD(mem.oamDmaPos); }
	{ static const char label[] = { i,m,e,         NUL }; ADD(mem.IME); }
	{ static const char label[] = { s,r,a,m,o,n,   NUL }; ADD(mem.enable_ram); }
	{ static const char label[] = { r,a,m,b,m,o,d, NUL }; ADD(mem.rambank_mode); }
	{ static const char label[] = { h,d,m,a,       NUL }; ADD(mem.hdma_transfer); }
	{ static const char label[] = { b,g,p,         NUL }; ADDPTR(ppu.bgpData); }
	{ static const char label[] = { o,b,j,p,       NUL }; ADDPTR(ppu.objpData); }
	{ static const char label[] = { s,p,o,s,b,u,f, NUL }; ADDPTR(ppu.oamReaderBuf); }
	{ static const char label[] = { s,p,s,z,b,u,f, NUL }; ADDPTR(ppu.oamReaderSzbuf); }
	{ static const char label[] = { v,c,y,c,l,e,s, NUL }; ADD(ppu.videoCycles); }
	{ static const char label[] = { e,d,M,NO0,t,i,m, NUL }; ADD(ppu.enableDisplayM0Time); }
	{ static const char label[] = { w,i,n,y,p,o,s, NUL }; ADD(ppu.winYPos); }
	{ static const char label[] = { d,r,a,w,c,y,c, NUL }; ADD(ppu.drawStartCycle); }
	{ static const char label[] = { s,c,r,d,c,y,c, NUL }; ADD(ppu.scReadOffset); }
	{ static const char label[] = { l,c,d,c,       NUL }; ADD(ppu.lcdc); }
	{ static const char label[] = { s,c,x,NO0,     NUL }; ADD(ppu.scx[0]); }
	{ static const char label[] = { s,c,x,NO1,     NUL }; ADD(ppu.scx[1]); }
	{ static const char label[] = { s,c,y,NO0,     NUL }; ADD(ppu.scy[0]); }
	{ static const char label[] = { s,c,y,NO1,     NUL }; ADD(ppu.scy[1]); }
	{ static const char label[] = { s,c,x,AMP,NO7, NUL }; ADD(ppu.scxAnd7); }
	{ static const char label[] = { w,e,m,a,s,t,r, NUL }; ADD(ppu.weMaster); }
	{ static const char label[] = { w,x,           NUL }; ADD(ppu.wx); }
	{ static const char label[] = { w,y,           NUL }; ADD(ppu.wy); }
	{ static const char label[] = { l,y,c,s,k,i,p, NUL }; ADD(ppu.lycIrqSkip); }
	{ static const char label[] = { s,p,u,c,n,t,r, NUL }; ADD(spu.cycleCounter); }
	{ static const char label[] = { s,w,p,c,n,t,r, NUL }; ADD(spu.ch1.sweep.counter); }
	{ static const char label[] = { s,w,p,s,h,d,w, NUL }; ADD(spu.ch1.sweep.shadow); }
	{ static const char label[] = { s,w,p,n,e,g,   NUL }; ADD(spu.ch1.sweep.negging); }
	{ static const char label[] = { d,u,t,NO1,c,t,r, NUL }; ADD(spu.ch1.duty.nextPosUpdate); }
	{ static const char label[] = { d,u,t,NO1,p,o,s, NUL }; ADD(spu.ch1.duty.pos); }
	{ static const char label[] = { e,n,v,NO1,c,t,r, NUL }; ADD(spu.ch1.env.counter); }
	{ static const char label[] = { e,n,v,NO1,v,o,l, NUL }; ADD(spu.ch1.env.volume); }
	{ static const char label[] = { l,e,n,NO1,c,t,r, NUL }; ADD(spu.ch1.lcounter.counter); }
	{ static const char label[] = { l,e,n,NO1,v,a,l, NUL }; ADD(spu.ch1.lcounter.lengthCounter); }
	{ static const char label[] = { n,r,NO1,NO0,       NUL }; ADD(spu.ch1.sweep.nr0); }
	{ static const char label[] = { n,r,NO1,NO3,       NUL }; ADD(spu.ch1.duty.nr3); }
	{ static const char label[] = { n,r,NO1,NO4,       NUL }; ADD(spu.ch1.nr4); }
	{ static const char label[] = { c,NO1,m,a,s,t,r, NUL }; ADD(spu.ch1.master); }
	{ static const char label[] = { d,u,t,NO2,c,t,r, NUL }; ADD(spu.ch2.duty.nextPosUpdate); }
	{ static const char label[] = { d,u,t,NO2,p,o,s, NUL }; ADD(spu.ch2.duty.pos); }
	{ static const char label[] = { e,n,v,NO2,c,t,r, NUL }; ADD(spu.ch2.env.counter); }
	{ static const char label[] = { e,n,v,NO2,v,o,l, NUL }; ADD(spu.ch2.env.volume); }
	{ static const char label[] = { l,e,n,NO2,c,t,r, NUL }; ADD(spu.ch2.lcounter.counter); }
	{ static const char label[] = { l,e,n,NO2,v,a,l, NUL }; ADD(spu.ch2.lcounter.lengthCounter); }
	{ static const char label[] = { n,r,NO2,NO3,       NUL }; ADD(spu.ch2.duty.nr3); }
	{ static const char label[] = { n,r,NO2,NO4,       NUL }; ADD(spu.ch2.nr4); }
	{ static const char label[] = { c,NO2,m,a,s,t,r, NUL }; ADD(spu.ch2.master); }
	{ static const char label[] = { w,a,v,e,r,a,m, NUL }; ADDPTR(spu.ch3.waveRam); }
	{ static const char label[] = { l,e,n,NO3,c,t,r, NUL }; ADD(spu.ch3.lcounter.counter); }
	{ static const char label[] = { l,e,n,NO3,v,a,l, NUL }; ADD(spu.ch3.lcounter.lengthCounter); }
	{ static const char label[] = { w,a,v,e,c,t,r, NUL }; ADD(spu.ch3.waveCounter); }
	{ static const char label[] = { l,w,a,v,r,d,t, NUL }; ADD(spu.ch3.lastReadTime); }
	{ static const char label[] = { w,a,v,e,p,o,s, NUL }; ADD(spu.ch3.wavePos); }
	{ static const char label[] = { w,a,v,s,m,p,l, NUL }; ADD(spu.ch3.sampleBuf); }
	{ static const char label[] = { n,r,NO3,NO3,       NUL }; ADD(spu.ch3.nr3); }
	{ static const char label[] = { n,r,NO3,NO4,       NUL }; ADD(spu.ch3.nr4); }
	{ static const char label[] = { c,NO3,m,a,s,t,r, NUL }; ADD(spu.ch3.master); }
	{ static const char label[] = { l,f,s,r,c,t,r, NUL }; ADD(spu.ch4.lfsr.counter); }
	{ static const char label[] = { l,f,s,r,r,e,g, NUL }; ADD(spu.ch4.lfsr.reg); }
	{ static const char label[] = { e,n,v,NO4,c,t,r, NUL }; ADD(spu.ch4.env.counter); }
	{ static const char label[] = { e,n,v,NO4,v,o,l, NUL }; ADD(spu.ch4.env.volume); }
	{ static const char label[] = { l,e,n,NO4,c,t,r, NUL }; ADD(spu.ch4.lcounter.counter); }
	{ static const char label[] = { l,e,n,NO4,v,a,l, NUL }; ADD(spu.ch4.lcounter.lengthCounter); }
	{ static const char label[] = { n,r,NO4,NO4,       NUL }; ADD(spu.ch4.nr4); }
	{ static const char label[] = { c,NO4,m,a,s,t,r, NUL }; ADD(spu.ch4.master); }
	{ static const char label[] = { r,t,c,b,a,s,e, NUL }; ADD(rtc.baseTime); }
	{ static const char label[] = { r,t,c,h,a,l,t, NUL }; ADD(rtc.haltTime); }
	{ static const char label[] = { r,t,c,i,n,d,x, NUL }; ADD(rtc.index); }
	{ static const char label[] = { r,t,c,d,h,     NUL }; ADD(rtc.dataDh); }
	{ static const char label[] = { r,t,c,d,l,     NUL }; ADD(rtc.dataDl); }
	{ static const char label[] = { r,t,c,h,       NUL }; ADD(rtc.dataH); }
	{ static const char label[] = { r,t,c,m,       NUL }; ADD(rtc.dataM); }
	{ static const char label[] = { r,t,c,s,       NUL }; ADD(rtc.dataS); }
	{ static const char label[] = { r,t,c,l,l,d,   NUL }; ADD(rtc.lastLatchData); }
	
#undef ADD
#undef ADDPTR
#undef ADDTIME

	list.resize(list.size());
	std::sort(list.begin(), list.end());
	
	maxLabelsize_ = 0;
	
	for (std::size_t i = 0; i < list.size(); ++i) {
		if (list[i].labelsize > maxLabelsize_)
			maxLabelsize_ = list[i].labelsize;
	}
}

static void writeSnapShot(std::ofstream &file, const Gambatte::uint_least32_t *pixels, const unsigned pitch) {
	put24(file, pixels ? StateSaver::SS_WIDTH * StateSaver::SS_HEIGHT * sizeof(Gambatte::uint_least32_t) : 0);
	
	if (pixels) {
		Gambatte::uint_least32_t buf[StateSaver::SS_WIDTH];
		
		for (unsigned h = StateSaver::SS_HEIGHT; h--;) {
			for (unsigned x = 0; x < StateSaver::SS_WIDTH; ++x) {
				unsigned long rb = 0;
				unsigned long g = 0;
				
				static const unsigned w[StateSaver::SS_DIV] = { 3, 5, 5, 3 };
				
				for (unsigned y = 0; y < StateSaver::SS_DIV; ++y)
					for (unsigned xx = 0; xx < StateSaver::SS_DIV; ++xx) {
						rb += (pixels[x * StateSaver::SS_DIV + y * pitch + xx] & 0xFF00FF) * w[y] * w[xx];
						g += (pixels[x * StateSaver::SS_DIV + y * pitch + xx] & 0x00FF00) * w[y] * w[xx];
					}
				
				buf[x] = (rb >> 8 & 0xFF00FF) | (g >> 8 & 0x00FF00);
			}
			
			file.write(reinterpret_cast<const char*>(buf), sizeof(buf));
			pixels += pitch * StateSaver::SS_DIV;
		}
	}
}

static SaverList list;

void StateSaver::saveState(const SaveState &state, const char *filename) {
	std::ofstream file(filename, std::ios_base::binary);
	
	if (file.fail())
		return;
	
	{ static const char ver[] = { 0, 0 }; file.write(ver, sizeof(ver)); }
	
	writeSnapShot(file, state.ppu.drawBuffer.get(), state.ppu.drawBuffer.getSz() / 144);
	
	for (SaverList::const_iterator it = list.begin(); it != list.end(); ++it) {
		file.write(it->label, it->labelsize);
		(*it->save)(file, state);
	}
}

bool StateSaver::loadState(SaveState &state, const char *filename) {
	std::ifstream file(filename, std::ios_base::binary);
	
	if (file.fail() || file.get() != 0)
		return false;
	
	file.ignore();
	file.ignore(get24(file));
	
	Array<char> labelbuf(list.maxLabelsize());
	const Saver labelbufSaver = { label: labelbuf, save: 0, load: 0, labelsize: list.maxLabelsize() };
	
	SaverList::const_iterator done = list.begin();
	
	while (file.good() && done != list.end()) {
		file.getline(labelbuf, list.maxLabelsize(), NUL);
		
		SaverList::const_iterator it = done;
		
		if (std::strcmp(labelbuf, it->label)) {
			it = std::lower_bound(it + 1, list.end(), labelbufSaver);
			
			if (it == list.end() || std::strcmp(labelbuf, it->label)) {
				file.ignore(get24(file));
				continue;
			}
		} else
			++done;
		
		(*it->load)(file, state);
	}
	
	state.cpu.cycleCounter &= 0x7FFFFFFF;
	state.spu.cycleCounter &= 0x7FFFFFFF;
	
	return true;
}
