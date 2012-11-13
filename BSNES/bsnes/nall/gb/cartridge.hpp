#ifndef NALL_GB_CARTRIDGE_HPP
#define NALL_GB_CARTRIDGE_HPP

namespace nall {

struct GameBoyCartridge {
  string markup;
  inline GameBoyCartridge(uint8_t *data, unsigned size);

//private:
  struct Information {
    string mapper;
    bool ram;
    bool battery;
    bool rtc;
    bool rumble;

    unsigned romsize;
    unsigned ramsize;
  } info;
};

GameBoyCartridge::GameBoyCartridge(uint8_t *romdata, unsigned romsize) {
  markup = "";
  if(romsize < 0x4000) return;

  info.mapper = "unknown";
  info.ram = false;
  info.battery = false;
  info.rtc = false;
  info.rumble = false;

  info.romsize = 0;
  info.ramsize = 0;

  unsigned base = romsize - 0x8000;
  if(romdata[base + 0x0104] == 0xce && romdata[base + 0x0105] == 0xed
  && romdata[base + 0x0106] == 0x66 && romdata[base + 0x0107] == 0x66
  && romdata[base + 0x0108] == 0xcc && romdata[base + 0x0109] == 0x0d
  && romdata[base + 0x0147] >= 0x0b && romdata[base + 0x0147] <= 0x0d
  ) {
    //MMM01 stores header at bottom of image
    //flip this around for consistency with all other mappers
    uint8_t header[0x8000];
    memcpy(header, romdata + base, 0x8000);
    memmove(romdata + 0x8000, romdata, romsize - 0x8000);
    memcpy(romdata, header, 0x8000);
  }

  switch(romdata[0x0147]) {
    case 0x00: info.mapper = "none";  break;
    case 0x01: info.mapper = "MBC1";  break;
    case 0x02: info.mapper = "MBC1";  info.ram = true; break;
    case 0x03: info.mapper = "MBC1";  info.ram = true; info.battery = true; break;
    case 0x05: info.mapper = "MBC2";  info.ram = true; break;
    case 0x06: info.mapper = "MBC2";  info.ram = true; info.battery = true; break;
    case 0x08: info.mapper = "none";  info.ram = true; break;
    case 0x09: info.mapper = "MBC0";  info.ram = true; info.battery = true; break;
    case 0x0b: info.mapper = "MMM01"; break;
    case 0x0c: info.mapper = "MMM01"; info.ram = true; break;
    case 0x0d: info.mapper = "MMM01"; info.ram = true; info.battery = true; break;
    case 0x0f: info.mapper = "MBC3";  info.rtc = true; info.battery = true; break;
    case 0x10: info.mapper = "MBC3";  info.rtc = true; info.ram = true; info.battery = true; break;
    case 0x11: info.mapper = "MBC3";  break;
    case 0x12: info.mapper = "MBC3";  info.ram = true; break;
    case 0x13: info.mapper = "MBC3";  info.ram = true; info.battery = true; break;
    case 0x19: info.mapper = "MBC5";  break;
    case 0x1a: info.mapper = "MBC5";  info.ram = true; break;
    case 0x1b: info.mapper = "MBC5";  info.ram = true; info.battery = true; break;
    case 0x1c: info.mapper = "MBC5";  info.rumble = true; break;
    case 0x1d: info.mapper = "MBC5";  info.rumble = true; info.ram = true; break;
    case 0x1e: info.mapper = "MBC5";  info.rumble = true; info.ram = true; info.battery = true; break;
    case 0xfc: break;  //Pocket Camera
    case 0xfd: break;  //Bandai TAMA5
    case 0xfe: info.mapper = "HuC3";  break;
    case 0xff: info.mapper = "HuC1";  info.ram = true; info.battery = true; break;
  }

  switch(romdata[0x0148]) { default:
    case 0x00: info.romsize =   2 * 16 * 1024; break;
    case 0x01: info.romsize =   4 * 16 * 1024; break;
    case 0x02: info.romsize =   8 * 16 * 1024; break;
    case 0x03: info.romsize =  16 * 16 * 1024; break;
    case 0x04: info.romsize =  32 * 16 * 1024; break;
    case 0x05: info.romsize =  64 * 16 * 1024; break;
    case 0x06: info.romsize = 128 * 16 * 1024; break;
    case 0x07: info.romsize = 256 * 16 * 1024; break;
    case 0x52: info.romsize =  72 * 16 * 1024; break;
    case 0x53: info.romsize =  80 * 16 * 1024; break;
    case 0x54: info.romsize =  96 * 16 * 1024; break;
  }

  switch(romdata[0x0149]) { default:
    case 0x00: info.ramsize =  0 * 1024; break;
    case 0x01: info.ramsize =  2 * 1024; break;
    case 0x02: info.ramsize =  8 * 1024; break;
    case 0x03: info.ramsize = 32 * 1024; break;
  }

  if(info.mapper == "MBC2") info.ramsize = 512;  //512 x 4-bit

  markup = "<?xml version='1.0' encoding='UTF-8'?>\n";
  markup.append("<cartridge mapper='", info.mapper, "' rtc='", info.rtc, "' rumble='", info.rumble, "'>\n");
  markup.append("  <rom size='0x", hex(romsize), "'/>\n");
  if(info.ramsize > 0) markup.append("  <ram size='0x", hex(info.ramsize), "' nonvolatile='", info.battery, "'/>\n");
  markup.append("</cartridge>\n");
  markup.transform("'", "\"");
}

}

#endif
