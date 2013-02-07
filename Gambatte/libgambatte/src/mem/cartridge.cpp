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
#include "cartridge.h"
#include "file/file.h"
#include "../savestate.h"
#include "pakinfo_internal.h"
#include <cstring>
#include <fstream>

namespace gambatte {

namespace {

static unsigned toMulti64Rombank(const unsigned rombank) {
	return (rombank >> 1 & 0x30) | (rombank & 0xF);
}

class DefaultMbc : public Mbc {
public:
	virtual bool isAddressWithinAreaRombankCanBeMappedTo(unsigned addr, unsigned bank) const {
		return (addr< 0x4000) == (bank == 0);
	}
};

class Mbc0 : public DefaultMbc {
	MemPtrs &memptrs;
	bool enableRam;

public:
	explicit Mbc0(MemPtrs &memptrs)
	: memptrs(memptrs),
	  enableRam(false)
	{
	}

	virtual void romWrite(const unsigned P, const unsigned data) {
		if (P < 0x2000) {
			enableRam = (data & 0xF) == 0xA;
			memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, 0);
		}
	}

	virtual void saveState(SaveState::Mem &ss) const {
		ss.enableRam = enableRam;
	}

	virtual void loadState(const SaveState::Mem &ss) {
		enableRam = ss.enableRam;
		memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, 0);
	}
};

static inline unsigned rambanks(const MemPtrs &memptrs) {
	return static_cast<std::size_t>(memptrs.rambankdataend() - memptrs.rambankdata()) / 0x2000;
}

static inline unsigned rombanks(const MemPtrs &memptrs) {
	return static_cast<std::size_t>(memptrs.romdataend()     - memptrs.romdata()    ) / 0x4000;
}

class Mbc1 : public DefaultMbc {
	MemPtrs &memptrs;
	unsigned char rombank;
	unsigned char rambank;
	bool enableRam;
	bool rambankMode;

	static unsigned adjustedRombank(unsigned bank) { return bank & 0x1F ? bank : bank | 1; }
	void setRambank() const { memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, rambank & (rambanks(memptrs) - 1)); }
	void setRombank() const { memptrs.setRombank(adjustedRombank(rombank & (rombanks(memptrs) - 1))); }

public:
	explicit Mbc1(MemPtrs &memptrs)
	: memptrs(memptrs),
	  rombank(1),
	  rambank(0),
	  enableRam(false),
	  rambankMode(false)
	{
	}

	virtual void romWrite(const unsigned P, const unsigned data) {
		switch (P >> 13 & 3) {
		case 0:
			enableRam = (data & 0xF) == 0xA;
			setRambank();
			break;
		case 1:
			rombank = rambankMode ? data & 0x1F : (rombank & 0x60) | (data & 0x1F);
			setRombank();
			break;
		case 2:
			if (rambankMode) {
				rambank = data & 3;
				setRambank();
			} else {
				rombank = (data << 5 & 0x60) | (rombank & 0x1F);
				setRombank();
			}

			break;
		case 3:
			// Pretty sure this should take effect immediately, but I have a policy not to change old behavior
			// unless I have something (eg. a verified test or a game) that justifies it.
			rambankMode = data & 1;
			break;
		}
	}

	virtual void saveState(SaveState::Mem &ss) const {
		ss.rombank = rombank;
		ss.rambank = rambank;
		ss.enableRam = enableRam;
		ss.rambankMode = rambankMode;
	}

	virtual void loadState(const SaveState::Mem &ss) {
		rombank = ss.rombank;
		rambank = ss.rambank;
		enableRam = ss.enableRam;
		rambankMode = ss.rambankMode;
		setRambank();
		setRombank();
	}
};

class Mbc1Multi64 : public Mbc {
	MemPtrs &memptrs;
	unsigned char rombank;
	bool enableRam;
	bool rombank0Mode;

	static unsigned adjustedRombank(unsigned bank) { return bank & 0x1F ? bank : bank | 1; }

	void setRombank() const {
		if (rombank0Mode) {
			const unsigned rb = toMulti64Rombank(rombank);
			memptrs.setRombank0(rb & 0x30);
			memptrs.setRombank(adjustedRombank(rb));
		} else {
			memptrs.setRombank0(0);
			memptrs.setRombank(adjustedRombank(rombank & (rombanks(memptrs) - 1)));
		}
	}

public:
	explicit Mbc1Multi64(MemPtrs &memptrs)
	: memptrs(memptrs),
	  rombank(1),
	  enableRam(false),
	  rombank0Mode(false)
	{
	}

