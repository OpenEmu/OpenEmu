#ifdef SMEMORY_CPP

void sBus::map_generic() {
  switch(cartridge.mapper()) {
    case Cartridge::LoROM: {
      map(MapLinear, 0x00, 0x7f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0x80, 0xff, 0x8000, 0xffff, memory::cartrom);
      map_generic_sram();
    } break;

    case Cartridge::HiROM: {
      map(MapShadow, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0x40, 0x7f, 0x0000, 0xffff, memory::cartrom);
      map(MapShadow, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0xc0, 0xff, 0x0000, 0xffff, memory::cartrom);
      map_generic_sram();
    } break;

    case Cartridge::ExLoROM: {
      map(MapLinear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0x40, 0x7f, 0x0000, 0xffff, memory::cartrom);
      map(MapLinear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0xc0, 0xff, 0x0000, 0xffff, memory::cartrom);
      map_generic_sram();
    } break;

    case Cartridge::ExHiROM: {
      map(MapShadow, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom, 0x400000);
      map(MapLinear, 0x40, 0x7f, 0x0000, 0xffff, memory::cartrom, 0x400000);
      map(MapShadow, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom, 0x000000);
      map(MapLinear, 0xc0, 0xff, 0x0000, 0xffff, memory::cartrom, 0x000000);
      map_generic_sram();
    } break;

    case Cartridge::SuperFXROM: {
      //mapped via SuperFXBus::init();
    } break;

    case Cartridge::SA1ROM: {
      //mapped via SA1Bus::init();
    } break;

    case Cartridge::SPC7110ROM: {
      map(MapDirect, 0x00, 0x00, 0x6000, 0x7fff, spc7110);          //save RAM w/custom logic
      map(MapShadow, 0x00, 0x0f, 0x8000, 0xffff, memory::cartrom);  //program ROM
      map(MapDirect, 0x30, 0x30, 0x6000, 0x7fff, spc7110);          //save RAM w/custom logic
      map(MapDirect, 0x50, 0x50, 0x0000, 0xffff, spc7110);          //decompression MMIO port
      map(MapShadow, 0x80, 0x8f, 0x8000, 0xffff, memory::cartrom);  //program ROM
      map(MapLinear, 0xc0, 0xcf, 0x0000, 0xffff, memory::cartrom);  //program ROM
      map(MapDirect, 0xd0, 0xff, 0x0000, 0xffff, spc7110);          //MMC-controlled data ROM
    } break;

    case Cartridge::BSXROM: {
      //full map is dynamically mapped by:
      //src/chip/bsx/bsx_cart.cpp : BSXCart::update_memory_map();
      map(MapLinear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
    } break;

    case Cartridge::BSCLoROM: {
      map(MapLinear, 0x00, 0x1f, 0x8000, 0xffff, memory::cartrom, 0x000000);
      map(MapLinear, 0x20, 0x3f, 0x8000, 0xffff, memory::cartrom, 0x100000);
      map(MapLinear, 0x70, 0x7f, 0x0000, 0x7fff, memory::cartram, 0x000000);
      map(MapLinear, 0x80, 0x9f, 0x8000, 0xffff, memory::cartrom, 0x200000);
      map(MapLinear, 0xa0, 0xbf, 0x8000, 0xffff, memory::cartrom, 0x100000);
      map(MapLinear, 0xc0, 0xef, 0x0000, 0xffff, bsxflash);
      map(MapLinear, 0xf0, 0xff, 0x0000, 0x7fff, memory::cartram, 0x000000);
    } break;

    case Cartridge::BSCHiROM: {
      map(MapShadow, 0x00, 0x1f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0x20, 0x3f, 0x6000, 0x7fff, memory::cartram);
      map(MapShadow, 0x20, 0x3f, 0x8000, 0xffff, bsxflash);
      map(MapLinear, 0x40, 0x5f, 0x0000, 0xffff, memory::cartrom);
      map(MapLinear, 0x60, 0x7f, 0x0000, 0xffff, bsxflash);
      map(MapShadow, 0x80, 0x9f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0xa0, 0xbf, 0x6000, 0x7fff, memory::cartram);
      map(MapShadow, 0xa0, 0xbf, 0x8000, 0xffff, bsxflash);
      map(MapLinear, 0xc0, 0xdf, 0x0000, 0xffff, memory::cartrom);
      map(MapLinear, 0xe0, 0xff, 0x0000, 0xffff, bsxflash);
    } break;

    case Cartridge::STROM: {
      map(MapLinear, 0x00, 0x1f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0x20, 0x3f, 0x8000, 0xffff, memory::stArom);
      map(MapLinear, 0x40, 0x5f, 0x8000, 0xffff, memory::stBrom);
      map(MapLinear, 0x60, 0x63, 0x8000, 0xffff, memory::stAram);
      map(MapLinear, 0x70, 0x73, 0x8000, 0xffff, memory::stBram);
      map(MapLinear, 0x80, 0x9f, 0x8000, 0xffff, memory::cartrom);
      map(MapLinear, 0xa0, 0xbf, 0x8000, 0xffff, memory::stArom);
      map(MapLinear, 0xc0, 0xdf, 0x8000, 0xffff, memory::stBrom);
      map(MapLinear, 0xe0, 0xe3, 0x8000, 0xffff, memory::stAram);
      map(MapLinear, 0xf0, 0xf3, 0x8000, 0xffff, memory::stBram);
    } break;
  }
}

void sBus::map_generic_sram() {
  if(memory::cartram.size() == 0 || memory::cartram.size() == -1U) { return; }

  map(MapLinear, 0x20, 0x3f, 0x6000, 0x7fff, memory::cartram);
  map(MapLinear, 0xa0, 0xbf, 0x6000, 0x7fff, memory::cartram);

  //research shows only games with very large ROM/RAM sizes require MAD-1 memory mapping of RAM
  //otherwise, default to safer, larger RAM address window
  uint16 addr_hi = (memory::cartrom.size() > 0x200000 || memory::cartram.size() > 32 * 1024) ? 0x7fff : 0xffff;
  map(MapLinear, 0x70, 0x7f, 0x0000, addr_hi, memory::cartram);
  if(cartridge.mapper() != Cartridge::LoROM) return;
  map(MapLinear, 0xf0, 0xff, 0x0000, addr_hi, memory::cartram);
}

#endif
