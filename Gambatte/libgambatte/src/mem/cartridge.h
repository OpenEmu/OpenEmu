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
#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "memptrs.h"
#include "rtc.h"
#include <string>
#include "file/file.h"

namespace gambatte {

struct SaveState;

class Cartridge {
	MemPtrs memptrs;
	Rtc rtc;
	
	unsigned short rombank;
	unsigned char rambank;
	bool enableRam;
	bool rambankMode;
	
	unsigned rambanks() const { return (memptrs.rambankdataend() - memptrs.rambankdata()) / 0x2000; }
	unsigned rombanks() const { return (memptrs.romdataend()     - memptrs.romdata(0)   ) / 0x4000; }

   bool loadROM(File &file, const bool forceDmg);

   bool hasBattery() const {
      switch (memptrs.romdata(0)[0x147]) {
         case 0x03:
         case 0x06:
         case 0x09:
         case 0x0F:
         case 0x10:
         case 0x13:
         case 0x1B:
         case 0x1E: return true;
         default: return false;
      }
   }

   bool hasRtc() const {
      switch (memptrs.romdata(0)[0x147]) {
         case 0x0F:
         case 0x10: return true;
         default: return false;
      }
   }
	
public:
	Cartridge();
	void setStatePtrs(SaveState &);
	void saveState(SaveState &) const;
	void loadState(const SaveState &);
	
	const unsigned char * rmem(unsigned area) const { return memptrs.rmem(area); }
	unsigned char * wmem(unsigned area) const { return memptrs.wmem(area); }
	unsigned char * romdata(unsigned area) const { return memptrs.romdata(area); }
	unsigned char * wramdata(unsigned area) const { return memptrs.wramdata(area); }
	const unsigned char * rdisabledRam() const { return memptrs.rdisabledRam(); }
	const unsigned char * rsrambankptr() const { return memptrs.rsrambankptr(); }
	unsigned char * wsrambankptr() const { return memptrs.wsrambankptr(); }
	OamDmaSrc oamDmaSrc() const { return memptrs.oamDmaSrc(); }
	
	void setWrambank(unsigned bank) { memptrs.setWrambank(bank); }
	void setOamDmaSrc(OamDmaSrc oamDmaSrc) { memptrs.setOamDmaSrc(oamDmaSrc); }
	
	void mbcWrite(unsigned addr, unsigned data);
	
	bool isCgb() const { return gambatte::isCgb(memptrs); }
	
	void rtcWrite(unsigned data) { rtc.write(data); }
	unsigned char rtcRead() const { return *rtc.getActive(); }
	
	const std::string saveBasePath() const;
	void setSaveDir(const std::string &dir);
	bool loadROM(const void *romdata, unsigned romsize, bool forceDmg);

   void *savedata_ptr()
   {
      // Check ROM header for battery.
      if (hasBattery())
         return memptrs.rambankdata();
      else
         return 0;
   }

   unsigned savedata_size()
   {
      if (hasBattery())
         return memptrs.rambankdataend() - memptrs.rambankdata();
      else
         return 0;
   }

   // Not endian-safe at all, but hey.
   void *rtcdata_ptr()
   {
      if (hasRtc())
         return &rtc.getBaseTime();
      else
         return 0;
   }

   unsigned rtcdata_size()
   { 
      if (hasRtc())
         return sizeof(rtc.getBaseTime());
      else
         return 0;
   }
};

}

#endif
