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

class omemstream
{
   public:
      omemstream(void *data) : wr_ptr(static_cast<uint8_t*>(data)), has_written(0) {}

      void put(uint8_t data)
      {
         if (wr_ptr)
            *wr_ptr++ = data;
         has_written++;
      }

      void write(const void *data, size_t size)
      {
         if (wr_ptr)
         {
            std::memcpy(wr_ptr, data, size);
            wr_ptr += size;
         }

         has_written += size;
      }

      size_t size() const { return has_written; }
      bool fail() const { return false; }
      bool good() const { return true; }

   private:
      uint8_t *wr_ptr;
      size_t has_written;
};

class imemstream
{
   public:
      imemstream(const void *data) : rd_ptr(static_cast<const uint8_t*>(data)), has_read(0) {}

      uint8_t get()
      {
         uint8_t ret = *rd_ptr++;
         has_read++;
         return ret;
      }

      void read(void *data, size_t size)
      {
         std::memcpy(data, rd_ptr, size);
         rd_ptr += size;
      }

      void ignore(size_t len = 1)
      {
         rd_ptr += len;
         has_read += len;
      }

      void getline(char *data, size_t size, char delim = '\n')
      {
         size_t count = 0;
         while ((count < size - 1) && (*rd_ptr != delim))
         {
            *data++ = *rd_ptr++;
            has_read++;
         }

         rd_ptr++;
         has_read++;
         *data = '\0';
      }

      bool fail() const { return false; }
      bool good() const { return true; }

   private:
      const uint8_t *rd_ptr;
      size_t has_read;
};


namespace {

using namespace gambatte;

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
	void (*save)(omemstream &file, const SaveState &state);
	void (*load)(imemstream &file, SaveState &state);
	unsigned char labelsize;
};

static inline bool operator<(const Saver &l, const Saver &r) {
	return std::strcmp(l.label, r.label) < 0;
}

static void put24(omemstream &file, const unsigned long data) {
	file.put(data >> 16 & 0xFF);
	file.put(data >> 8 & 0xFF);
	file.put(data & 0xFF);
}

static void put32(omemstream &file, const unsigned long data) {
	file.put(data >> 24 & 0xFF);
	file.put(data >> 16 & 0xFF);
	file.put(data >> 8 & 0xFF);
	file.put(data & 0xFF);
}

static void write(omemstream &file, const unsigned char data) {
	static const char inf[] = { 0x00, 0x00, 0x01 };
	
	file.write(inf, sizeof(inf));
	file.put(data & 0xFF);
}

static void write(omemstream &file, const unsigned short data) {
	static const char inf[] = { 0x00, 0x00, 0x02 };
	
	file.write(inf, sizeof(inf));
	file.put(data >> 8 & 0xFF);
	file.put(data & 0xFF);
}

static void write(omemstream &file, const unsigned long data) {
	static const char inf[] = { 0x00, 0x00, 0x04 };
	
	file.write(inf, sizeof(inf));
	put32(file, data);
}

static inline void write(omemstream &file, const bool data) {
	write(file, static_cast<unsigned char>(data));
}

static void write(omemstream &file, const unsigned char *data, const unsigned long sz) {
	put24(file, sz);
	file.write(reinterpret_cast<const char*>(data), sz);
}

static void write(omemstream &file, const bool *data, const unsigned long sz) {
	put24(file, sz);
	
	for (unsigned long i = 0; i < sz; ++i)
		file.put(data[i]);
}

static unsigned long get24(imemstream &file) {
	unsigned long tmp = file.get() & 0xFF;
	
	tmp = tmp << 8 | (file.get() & 0xFF);
	
	return tmp << 8 | (file.get() & 0xFF);
}

