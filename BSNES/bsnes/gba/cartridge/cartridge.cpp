#include <gba/gba.hpp>

namespace GameBoyAdvance {

#include "eeprom.cpp"
#include "flashrom.cpp"
#include "serialization.cpp"
Cartridge cartridge;

bool Cartridge::load(const string &markup, const stream &memory) {
  information.markup = markup;
  XML::Document document(markup);

  unsigned size = memory.size();
  memory.read(rom.data, min(rom.size, size));
  for(unsigned addr = size; addr < rom.size; addr++) {
    rom.data[addr] = rom.data[Bus::mirror(addr, size)];
  }

  has_sram     = false;
  has_eeprom   = false;
  has_flashrom = false;

  if(document["cartridge"]["ram"].exists()) {
    auto &info = document["cartridge"]["ram"];

    if(info["type"].data == "SRAM" || info["type"].data == "FRAM") {
      has_sram = true;
      ram.size = numeral(info["size"].data);
      ram.mask = ram.size - 1;
      for(unsigned n = 0; n < ram.size; n++) ram.data[n] = 0xff;

      interface->memory.append({2, "save.ram"});
    }

    if(info["type"].data == "EEPROM") {
      has_eeprom = true;
      eeprom.size = numeral(info["size"].data);
      eeprom.bits = eeprom.size <= 512 ? 6 : 14;
      if(eeprom.size == 0) eeprom.size = 8192, eeprom.bits = 0;  //auto-detect size
      eeprom.mask = size > 16 * 1024 * 1024 ? 0x0fffff00 : 0x0f000000;
      eeprom.test = size > 16 * 1024 * 1024 ? 0x0dffff00 : 0x0d000000;
      for(unsigned n = 0; n < eeprom.size; n++) eeprom.data[n] = 0xff;

      interface->memory.append({3, "save.ram"});
    }

    if(info["type"].data == "FlashROM") {
      has_flashrom = true;
      flashrom.id = numeral(info["id"].data);
      flashrom.size = numeral(info["size"].data);
      for(unsigned n = 0; n < flashrom.size; n++) flashrom.data[n] = 0xff;

      interface->memory.append({4, "save.ram"});
    }
  }

  sha256 = nall::sha256(rom.data, size);

  system.load();
  return loaded = true;
}

void Cartridge::unload() {
  if(loaded == false) return;
  loaded = false;
}

void Cartridge::power() {
  eeprom.power();
  flashrom.power();
}

uint8* Cartridge::ram_data() {
  if(has_sram) return ram.data;
  if(has_eeprom) return eeprom.data;
  if(has_flashrom) return flashrom.data;
  return nullptr;
}

unsigned Cartridge::ram_size() {
  if(has_sram) return ram.size;
  if(has_eeprom) return eeprom.size;
  if(has_flashrom) return flashrom.size;
  return 0u;
}

uint32 Cartridge::read(uint8 *data, uint32 addr, uint32 size) {
  if(size == Word) addr &= ~3;
  if(size == Half) addr &= ~1;
  data += addr;
  if(size == Word) return data[0] << 0 | data[1] << 8 | data[2] << 16 | data[3] << 24;
  if(size == Half) return data[0] << 0 | data[1] << 8;
  return data[0];
}

void Cartridge::write(uint8 *data, uint32 addr, uint32 size, uint32 word) {
  if(size == Word) addr &= ~3;
  if(size == Half) addr &= ~1;
  data += addr;
  switch(size) {
  case Word: data[3] = word >> 24;
             data[2] = word >> 16;
  case Half: data[1] = word >>  8;
  case Byte: data[0] = word >>  0;
  }
}

#define RAM_ANALYZE

uint32 Cartridge::read(uint32 addr, uint32 size) {
  #ifdef RAM_ANALYZE
  if((addr & 0x0e000000) == 0x0e000000) {
    static bool once = true;
    if(once) once = false, print("* SRAM/FlashROM read detected\n");
  }
  #endif

  if(has_sram     && (addr & 0x0e000000 ) == 0x0e000000 ) return read(ram.data, addr & ram.mask, size);
  if(has_eeprom   && (addr & eeprom.mask) == eeprom.test) return eeprom.read();
  if(has_flashrom && (addr & 0x0e000000 ) == 0x0e000000 ) return flashrom.read(addr);
  if(addr < 0x0e000000) return read(rom.data, addr & 0x01ffffff, size);
  return cpu.pipeline.fetch.instruction;
}

void Cartridge::write(uint32 addr, uint32 size, uint32 word) {
  #ifdef RAM_ANALYZE
  if((addr & 0x0e000000) == 0x0e000000) {
    static bool once = true;
    if(once) once = false, print("* SRAM/FlashROM write detected\n");
  }
  if((addr & 0x0f000000) == 0x0d000000) {
    static bool once = true;
    if(once) once = false, print("* EEPROM write detected\n");
  }
  if((addr & 0x0e00ffff) == 0x0e005555 && (word & 0xff) == 0xaa) {
    static bool once = true;
    if(once) once = false, print("* FlashROM write detected\n");
  }
  #endif

  if(has_sram     && (addr & 0x0e000000 ) == 0x0e000000 ) return write(ram.data, addr & ram.mask, size, word);
  if(has_eeprom   && (addr & eeprom.mask) == eeprom.test) return eeprom.write(word & 1);
  if(has_flashrom && (addr & 0x0e000000 ) == 0x0e000000 ) return flashrom.write(addr, word);
}

Cartridge::Cartridge() {
  loaded = false;
  rom.data = new uint8[rom.size = 32 * 1024 * 1024];
  ram.data = new uint8[ram.size = 32 * 1024];
  eeprom.data = new uint8[eeprom.size = 8 * 1024];
  flashrom.data = new uint8[flashrom.size = 128 * 1024];
}

Cartridge::~Cartridge() {
  delete[] rom.data;
  delete[] ram.data;
  delete[] eeprom.data;
  delete[] flashrom.data;
}

}