	virtual void romWrite(const unsigned P, const unsigned data) {
		switch (P >> 13 & 3) {
		case 0:
			enableRam = (data & 0xF) == 0xA;
			memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, 0);
			break;
		case 1:
			rombank = (rombank   & 0x60) | (data    & 0x1F);
			memptrs.setRombank(adjustedRombank(rombank0Mode ? toMulti64Rombank(rombank) : rombank & (rombanks(memptrs) - 1)));
			break;
		case 2:
			rombank = (data << 5 & 0x60) | (rombank & 0x1F);
			setRombank();
			break;
		case 3:
			rombank0Mode = data & 1;
			setRombank();
			break;
		}
	}

	virtual void saveState(SaveState::Mem &ss) const {
		ss.rombank = rombank;
		ss.enableRam = enableRam;
		ss.rambankMode = rombank0Mode;
	}

	virtual void loadState(const SaveState::Mem &ss) {
		rombank = ss.rombank;
		enableRam = ss.enableRam;
		rombank0Mode = ss.rambankMode;
		memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, 0);
		setRombank();
	}
	
	virtual bool isAddressWithinAreaRombankCanBeMappedTo(unsigned addr, unsigned bank) const {
		return (addr < 0x4000) == ((bank & 0xF) == 0);
	}
};

class Mbc2 : public DefaultMbc {
	MemPtrs &memptrs;
	unsigned char rombank;
	bool enableRam;

public:
	explicit Mbc2(MemPtrs &memptrs)
	: memptrs(memptrs),
	  rombank(1),
	  enableRam(false)
	{
	}

	virtual void romWrite(const unsigned P, const unsigned data) {
		switch (P & 0x6100) {
		case 0x0000:
			enableRam = (data & 0xF) == 0xA;
			memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, 0);
			break;
		case 0x2100:
			rombank = data & 0xF;
			memptrs.setRombank(rombank & (rombanks(memptrs) - 1));
			break;
		}
	}

	virtual void saveState(SaveState::Mem &ss) const {
		ss.rombank = rombank;
		ss.enableRam = enableRam;
	}

	virtual void loadState(const SaveState::Mem &ss) {
		rombank = ss.rombank;
		enableRam = ss.enableRam;
		memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, 0);
		memptrs.setRombank(rombank & (rombanks(memptrs) - 1));
	}
};

class Mbc3 : public DefaultMbc {
	MemPtrs &memptrs;
	Rtc *const rtc;
	unsigned char rombank;
	unsigned char rambank;
	bool enableRam;

	void setRambank() const {
		unsigned flags = enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0;
		
		if (rtc) {
			rtc->set(enableRam, rambank);
			
			if (rtc->getActive())
				flags |= MemPtrs::RTC_EN;
		}

		memptrs.setRambank(flags, rambank & (rambanks(memptrs) - 1));
	}

public:
	Mbc3(MemPtrs &memptrs, Rtc *const rtc)
	: memptrs(memptrs),
	  rtc(rtc),
	  rombank(1),
	  rambank(0),
	  enableRam(false)
	{
	}

	virtual void romWrite(const unsigned P, const unsigned data) {
		switch (P >> 13 & 3) {
		case 0:
			enableRam = (data & 0xF) == 0xA;
			setRambank();
			break;
		case 1:
			rombank = data & 0x7F;
			memptrs.setRombank(rombank & (rombanks(memptrs) - 1));
			break;
		case 2:
			rambank = data;
			setRambank();
			break;
		case 3:
			if (rtc)
				rtc->latch(data);

			break;
		}
	}

	virtual void saveState(SaveState::Mem &ss) const {
		ss.rombank = rombank;
		ss.rambank = rambank;
		ss.enableRam = enableRam;
	}

	virtual void loadState(const SaveState::Mem &ss) {
		rombank = ss.rombank;
		rambank = ss.rambank;
		enableRam = ss.enableRam;
		setRambank();
		memptrs.setRombank(rombank & (rombanks(memptrs) - 1));
	}
};

class HuC1 : public DefaultMbc {
	MemPtrs &memptrs;
	unsigned char rombank;
	unsigned char rambank;
	bool enableRam;
	bool rambankMode;

	void setRambank() const {
		memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : MemPtrs::READ_EN,
		                   rambankMode ? rambank & (rambanks(memptrs) - 1) : 0);
	}

	void setRombank() const { memptrs.setRombank((rambankMode ? rombank : rambank << 6 | rombank) & (rombanks(memptrs) - 1)); }