static unsigned long read(imemstream &file) {
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

static inline void read(imemstream &file, unsigned char &data) {
	data = read(file) & 0xFF;
}

static inline void read(imemstream &file, unsigned short &data) {
	data = read(file) & 0xFFFF;
}

static inline void read(imemstream &file, unsigned long &data) {
	data = read(file);
}

static inline void read(imemstream &file, bool &data) {
	data = read(file);
}

static void read(imemstream &file, unsigned char *data, unsigned long sz) {
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

static void read(imemstream &file, bool *data, unsigned long sz) {
	const unsigned long size = get24(file);
	
	if (size < sz)
		sz = size;
	
	for (unsigned long i = 0; i < sz; ++i)
		data[i] = file.get();
	
	file.ignore(size - sz);
}

} // anon namespace

namespace gambatte {

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

static void pushSaver(SaverList::list_t &list, const char *label,
		void (*save)(omemstream &file, const SaveState &state),
		void (*load)(imemstream &file, SaveState &state), unsigned labelsize) {
	const Saver saver = { label, save, load, labelsize };
	list.push_back(saver);
}

SaverList::SaverList() {
#define ADD(arg) do { \
	struct Func { \
		static void save(omemstream &file, const SaveState &state) { write(file, state.arg); } \
		static void load(imemstream &file, SaveState &state) { read(file, state.arg); } \
	}; \
	\
	pushSaver(list, label, Func::save, Func::load, sizeof label); \
} while (0)

#define ADDPTR(arg) do { \
	struct Func { \
		static void save(omemstream &file, const SaveState &state) { write(file, state.arg.get(), state.arg.getSz()); } \
		static void load(imemstream &file, SaveState &state) { read(file, state.arg.ptr, state.arg.getSz()); } \
	}; \
	\
	pushSaver(list, label, Func::save, Func::load, sizeof label); \
} while (0)

#define ADDARRAY(arg) do { \
	struct Func { \
		static void save(omemstream &file, const SaveState &state) { write(file, state.arg, sizeof(state.arg)); } \
		static void load(imemstream &file, SaveState &state) { read(file, state.arg, sizeof(state.arg)); } \
	}; \
	\
	pushSaver(list, label, Func::save, Func::load, sizeof label); \
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
	{ static const char label[] = { h,a,l,t,       NUL }; ADD(mem.halted); }
	{ static const char label[] = { v,r,a,m,       NUL }; ADDPTR(mem.vram); }
	{ static const char label[] = { s,r,a,m,       NUL }; ADDPTR(mem.sram); }
	{ static const char label[] = { w,r,a,m,       NUL }; ADDPTR(mem.wram); }
	{ static const char label[] = { h,r,a,m,       NUL }; ADDPTR(mem.ioamhram); }
	{ static const char label[] = { l,d,i,v,u,p,   NUL }; ADD(mem.divLastUpdate); }
	{ static const char label[] = { l,t,i,m,a,u,p, NUL }; ADD(mem.timaLastUpdate); }
	{ static const char label[] = { t,m,a,t,i,m,e, NUL }; ADD(mem.tmatime); }
	{ static const char label[] = { s,e,r,i,a,l,t, NUL }; ADD(mem.nextSerialtime); }
	{ static const char label[] = { l,o,d,m,a,u,p, NUL }; ADD(mem.lastOamDmaUpdate); }
	{ static const char label[] = { m,i,n,i,n,t,t, NUL }; ADD(mem.minIntTime); }
	{ static const char label[] = { u,n,h,a,l,t,t, NUL }; ADD(mem.unhaltTime); }
	{ static const char label[] = { r,o,m,b,a,n,k, NUL }; ADD(mem.rombank); }
	{ static const char label[] = { d,m,a,s,r,c,   NUL }; ADD(mem.dmaSource); }
	{ static const char label[] = { d,m,a,d,s,t,   NUL }; ADD(mem.dmaDestination); }
	{ static const char label[] = { r,a,m,b,a,n,k, NUL }; ADD(mem.rambank); }
	{ static const char label[] = { o,d,m,a,p,o,s, NUL }; ADD(mem.oamDmaPos); }
	{ static const char label[] = { i,m,e,         NUL }; ADD(mem.IME); }
	{ static const char label[] = { s,r,a,m,o,n,   NUL }; ADD(mem.enableRam); }
	{ static const char label[] = { r,a,m,b,m,o,d, NUL }; ADD(mem.rambankMode); }
	{ static const char label[] = { h,d,m,a,       NUL }; ADD(mem.hdmaTransfer); }
	{ static const char label[] = { b,g,p,         NUL }; ADDPTR(ppu.bgpData); }
	{ static const char label[] = { o,b,j,p,       NUL }; ADDPTR(ppu.objpData); }
	{ static const char label[] = { s,p,o,s,b,u,f, NUL }; ADDPTR(ppu.oamReaderBuf); }
	{ static const char label[] = { s,p,s,z,b,u,f, NUL }; ADDPTR(ppu.oamReaderSzbuf); }
	{ static const char label[] = { s,p,a,t,t,r,   NUL }; ADDARRAY(ppu.spAttribList); }
	{ static const char label[] = { s,p,b,y,t,e,NO0, NUL }; ADDARRAY(ppu.spByte0List); }
	{ static const char label[] = { s,p,b,y,t,e,NO1, NUL }; ADDARRAY(ppu.spByte1List); }
	{ static const char label[] = { v,c,y,c,l,e,s, NUL }; ADD(ppu.videoCycles); }
	{ static const char label[] = { e,d,M,NO0,t,i,m, NUL }; ADD(ppu.enableDisplayM0Time); }
	{ static const char label[] = { m,NO0,t,i,m,e, NUL }; ADD(ppu.lastM0Time); }
	{ static const char label[] = { n,m,NO0,i,r,q, NUL }; ADD(ppu.nextM0Irq); }
	{ static const char label[] = { b,g,t,w,       NUL }; ADD(ppu.tileword); }
	{ static const char label[] = { b,g,n,t,w,     NUL }; ADD(ppu.ntileword); }
	{ static const char label[] = { w,i,n,y,p,o,s, NUL }; ADD(ppu.winYPos); }
	{ static const char label[] = { x,p,o,s,       NUL }; ADD(ppu.xpos); }
	{ static const char label[] = { e,n,d,x,       NUL }; ADD(ppu.endx); }
	{ static const char label[] = { p,p,u,r,NO0,   NUL }; ADD(ppu.reg0); }
	{ static const char label[] = { p,p,u,r,NO1,   NUL }; ADD(ppu.reg1); }
	{ static const char label[] = { b,g,a,t,r,b,   NUL }; ADD(ppu.attrib); }
	{ static const char label[] = { b,g,n,a,t,r,b, NUL }; ADD(ppu.nattrib); }
	{ static const char label[] = { p,p,u,s,t,a,t, NUL }; ADD(ppu.state); }
	{ static const char label[] = { n,s,p,r,i,t,e, NUL }; ADD(ppu.nextSprite); }
	{ static const char label[] = { c,s,p,r,i,t,e, NUL }; ADD(ppu.currentSprite); }
	{ static const char label[] = { l,y,c,         NUL }; ADD(ppu.lyc); }
	{ static const char label[] = { m,NO0,l,y,c,   NUL }; ADD(ppu.m0lyc); }
	{ static const char label[] = { o,l,d,w,y,     NUL }; ADD(ppu.oldWy); }
	{ static const char label[] = { w,i,n,d,r,a,w, NUL }; ADD(ppu.winDrawState); }
	{ static const char label[] = { w,s,c,x,       NUL }; ADD(ppu.wscx); }
	{ static const char label[] = { w,e,m,a,s,t,r, NUL }; ADD(ppu.weMaster); }
	{ static const char label[] = { l,c,d,s,i,r,q, NUL }; ADD(ppu.pendingLcdstatIrq); }
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
#undef ADDARRAY

	list.resize(list.size());
	std::sort(list.begin(), list.end());
	
	maxLabelsize_ = 0;
	
	for (std::size_t i = 0; i < list.size(); ++i) {
		if (list[i].labelsize > maxLabelsize_)
			maxLabelsize_ = list[i].labelsize;
	}
}

}

