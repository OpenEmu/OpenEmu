#ifndef NALL_NES_CARTRIDGE_HPP
#define NALL_NES_CARTRIDGE_HPP

#include <nall/sha256.hpp>

namespace nall {

struct FamicomCartridge {
  string markup;
  inline FamicomCartridge(const uint8_t *data, unsigned size);
};

FamicomCartridge::FamicomCartridge(const uint8_t *data, unsigned size) {
  markup = "<?xml version='1.0' encoding='UTF-8'?>\n";
  if(size < 16) return;
  if(data[0] != 'N') return;
  if(data[1] != 'E') return;
  if(data[2] != 'S') return;
  if(data[3] !=  26) return;

  unsigned mapper = ((data[7] >> 4) << 4) | (data[6] >> 4);
  unsigned mirror = ((data[6] & 0x08) >> 2) | (data[6] & 0x01);
  unsigned prgrom = data[4] * 0x4000;
  unsigned chrrom = data[5] * 0x2000;
  unsigned prgram = 0u;
  unsigned chrram = chrrom == 0u ? 8192u : 0u;

  markup.append("<cartridge sha256='", sha256(data, size), "'>\n");

  switch(mapper) {
  default:
    markup.append("  <board type='NES-NROM-256'/>\n");
    markup.append("  <mirror mode='", mirror == 0 ? "horizontal" : "vertical", "'/>\n");
    break;

  case   1:
    markup.append("  <board type='NES-SXROM'/>\n");
    markup.append("  <chip type='MMC1B2'/>\n");
    prgram = 8192;
    break;

  case   2:
    markup.append("  <board type='NES-UOROM'/>\n");
    markup.append("  <mirror mode='", mirror == 0 ? "horizontal" : "vertical", "'/>\n");
    break;

  case   3:
    markup.append("  <board type='NES-CNROM'/>\n");
    markup.append("  <mirror mode='", mirror == 0 ? "horizontal" : "vertical", "'/>\n");
    break;

  case   4:
    //MMC3
    markup.append("  <board type='NES-TLROM'/>\n");
    markup.append("  <chip type='MMC3B'/>\n");
    prgram = 8192;
    //MMC6
  //markup.append("  <board type='NES-HKROM'/>\n");
  //markup.append("  <chip type='MMC6'/>\n");
  //prgram = 1024;
    break;

  case   5:
    markup.append("  <board type='NES-ELROM'/>\n");
    markup.append("  <chip type='MMC5'/>\n");
    prgram = 65536;
    break;

  case   7:
    markup.append(" <board type='NES-AOROM'/>\n");
    break;

  case   9:
    markup.append("  <board type='NES-PNROM'/>\n");
    markup.append("  <chip type='MMC2'/>\n");
    prgram = 8192;
    break;

  case  10:
    markup.append("  <board type='NES-FKROM'/>\n");
    markup.append("  <chip type='MMC4'/>\n");
    prgram = 8192;
    break;

  case  16:
    markup.append("  <board type='BANDAI-FCG'/>\n");
    markup.append("  <chip type='LZ93D50'/>\n");
    break;

  case  21:
  case  23:
  case  25:
    //VRC4
    markup.append("  <board type='KONAMI-VRC-4'/>\n");
    markup.append("  <chip type='VRC4'>\n");
    markup.append("    <pinout a0='1' a1='0'/>\n");
    markup.append("  </chip>\n");
    prgram = 8192;
    break;

  case  22:
    //VRC2
    markup.append("  <board type='KONAMI-VRC-2'/>\n");
    markup.append("  <chip type='VRC2'>\n");
    markup.append("    <pinout a0='0' a1='1'/>\n");
    markup.append("  </chip>\n");
    break;

  case  24:
    markup.append("  <board type='KONAMI-VRC-6'/>\n");
    markup.append("  <chip type='VRC6'/>\n");
    break;

  case  26:
    markup.append("  <board type='KONAMI-VRC-6'/>\n");
    markup.append("  <chip type='VRC6'/>\n");
    prgram = 8192;
    break;

  case  34:
    markup.append("  <board type='NES-BNROM'/>\n");
    markup.append("  <mirror mode='", mirror == 0 ? "horizontal" : "vertical", "'/>\n");
    break;

  case  66:
    markup.append("  <board type='NES-GNROM'/>\n");
    markup.append("  <mirror mode='", mirror == 0 ? "horizontal" : "vertical", "'/>\n");
    break;

  case  69:
    markup.append("  <board type='SUNSOFT-5B'/>\n");
    markup.append("  <chip type='5B'/>\n");
    prgram = 8192;
    break;

  case  73:
    markup.append("  <board type='KONAMI-VRC-3'/>\n");
    markup.append("  <chip type='VRC3'/>\n");
    markup.append("  <mirror mode='", mirror == 0 ? "horizontal" : "vertical", "'/>\n");
    prgram = 8192;
    break;

  case  75:
    markup.append("  <board type='KONAMI-VRC-1'/>\n");
    markup.append("  <chip type='VRC1'/>\n");
    break;

  case  85:
    markup.append("  <board type='KONAMI-VRC-7/'>\n");
    markup.append("  <chip type='VRC7'/>\n");
    prgram = 8192;
    break;
  }

  markup.append("  <prg>\n");
  if(prgrom) markup.append("    <rom size='", prgrom, "'/>\n");
  if(prgram) markup.append("    <ram size='", prgram, "' nonvolatile='true'/>\n");
  markup.append("  </prg>\n");

  markup.append("  <chr>\n");
  if(chrrom) markup.append("    <rom size='", chrrom, "'/>\n");
  if(chrram) markup.append("    <ram size='", chrram, "'/>\n");
  markup.append("  </chr>\n");

  markup.append("</cartridge>\n");
  markup.transform("'", "\"");
}

}

#endif
