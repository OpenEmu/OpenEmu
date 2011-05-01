#include <../base.hpp>
#include <nall/crc32.hpp>
#include <nall/sha256.hpp>

#define CARTRIDGE_CPP
namespace SNES {

#include "header.cpp"
#include "gameboyheader.cpp"
#include "serialization.cpp"

namespace memory {
  MappedRAM cartrom, cartram, cartrtc;
  MappedRAM bsxflash, bsxram, bsxpram;
  MappedRAM stArom, stAram;
  MappedRAM stBrom, stBram;
  MappedRAM gbrom, gbram, gbrtc;
};

Cartridge cartridge;

void Cartridge::load(Mode cartridge_mode) {
  mode = cartridge_mode;
  read_header(memory::cartrom.data(), memory::cartrom.size());

  if(ram_size > 0) {
    memory::cartram.map(allocate<uint8_t>(ram_size, 0xff), ram_size);
  }

  if(has_srtc || has_spc7110rtc) {
    memory::cartrtc.map(allocate<uint8_t>(20, 0xff), 20);
  }

  if(mode == ModeBsx) {
    memory::bsxram.map (allocate<uint8_t>( 32 * 1024, 0xff),  32 * 1024);
    memory::bsxpram.map(allocate<uint8_t>(512 * 1024, 0xff), 512 * 1024);
  }

  if(mode == ModeSufamiTurbo) {
    if(memory::stArom.data()) memory::stAram.map(allocate<uint8_t>(128 * 1024, 0xff), 128 * 1024);
    if(memory::stBrom.data()) memory::stBram.map(allocate<uint8_t>(128 * 1024, 0xff), 128 * 1024);
  }

  if(mode == ModeSuperGameBoy) {
    if(memory::gbrom.data()) {
      unsigned ram_size = gameboy_ram_size();
      unsigned rtc_size = gameboy_rtc_size();

      if(ram_size) memory::gbram.map(allocate<uint8_t>(ram_size, 0xff), ram_size);
      if(rtc_size) memory::gbrtc.map(allocate<uint8_t>(rtc_size, 0x00), rtc_size);
    }
  }

  memory::cartrom.write_protect(true);
  memory::cartram.write_protect(false);
  memory::cartrtc.write_protect(false);
  memory::bsxflash.write_protect(true);
  memory::bsxram.write_protect(false);
  memory::bsxpram.write_protect(false);
  memory::stArom.write_protect(true);
  memory::stAram.write_protect(false);
  memory::stBrom.write_protect(true);
  memory::stBram.write_protect(false);
  memory::gbrom.write_protect(true);
  memory::gbram.write_protect(false);
  memory::gbrtc.write_protect(false);

  unsigned checksum = ~0;
  for(unsigned n = 0; n < memory::cartrom.size(); n++) checksum = crc32_adjust(checksum, memory::cartrom[n]);
  if(memory::bsxflash.size() != 0 && memory::bsxflash.size() != ~0)
  for(unsigned n = 0; n < memory::bsxflash.size(); n++) checksum = crc32_adjust(checksum, memory::bsxflash[n]);
  if(memory::stArom.size() != 0 && memory::stArom.size() != ~0)
  for(unsigned n = 0; n < memory::stArom.size(); n++) checksum = crc32_adjust(checksum, memory::stArom[n]);
  if(memory::stBrom.size() != 0 && memory::stBrom.size() != ~0)
  for(unsigned n = 0; n < memory::stBrom.size(); n++) checksum = crc32_adjust(checksum, memory::stBrom[n]);
  if(memory::gbrom.size() != 0 && memory::gbrom.size() != ~0)
  for(unsigned n = 0; n < memory::gbrom.size(); n++) checksum = crc32_adjust(checksum, memory::gbrom[n]);
  crc32 = ~checksum;

#if 0
  fprintf(stdout, "crc32  = %.8x\n", (unsigned)crc32);

  sha256_ctx sha;
  uint8_t shahash[32];
  sha256_init(&sha);
  sha256_chunk(&sha, memory::cartrom.data(), memory::cartrom.size());
  sha256_final(&sha);
  sha256_hash(&sha, shahash);

  fprintf(stdout, "sha256 = ");
  for(unsigned i = 0; i < 32; i++) fprintf(stdout, "%.2x", shahash[i]);
  fprintf(stdout, "\n");
#endif

  bus.load_cart();
  system.serialize_init();
  loaded = true;
}

void Cartridge::unload() {
  memory::cartrom.reset();
  memory::cartram.reset();
  memory::cartrtc.reset();
  memory::bsxflash.reset();
  memory::bsxram.reset();
  memory::bsxpram.reset();
  memory::stArom.reset();
  memory::stAram.reset();
  memory::stBrom.reset();
  memory::stBram.reset();
  memory::gbrom.reset();
  memory::gbram.reset();
  memory::gbrtc.reset();

  if(loaded == false) return;
  bus.unload_cart();
  loaded = false;
}

bool Cartridge::has_21fx() const {
  return s21fx.exists();
}

Cartridge::Cartridge() {
  loaded = false;
  unload();
}

Cartridge::~Cartridge() {
  unload();
}

}
