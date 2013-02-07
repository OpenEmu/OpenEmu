#include "pakinfo_internal.h"
#include <cstring>

namespace gambatte {

enum { FLAG_MULTIPAK = 1, FLAG_HEADER_CHECKSUM_OK = 2 };

static bool isHeaderChecksumOk(unsigned const char header[]) {
	unsigned csum = 0;

	for (int i = 0x134; i < 0x14D; ++i)
		csum -= header[i] + 1;

	return (csum & 0xFF) == header[0x14D];
}

static bool isMbc2(unsigned char h147) { return h147 == 5 || h147 == 6; }

unsigned numRambanksFromH14x(unsigned char h147, unsigned char h149) {
	switch (h149) {
	case 0x00: return isMbc2(h147) ? 1 : 0;
	case 0x01:
	case 0x02: return 1;
	}

	return 4;
}

PakInfo::PakInfo()
: flags_(), rombanks_()
{
	std::memset(h144x_, 0 , sizeof h144x_);
}

PakInfo::PakInfo(bool multipak, unsigned rombanks, unsigned char const romheader[])
: flags_(  multipak * FLAG_MULTIPAK
         + isHeaderChecksumOk(romheader) * FLAG_HEADER_CHECKSUM_OK),
  rombanks_(rombanks)
{
	std::memcpy(h144x_, romheader + 0x144, sizeof h144x_);
}

bool PakInfo::headerChecksumOk() const { return flags_ & FLAG_HEADER_CHECKSUM_OK; }

static char const * h147ToCstr(unsigned char const h147) {
	switch (h147) {
	case 0x00: return "NULL";
	case 0x01: return "MBC1";
	case 0x02: return "MBC1 [RAM]";
	case 0x03: return "MBC1 [RAM,battery]";
	case 0x05: return "MBC2";
	case 0x06: return "MBC2 [battery]";
	case 0x08: return "NULL [RAM]";
	case 0x09: return "NULL [RAM,battery]";
	case 0x0B: return "MMM01";
	case 0x0C: return "MMM01 [RAM]";
	case 0x0D: return "MMM01 [RAM,battery]";
	case 0x0F: return "MBC3 [RTC,battery]";
	case 0x10: return "MBC3 [RAM,RTC,battery]";
	case 0x11: return "MBC3";
	case 0x12: return "MBC3 [RAM]";
	case 0x13: return "MBC3 [RAM,battery]";
	case 0x15: return "MBC4";
	case 0x16: return "MBC4 [RAM]";
	case 0x17: return "MBC4 [RAM,battery]";
	case 0x19: return "MBC5";
	case 0x1A: return "MBC5 [RAM]";
	case 0x1B: return "MBC5 [RAM,battery]";
	case 0x1C: return "MBC5 [rumble]";
	case 0x1D: return "MBC5 [RAM,rumble]";
	case 0x1E: return "MBC5 [RAM,rumble,battery]";
	case 0xFC: return "Pocket Camera";
	case 0xFD: return "Bandai TAMA5";
	case 0xFE: return "HuC3";
	case 0xFF: return "HuC1 [RAM,battery]";
	}

	return "Unknown";
}

std::string const PakInfo::mbc() const {
	std::string h147str = h147ToCstr(h144x_[3]);

	if (flags_ & FLAG_MULTIPAK)
		h147str += " (Custom MultiPak)";

	return h147str;
}

unsigned PakInfo::rambanks() const { return numRambanksFromH14x(h144x_[3], h144x_[5]); }
unsigned PakInfo::rombanks() const { return rombanks_; }

}
