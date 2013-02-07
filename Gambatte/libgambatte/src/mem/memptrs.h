/***************************************************************************
 *   Copyright (C) 2007-2010 by Sindre Aamås                               *
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
#ifndef MEMPTRS_H
#define MEMPTRS_H

namespace gambatte {

enum OamDmaSrc { OAM_DMA_SRC_ROM, OAM_DMA_SRC_SRAM, OAM_DMA_SRC_VRAM,
                 OAM_DMA_SRC_WRAM, OAM_DMA_SRC_INVALID, OAM_DMA_SRC_OFF };

class MemPtrs {
	const unsigned char *rmem_[0x10];
	      unsigned char *wmem_[0x10];
	
	unsigned char *romdata_[2];
	unsigned char *wramdata_[2];
	unsigned char *vrambankptr_;
	unsigned char *rsrambankptr_;
	unsigned char *wsrambankptr_;
	unsigned char *memchunk_;
	unsigned char *rambankdata_;
	unsigned char *wramdataend_;
	
	OamDmaSrc oamDmaSrc_;
	
	MemPtrs(const MemPtrs &);
	MemPtrs & operator=(const MemPtrs &);
	void disconnectOamDmaAreas();
	unsigned char * rdisabledRamw() const { return wramdataend_         ; }
	unsigned char * wdisabledRam()  const { return wramdataend_ + 0x2000; }
public:
	enum RamFlag { READ_EN = 1, WRITE_EN = 2, RTC_EN = 4 };
	
	MemPtrs();
	~MemPtrs();
	void reset(unsigned rombanks, unsigned rambanks, unsigned wrambanks);
	
	const unsigned char * rmem(unsigned area) const { return rmem_[area]; }
	unsigned char * wmem(unsigned area) const { return wmem_[area]; }
	unsigned char * vramdata() const { return rambankdata_ - 0x4000; }
	unsigned char * vramdataend() const { return rambankdata_; }
	unsigned char * romdata() const { return memchunk_ + 0x4000; }
	unsigned char * romdata(unsigned area) const { return romdata_[area]; }
	unsigned char * romdataend() const { return rambankdata_ - 0x4000; }
	unsigned char * wramdata(unsigned area) const { return wramdata_[area]; }
	unsigned char * wramdataend() const { return wramdataend_; }
	unsigned char * rambankdata() const { return rambankdata_; }
	unsigned char * rambankdataend() const { return wramdata_[0]; }
	const unsigned char * rdisabledRam() const { return rdisabledRamw(); }
	const unsigned char * rsrambankptr() const { return rsrambankptr_; }
	unsigned char * wsrambankptr() const { return wsrambankptr_; }
	unsigned char * vrambankptr() const { return vrambankptr_; }
	OamDmaSrc oamDmaSrc() const { return oamDmaSrc_; }
	
	void setRombank0(unsigned bank);
	void setRombank(unsigned bank);
	void setRambank(unsigned ramFlags, unsigned rambank);
	void setVrambank(unsigned bank) { vrambankptr_ = vramdata() + bank * 0x2000ul - 0x8000; }
	void setWrambank(unsigned bank);
	void setOamDmaSrc(OamDmaSrc oamDmaSrc);
};

inline bool isCgb(const MemPtrs &memptrs) {
	return memptrs.wramdataend() - memptrs.wramdata(0) == 0x8000;
}

}

#endif
