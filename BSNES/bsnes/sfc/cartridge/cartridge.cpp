#include <sfc/sfc.hpp>

#define CARTRIDGE_CPP
namespace SuperFamicom {

#include "markup.cpp"
#include "serialization.cpp"

Cartridge cartridge;

void Cartridge::load(const string &markup, const stream &stream) {
  rom.copy(stream);

  region = Region::NTSC;
  ram_size = 0;

  has_gb_slot    = false;
  has_bs_cart    = false;
  has_bs_slot    = false;
  has_st_slots   = false;
  has_nss_dip    = false;
  has_superfx    = false;
  has_sa1        = false;
  has_necdsp     = false;
  has_hitachidsp = false;
  has_armdsp     = false;
  has_srtc       = false;
  has_sdd1       = false;
  has_spc7110    = false;
  has_spc7110rtc = false;
  has_obc1       = false;
  has_msu1       = false;
  has_link       = false;

  parse_markup(markup);
//print(markup, "\n\n");

  //Super Game Boy
  if(cartridge.has_gb_slot()) {
    sha256 = nall::sha256(GameBoy::cartridge.romdata, GameBoy::cartridge.romsize);
  }

  //Broadcast Satellaview
  else if(cartridge.has_bs_cart() && cartridge.has_bs_slot()) {
    sha256 = nall::sha256(bsxflash.memory.data(), bsxflash.memory.size());
  }

  //Sufami Turbo
  else if(cartridge.has_st_slots()) {
    sha256 = nall::sha256(sufamiturbo.slotA.rom.data(), sufamiturbo.slotA.rom.size());
  }

  //Super Famicom
  else {
    sha256 = nall::sha256(rom.data(), rom.size());
  }

  if(ram_size > 0) {
    ram.map(allocate<uint8>(ram_size, 0xff), ram_size);
    interface->memory.append({ID::RAM, "save.ram"});
  }

  rom.write_protect(true);
  ram.write_protect(false);

  system.load();
  loaded = true;
}

void Cartridge::unload() {
  if(loaded == false) return;

  system.unload();
  rom.reset();
  ram.reset();

  loaded = false;
}

Cartridge::Cartridge() {
  loaded = false;
  unload();
}

Cartridge::~Cartridge() {
  unload();
}

}
