#include <../base.hpp>
#include <nall/crc32.hpp>
#include <nall/sha256.hpp>

#define CARTRIDGE_CPP
namespace SNES {

#include "header.cpp"
#include "gameboyheader.cpp"

namespace memory {
  MappedRAM cartrom, cartram, cartrtc;
  MappedRAM bsxflash, bsxram, bsxpram;
  MappedRAM stArom, stAram;
  MappedRAM stBrom, stBram;
  MappedRAM gbrom, gbram, gbrtc;
};

Cartridge cartridge;

void Cartridge::load(Mode cartridge_mode) {
  cartinfo_t cartinfo;
  read_header(cartinfo, memory::cartrom.data(), memory::cartrom.size());
  set_cartinfo(cartinfo);

  set(mode, cartridge_mode);

  if(cartinfo.ram_size > 0) {
    memory::cartram.map(allocate<uint8_t>(cartinfo.ram_size, 0xff), cartinfo.ram_size);
  }

  if(cartinfo.srtc || cartinfo.spc7110rtc) {
    memory::cartrtc.map(allocate<uint8_t>(20, 0xff), 20);
  }

  if(mode() == ModeBsx) {
    memory::bsxram.map (allocate<uint8_t>( 32 * 1024, 0xff),  32 * 1024);
    memory::bsxpram.map(allocate<uint8_t>(512 * 1024, 0xff), 512 * 1024);
  }

  if(mode() == ModeSufamiTurbo) {
    if(memory::stArom.data()) memory::stAram.map(allocate<uint8_t>(128 * 1024, 0xff), 128 * 1024);
    if(memory::stBrom.data()) memory::stBram.map(allocate<uint8_t>(128 * 1024, 0xff), 128 * 1024);
  }

  if(mode() == ModeSuperGameBoy) {
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
  set(crc32, ~checksum);

#if 0
  fprintf(stdout, "crc32  = %.8x\n", crc32());

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
  set(loaded, true);
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

  if(loaded() == false) return;
  bus.unload_cart();
  set(loaded, false);
}

Cartridge::Type Cartridge::detect_image_type(uint8_t *data, unsigned size) const {
  cartinfo_t info;
  read_header(info, data, size);
  return info.type;
}

void Cartridge::serialize(serializer &s) {
  if(memory::cartram.size() != 0 && memory::cartram.size() != ~0) {
    s.array(memory::cartram.data(), memory::cartram.size());
  }

  if(memory::cartrtc.size() != 0 && memory::cartrtc.size() != ~0) {
    s.array(memory::cartrtc.data(), memory::cartrtc.size());
  }

  if(memory::bsxram.size() != 0 && memory::bsxram.size() != ~0) {
    s.array(memory::bsxram.data(), memory::bsxram.size());
  }

  if(memory::bsxpram.size() != 0 && memory::bsxpram.size() != ~0) {
    s.array(memory::bsxpram.data(), memory::bsxpram.size());
  }

  if(memory::stAram.size() != 0 && memory::stAram.size() != ~0) {
    s.array(memory::stAram.data(), memory::stAram.size());
  }

  if(memory::stBram.size() != 0 && memory::stBram.size() != ~0) {
    s.array(memory::stBram.data(), memory::stBram.size());
  }

  if(memory::gbram.size() != 0 && memory::gbram.size() != ~0) {
    s.array(memory::gbram.data(), memory::gbram.size());
  }

  if(memory::gbrtc.size() != 0 && memory::gbrtc.size() != ~0) {
    s.array(memory::gbrtc.data(), memory::gbrtc.size());
  }
}

Cartridge::Cartridge() {
  set(loaded, false);
  unload();
}

Cartridge::~Cartridge() {
  unload();
}

void Cartridge::set_cartinfo(const Cartridge::cartinfo_t &source) {
  set(region,         source.region);
  set(mapper,         source.mapper);
  set(dsp1_mapper,    source.dsp1_mapper);

  set(has_bsx_slot,   source.bsx_slot);
  set(has_superfx,    source.superfx);
  set(has_sa1,        source.sa1);
  set(has_srtc,       source.srtc);
  set(has_sdd1,       source.sdd1);
  set(has_spc7110,    source.spc7110);
  set(has_spc7110rtc, source.spc7110rtc);
  set(has_cx4,        source.cx4);
  set(has_dsp1,       source.dsp1);
  set(has_dsp2,       source.dsp2);
  set(has_dsp3,       source.dsp3);
  set(has_dsp4,       source.dsp4);
  set(has_obc1,       source.obc1);
  set(has_st010,      source.st010);
  set(has_st011,      source.st011);
  set(has_st018,      source.st018);
}

//==========
//cartinfo_t
//==========

void Cartridge::cartinfo_t::reset() {
  type        = TypeUnknown;
  mapper      = LoROM;
  dsp1_mapper = DSP1Unmapped;
  region      = NTSC;

  rom_size = 0;
  ram_size = 0;

  bsx_slot   = false;
  superfx    = false;
  sa1        = false;
  srtc       = false;
  sdd1       = false;
  spc7110    = false;
  spc7110rtc = false;
  cx4        = false;
  dsp1       = false;
  dsp2       = false;
  dsp3       = false;
  dsp4       = false;
  obc1       = false;
  st010      = false;
  st011      = false;
  st018      = false;
}

Cartridge::cartinfo_t::cartinfo_t() {
  reset();
}

}