public:
	explicit HuC1(MemPtrs &memptrs)
	: memptrs(memptrs),
	  rombank(1),
	  rambank(0),
	  enableRam(false),
	  rambankMode(false)
	{
	}

	virtual void romWrite(const unsigned P, const unsigned data) {
		switch (P >> 13 & 3) {
		case 0:
			enableRam = (data & 0xF) == 0xA;
			setRambank();
			break;
		case 1:
			rombank = data & 0x3F;
			setRombank();
			break;
		case 2:
			rambank = data & 3;
			rambankMode ? setRambank() : setRombank();
			break;
		case 3:
			rambankMode = data & 1;
			setRambank();
			setRombank();
			break;
		}
	}

	virtual void saveState(SaveState::Mem &ss) const {
		ss.rombank = rombank;
		ss.rambank = rambank;
		ss.enableRam = enableRam;
		ss.rambankMode = rambankMode;
	}

	virtual void loadState(const SaveState::Mem &ss) {
		rombank = ss.rombank;
		rambank = ss.rambank;
		enableRam = ss.enableRam;
		rambankMode = ss.rambankMode;
		setRambank();
		setRombank();
	}
};

class Mbc5 : public DefaultMbc {
	MemPtrs &memptrs;
	unsigned short rombank;
	unsigned char rambank;
	bool enableRam;

	static unsigned adjustedRombank(const unsigned bank) { return bank ? bank : 1; }
	void setRambank() const { memptrs.setRambank(enableRam ? MemPtrs::READ_EN | MemPtrs::WRITE_EN : 0, rambank & (rambanks(memptrs) - 1)); }
	void setRombank() const { memptrs.setRombank(adjustedRombank(rombank & (rombanks(memptrs) - 1))); }

public:
	explicit Mbc5(MemPtrs &memptrs)
	: memptrs(memptrs),
	  rombank(1),
	  rambank(0),
	  enableRam(false)
	{
	}

	virtual void romWrite(const unsigned P, const unsigned data) {
		switch (P >> 13 & 3) {
		case 0:
			enableRam = (data & 0xF) == 0xA;
			setRambank();
			break;
		case 1:
			rombank = P < 0x3000 ? (rombank   & 0x100) |  data
			                     : (data << 8 & 0x100) | (rombank & 0xFF);
			setRombank();
			break;
		case 2:
			rambank = data & 0xF;
			setRambank();
			break;
		case 3:
			break;
		}
	}

	virtual void saveState(SaveState::Mem &ss) const {
		ss.rombank = rombank;
		ss.rambank = rambank;
		ss.enableRam = enableRam;
	}

	virtual void loadState(const SaveState::Mem &ss) {
		rombank = ss.rombank;
		rambank = ss.rambank;
		enableRam = ss.enableRam;
		setRambank();
		setRombank();
	}
};

//static bool hasRtc(const unsigned headerByte0x147) {
//	switch (headerByte0x147) {
//	case 0x0F:
//	case 0x10: return true;
//	default: return false;
//	}
//}

}

void Cartridge::setStatePtrs(SaveState &state) {
	state.mem.vram.set(memptrs.vramdata(), memptrs.vramdataend() - memptrs.vramdata());
	state.mem.sram.set(memptrs.rambankdata(), memptrs.rambankdataend() - memptrs.rambankdata());
	state.mem.wram.set(memptrs.wramdata(0), memptrs.wramdataend() - memptrs.wramdata(0));
}

void Cartridge::saveState(SaveState &state) const {
	mbc->saveState(state.mem);
	rtc.saveState(state);
}

void Cartridge::loadState(const SaveState &state) {
	rtc.loadState(state);
	mbc->loadState(state.mem);
}

//static const std::string stripExtension(const std::string &str) {
//	const std::string::size_type lastDot = str.find_last_of('.');
//	const std::string::size_type lastSlash = str.find_last_of('/');
//
//	if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastSlash < lastDot))
//		return str.substr(0, lastDot);
//
//	return str;
//}
//
//static const std::string stripDir(const std::string &str) {
//	const std::string::size_type lastSlash = str.find_last_of('/');
//
//	if (lastSlash != std::string::npos)
//		return str.substr(lastSlash + 1);
//
//	return str;
//}
//
//const std::string Cartridge::saveBasePath() const {
//	return saveDir.empty() ? defaultSaveBasePath : saveDir + stripDir(defaultSaveBasePath);
//}
//
//void Cartridge::setSaveDir(const std::string &dir) {
//	saveDir = dir;
//
//	if (!saveDir.empty() && saveDir[saveDir.length() - 1] != '/')
//		saveDir += '/';
//}
//
//static void enforce8bit(unsigned char *data, unsigned long sz) {
//	if (static_cast<unsigned char>(0x100))
//		while (sz--)
//			*data++ &= 0xFF;
//}