namespace {

static void writeSnapShot(omemstream &file) {
	put24(file, 0);
}

static SaverList list;

} // anon namespace

namespace gambatte {

void StateSaver::saveState(const SaveState &state, void *data) {
   omemstream file(data);
	
	if (file.fail())
		return;
	
	{ static const char ver[] = { 0, 1 }; file.write(ver, sizeof(ver)); }
	
	writeSnapShot(file);
	
	for (SaverList::const_iterator it = list.begin(); it != list.end(); ++it) {
		file.write(it->label, it->labelsize);
		(*it->save)(file, state);
	}
}

bool StateSaver::loadState(SaveState &state, const void *data) {
   imemstream file(data);

   if (file.fail() || file.get() != 0)
      return false;

   file.ignore();
   file.ignore(get24(file));

   const Array<char> labelbuf(list.maxLabelsize());
   const Saver labelbufSaver = { labelbuf, 0, 0, list.maxLabelsize() };

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

size_t StateSaver::stateSize(const SaveState &state) {
   omemstream file(0);

   if (file.fail())
      return 0;

   { static const char ver[] = { 0, 1 }; file.write(ver, sizeof(ver)); }

   writeSnapShot(file);

   for (SaverList::const_iterator it = list.begin(); it != list.end(); ++it) {
      file.write(it->label, it->labelsize);
      (*it->save)(file, state);
   }

   return file.size();
}

}

