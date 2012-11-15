/***************************************************************************
 *   Copyright (C) 2007-2010 by Sindre Aamås                               *
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
#ifndef MEMPTRS_H
#define MEMPTRS_H

namespace gambatte {

enum OamDmaSrc { OAM_DMA_SRC_ROM, OAM_DMA_SRC_SRAM, OAM_DMA_SRC_VRAM,
                 OAM_DMA_SRC_WRAM, OAM_DMA_SRC_INVALID, OAM_DMA_SRC_OFF };

class MemPtrs {
	const unsigned char *rmem_[0x10];
	      unsigned char *wmem_[0x10];
	
	unsigned char *memchunk_;
	unsigned char *romdata_[2];
	unsigned char *wramdata_[2];
	unsigned char *rambankdata_;
	unsigned char *rdisabledRam_;
	unsigned char *wdisabledRam_;
	unsigned char *rsrambankptr_;
	unsigned char *wsrambankptr_;
	
	OamDmaSrc oamDmaSrc_;
	
	MemPtrs(const MemPtrs &);
	MemPtrs & operator=(const MemPtrs &);
	void disconnectOamDmaAreas();
public:
	MemPtrs();
	~MemPtrs();
	void reset(unsigned rombanks, unsigned rambanks, unsigned wrambanks);
	
	const unsigned char * rmem(unsigned area) const { return rmem_[area]; }
	unsigned char * wmem(unsigned area) const { return wmem_[area]; }
	unsigned char * romdata(unsigned area) const { return romdata_[area]; }
	unsigned char * romdataend() const { return rambankdata_; }
	unsigned char * wramdata(unsigned area) const { return wramdata_[area]; }
	unsigned char * wramdataend() const { return rdisabledRam_; }
	unsigned char * rambankdata() const { return rambankdata_; }
	unsigned char * rambankdataend() const { return wramdata_[0]; }
	const unsigned char * rdisabledRam() const { return rdisabledRam_; }
	const unsigned char * rsrambankptr() const { return rsrambankptr_; }
	unsigned char * wsrambankptr() const { return wsrambankptr_; }
	OamDmaSrc oamDmaSrc() const { return oamDmaSrc_; }
	
	void setRombank(unsigned bank);
	void setRambank(bool enableRam, bool rtcActive, unsigned rambank);
	void setWrambank(unsigned bank);
	void setOamDmaSrc(OamDmaSrc oamDmaSrc);
};

inline bool isCgb(const MemPtrs &memptrs) {
	return memptrs.wramdataend() - memptrs.wramdata(0) == 0x8000;
}

}

#endif
