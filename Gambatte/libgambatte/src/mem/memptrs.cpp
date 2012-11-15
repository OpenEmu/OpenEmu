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
#include "memptrs.h"
#include <algorithm>
#include <cstring>

namespace gambatte {

MemPtrs::MemPtrs() :
	memchunk_(0), rambankdata_(0), rdisabledRam_(0), wdisabledRam_(0),
	rsrambankptr_(0), wsrambankptr_(0), oamDmaSrc_(OAM_DMA_SRC_OFF)
{
	std::fill(rmem_, rmem_ + 0x10, static_cast<unsigned char *>(0));
	std::fill(wmem_, wmem_ + 0x10, static_cast<unsigned char *>(0));
	
	romdata_[1] = romdata_[0] = 0;
	wramdata_[1] = wramdata_[0] = 0;
}

MemPtrs::~MemPtrs() {
	delete []memchunk_;
}

void MemPtrs::reset(const unsigned rombanks, const unsigned rambanks, const unsigned wrambanks) {
	delete []memchunk_;
	memchunk_ = new unsigned char[0x4000 + rombanks * 0x4000ul + rambanks * 0x2000ul + wrambanks * 0x1000ul + 0x4000];

	romdata_[0] = memchunk_ + 0x4000;
	rambankdata_ = romdata_[0] + rombanks * 0x4000ul;
	wramdata_[0] = rambankdata_ + rambanks * 0x2000ul;
	rdisabledRam_ = wramdata_[0] + wrambanks * 0x1000ul;
	wdisabledRam_ = rdisabledRam_ + 0x2000;

	std::memset(rdisabledRam_, 0xFF, 0x2000);
	
	oamDmaSrc_ = OAM_DMA_SRC_OFF;
	rmem_[0x3] = rmem_[0x2] = rmem_[0x1] = rmem_[0x0] = romdata_[0];
	rmem_[0xC] = wmem_[0xC] = wramdata_[0] - 0xC000;
	rmem_[0xE] = wmem_[0xE] = wramdata_[0] - 0xE000;
	setRombank(1);
	setRambank(false, false, 0);
	setWrambank(1);
}

void MemPtrs::setRombank(const unsigned bank) {
	romdata_[1] = romdata_[0] + bank * 0x4000ul - 0x4000;
	rmem_[0x7] = rmem_[0x6] = rmem_[0x5] = rmem_[0x4] = romdata_[1];
	disconnectOamDmaAreas();
}

void MemPtrs::setRambank(const bool enableRam, const bool rtcActive, const unsigned rambank) {
	rsrambankptr_ = rdisabledRam_ - 0xA000;
	wsrambankptr_ = wdisabledRam_ - 0xA000;

	if (enableRam) {
		if (rtcActive) {
			rsrambankptr_ = wsrambankptr_ = 0;
		} else if (rambankdata() != rambankdataend())
			rsrambankptr_ = wsrambankptr_ = rambankdata_ + rambank * 0x2000ul - 0xA000;
	}
	
	rmem_[0xB] = rmem_[0xA] = rsrambankptr_;
	wmem_[0xB] = wmem_[0xA] = wsrambankptr_;
	disconnectOamDmaAreas();
}

void MemPtrs::setWrambank(const unsigned bank) {
	wramdata_[1] = wramdata_[0] + ((bank & 0x07) ? (bank & 0x07) : 1) * 0x1000;
	rmem_[0xD] = wmem_[0xD] = wramdata_[1] - 0xD000;
	disconnectOamDmaAreas();
}

void MemPtrs::setOamDmaSrc(const OamDmaSrc oamDmaSrc) {
	rmem_[0x3] = rmem_[0x2] = rmem_[0x1] = rmem_[0x0] = romdata_[0];
	rmem_[0x7] = rmem_[0x6] = rmem_[0x5] = rmem_[0x4] = romdata_[1];
	rmem_[0xB] = rmem_[0xA] = rsrambankptr_;
	wmem_[0xB] = wmem_[0xA] = wsrambankptr_;
	rmem_[0xC] = wmem_[0xC] = wramdata_[0] - 0xC000;
	rmem_[0xD] = wmem_[0xD] = wramdata_[1] - 0xD000;
	rmem_[0xE] = wmem_[0xE] = wramdata_[0] - 0xE000;
	
	oamDmaSrc_ = oamDmaSrc;
	disconnectOamDmaAreas();
}

void MemPtrs::disconnectOamDmaAreas() {
	if (isCgb(*this)) {
		switch (oamDmaSrc_) {
		case OAM_DMA_SRC_ROM:  // fall through
		case OAM_DMA_SRC_SRAM:
		case OAM_DMA_SRC_INVALID:
			std::fill(rmem_, rmem_ + 8, static_cast<unsigned char *>(0));
			rmem_[0xB] = rmem_[0xA] = 0;
			wmem_[0xB] = wmem_[0xA] = 0;
			break;
		case OAM_DMA_SRC_VRAM:
			break;
		case OAM_DMA_SRC_WRAM:
			rmem_[0xE] = rmem_[0xD] = rmem_[0xC] = 0;
			wmem_[0xE] = wmem_[0xD] = wmem_[0xC] = 0;
			break;
		case OAM_DMA_SRC_OFF:
			break;
		}
	} else {
		switch (oamDmaSrc_) {
		case OAM_DMA_SRC_ROM:  // fall through
		case OAM_DMA_SRC_SRAM:
		case OAM_DMA_SRC_WRAM:
		case OAM_DMA_SRC_INVALID:
			std::fill(rmem_, rmem_ + 8, static_cast<unsigned char *>(0));
			rmem_[0xB] = rmem_[0xA] = 0;
			wmem_[0xB] = wmem_[0xA] = 0;
			rmem_[0xE] = rmem_[0xD] = rmem_[0xC] = 0;
			wmem_[0xE] = wmem_[0xD] = wmem_[0xC] = 0;
			break;
		case OAM_DMA_SRC_VRAM:
			break;
		case OAM_DMA_SRC_OFF:
			break;
		}
	}
}

}