static unsigned pow2ceil(unsigned n) {
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	++n;

	return n;
}

static bool presumedMulti64Mbc1(unsigned char const header[], unsigned const rombanks) {
	return header[0x147] == 1 && header[0x149] == 0 && rombanks == 64;
}

LoadRes Cartridge::loadROM(std::string const &romfile, bool const forceDmg, bool const multicartCompat) {
	const std::auto_ptr<File> rom(newFileInstance(romfile));

	if (rom->fail())
		return LOADRES_IO_ERROR;
	
	unsigned rambanks = 1;
	unsigned rombanks = 2;
	bool cgb = false;
	enum Cartridgetype { PLAIN, MBC1, MBC2, MBC3, MBC5, HUC1 } type = PLAIN;

	{
		unsigned char header[0x150];
		rom->read(reinterpret_cast<char*>(header), sizeof header);

		switch (header[0x0147]) {
		case 0x00: type = PLAIN; break;
		case 0x01:
		case 0x02:
		case 0x03: type = MBC1; break;
		case 0x05:
		case 0x06: type = MBC2; break;
		case 0x08:
		case 0x09: type = PLAIN; break;
		case 0x0B: 
		case 0x0C:
		case 0x0D: return LOADRES_UNSUPPORTED_MBC_MMM01;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13: type = MBC3; break;
		case 0x15: 
		case 0x16:
		case 0x17: return LOADRES_UNSUPPORTED_MBC_MBC4;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E: type = MBC5; break;
		case 0xFC: return LOADRES_UNSUPPORTED_MBC_POCKET_CAMERA;
		case 0xFD: return LOADRES_UNSUPPORTED_MBC_TAMA5;
		case 0xFE: return LOADRES_UNSUPPORTED_MBC_HUC3;
		case 0xFF: type = HUC1; break;
		default:   return LOADRES_BAD_FILE_OR_UNKNOWN_MBC;
		}

		/*switch (header[0x0148]) {
		case 0x00: rombanks = 2; break;
		case 0x01: rombanks = 4; break;
		case 0x02: rombanks = 8; break;
		case 0x03: rombanks = 16; break;
		case 0x04: rombanks = 32; break;
		case 0x05: rombanks = 64; break;
		case 0x06: rombanks = 128; break;
		case 0x07: rombanks = 256; break;
		case 0x08: rombanks = 512; break;
		case 0x52: rombanks = 72; break;
		case 0x53: rombanks = 80; break;
		case 0x54: rombanks = 96; break;
		default: return -1;
		}*/

		rambanks = numRambanksFromH14x(header[0x147], header[0x149]);
		cgb = header[0x0143] >> 7 & (1 ^ forceDmg);
	}

	std::size_t const filesize = rom->size();
	rombanks = std::max(pow2ceil(filesize / 0x4000), 2u);
	
	//defaultSaveBasePath.clear();
	ggUndoList.clear();
	mbc.reset();
	memptrs.reset(rombanks, rambanks, cgb ? 8 : 2);
	rtc.set(false, 0);

	rom->rewind();
	rom->read(reinterpret_cast<char*>(memptrs.romdata()), (filesize / 0x4000) * 0x4000ul);
	std::memset(memptrs.romdata() + (filesize / 0x4000) * 0x4000ul, 0xFF, (rombanks - filesize / 0x4000) * 0x4000ul);
	//enforce8bit(memptrs.romdata(), rombanks * 0x4000ul);
	
	if (rom->fail())
		return LOADRES_IO_ERROR;
	
	//defaultSaveBasePath = stripExtension(romfile);
	
	switch (type) {
	case PLAIN: mbc.reset(new Mbc0(memptrs)); break;
	case MBC1:
		if (multicartCompat && presumedMulti64Mbc1(memptrs.romdata(), rombanks)) {
			mbc.reset(new Mbc1Multi64(memptrs));
		} else
			mbc.reset(new Mbc1(memptrs));

		break;
	case MBC2: mbc.reset(new Mbc2(memptrs)); break;
	case MBC3: mbc.reset(new Mbc3(memptrs, hasRtc() ? &rtc : 0)); break;
	case MBC5: mbc.reset(new Mbc5(memptrs)); break;
	case HUC1: mbc.reset(new HuC1(memptrs)); break;
	}

	return LOADRES_OK;
}

