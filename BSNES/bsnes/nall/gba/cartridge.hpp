#ifndef NALL_GBA_CARTRIDGE_HPP
#define NALL_GBA_CARTRIDGE_HPP

#include <nall/sha256.hpp>
#include <nall/vector.hpp>

namespace nall {

struct GameBoyAdvanceCartridge {
  string markup;
  string identifiers;
  inline GameBoyAdvanceCartridge(const uint8_t *data, unsigned size);
};

GameBoyAdvanceCartridge::GameBoyAdvanceCartridge(const uint8_t *data, unsigned size) {
  struct Identifier {
    string name;
    unsigned size;
  };
  vector<Identifier> idlist;
  idlist.append({"SRAM_V",      6});
  idlist.append({"SRAM_F_V",    8});
  idlist.append({"EEPROM_V",    8});
  idlist.append({"FLASH_V",     7});
  idlist.append({"FLASH512_V", 10});
  idlist.append({"FLASH1M_V",   9});

  lstring list;
  for(auto &id : idlist) {
    for(signed n = 0; n < size - 16; n++) {
      if(!memcmp(data + n, (const char*)id.name, id.size)) {
        const char *p = (const char*)data + n + id.size;
        if(p[0] >= '0' && p[0] <= '9'
        && p[1] >= '0' && p[1] <= '9'
        && p[2] >= '0' && p[2] <= '9'
        ) {
          char text[16];
          memcpy(text, data + n, id.size + 3);
          text[id.size + 3] = 0;
          list.appendonce(text);
        }
      }
    }
  }
  identifiers = list.concatenate(",");

  markup = "<?xml version='1.0' encoding='UTF-8'?>\n";
  markup.append("<cartridge sha256='", sha256(data, size), "'>\n");
  markup.append("  <rom size='", size, "'/>\n");
  if(0);
  else if(identifiers.beginswith("SRAM_V"    )) markup.append("  <ram type='SRAM' size='32768'/>\n");
  else if(identifiers.beginswith("SRAM_F_V"  )) markup.append("  <ram type='FRAM' size='32768'/>\n");
  else if(identifiers.beginswith("EEPROM_V"  )) markup.append("  <ram type='EEPROM' size='0'/>\n");
  else if(identifiers.beginswith("FLASH_V"   )) markup.append("  <ram type='FlashROM' size='65536'/>\n");
  else if(identifiers.beginswith("FLASH512_V")) markup.append("  <ram type='FlashROM' size='65536'/>\n");
  else if(identifiers.beginswith("FLASH1M_V" )) markup.append("  <ram type='FlashROM' size='131072'/>\n");
  if(identifiers.empty() == false) markup.append("  <!-- detected: ", identifiers, " -->\n");

  markup.append("</cartridge>\n");
  markup.transform("'", "\"");
}

}

#endif