//static bool hasBattery(const unsigned char headerByte0x147) {
//	switch (headerByte0x147) {
//	case 0x03:
//	case 0x06:
//	case 0x09:
//	case 0x0F:
//	case 0x10:
//	case 0x13:
//	case 0x1B:
//	case 0x1E:
//	case 0xFF: return true;
//	default: return false;
//	}
//}

//void Cartridge::loadSavedata() {
//	const std::string &sbp = saveBasePath();
//
//	if (hasBattery()) {
//		std::ifstream file((sbp + ".sav").c_str(), std::ios::binary | std::ios::in);
//
//		if (file.is_open()) {
//			file.read(reinterpret_cast<char*>(memptrs.rambankdata()), memptrs.rambankdataend() - memptrs.rambankdata());
//			enforce8bit(memptrs.rambankdata(), memptrs.rambankdataend() - memptrs.rambankdata());
//		}
//	}
//
//	if (hasRtc()) {
//		std::ifstream file((sbp + ".rtc").c_str(), std::ios::binary | std::ios::in);
//
//		if (file.is_open()) {
//			unsigned long basetime = file.get() & 0xFF;
//
//			basetime = basetime << 8 | (file.get() & 0xFF);
//			basetime = basetime << 8 | (file.get() & 0xFF);
//			basetime = basetime << 8 | (file.get() & 0xFF);
//
//			rtc.setBaseTime(basetime);
//		}
//	}
//}
//
//void Cartridge::saveSavedata() {
//	const std::string &sbp = saveBasePath();
//
//	if (hasBattery()) {
//		std::ofstream file((sbp + ".sav").c_str(), std::ios::binary | std::ios::out);
//		file.write(reinterpret_cast<const char*>(memptrs.rambankdata()), memptrs.rambankdataend() - memptrs.rambankdata());
//	}
//
//	if (hasRtc()) {
//		std::ofstream file((sbp + ".rtc").c_str(), std::ios::binary | std::ios::out);
//		const unsigned long basetime = rtc.getBaseTime();
//
//		file.put(basetime >> 24 & 0xFF);
//		file.put(basetime >> 16 & 0xFF);
//		file.put(basetime >>  8 & 0xFF);
//		file.put(basetime       & 0xFF);
//	}
//}

static int asHex(const char c) {
	return c >= 'A' ? c - 'A' + 0xA : c - '0';
}

void Cartridge::applyGameGenie(const std::string &code) {
	if (6 < code.length()) {
		const unsigned val = (asHex(code[0]) << 4 | asHex(code[1])) & 0xFF;
		const unsigned addr = (asHex(code[2]) << 8 | asHex(code[4]) << 4 | asHex(code[5]) | (asHex(code[6]) ^ 0xF) << 12) & 0x7FFF;
		unsigned cmp = 0xFFFF;

		if (10 < code.length()) {
			cmp = (asHex(code[8]) << 4 | asHex(code[10])) ^ 0xFF;
			cmp = ((cmp >> 2 | cmp << 6) ^ 0x45) & 0xFF;
		}

		for (unsigned bank = 0; bank < static_cast<std::size_t>(memptrs.romdataend() - memptrs.romdata()) / 0x4000; ++bank) {
			if (mbc->isAddressWithinAreaRombankCanBeMappedTo(addr, bank)
					&& (cmp > 0xFF || memptrs.romdata()[bank * 0x4000ul + (addr & 0x3FFF)] == cmp)) {
				ggUndoList.push_back(AddrData(bank * 0x4000ul + (addr & 0x3FFF), memptrs.romdata()[bank * 0x4000ul + (addr & 0x3FFF)]));
				memptrs.romdata()[bank * 0x4000ul + (addr & 0x3FFF)] = val;
			}
		}
	}
}

void Cartridge::setGameGenie(const std::string &codes) {
	if (loaded()) {
		for (std::vector<AddrData>::reverse_iterator it = ggUndoList.rbegin(), end = ggUndoList.rend(); it != end; ++it) {
			if (memptrs.romdata() + it->addr < memptrs.romdataend())
				memptrs.romdata()[it->addr] = it->data;
		}
		
		ggUndoList.clear();
		
		std::string code;
		for (std::size_t pos = 0; pos < codes.length()
				&& (code = codes.substr(pos, codes.find(';', pos) - pos), true); pos += code.length() + 1) {
			applyGameGenie(code);
		}
	}
}

PakInfo const Cartridge::pakInfo(bool const multipakCompat) const {
	if (loaded()) {
		unsigned const rombs = rombanks(memptrs);
		return PakInfo(multipakCompat && presumedMulti64Mbc1(memptrs.romdata(), rombs),
		               rombs,
		               memptrs.romdata());
	}

	return PakInfo();
}

}
