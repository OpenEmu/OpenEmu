#ifdef BPPU_CPP

void bPPU::latch_counters() {
  regs.hcounter = cpu.hdot();
  regs.vcounter = cpu.vcounter();
  regs.counters_latched = true;
}

uint16 bPPU::get_vram_address() {
  uint16 addr = regs.vram_addr;
  switch(regs.vram_mapping) {
    case 0: break;  //direct mapping
    case 1: addr = (addr & 0xff00) | ((addr & 0x001f) << 3) | ((addr >> 5) & 7); break;
    case 2: addr = (addr & 0xfe00) | ((addr & 0x003f) << 3) | ((addr >> 6) & 7); break;
    case 3: addr = (addr & 0xfc00) | ((addr & 0x007f) << 3) | ((addr >> 7) & 7); break;
  }
  return (addr << 1);
}

//NOTE: all VRAM writes during active display are invalid. Unlike OAM and CGRAM, they will
//not be written anywhere at all. The below address ranges for where writes are invalid have
//been validated on hardware, as has the edge case where the S-CPU MDR can be written if the
//write occurs during the very last clock cycle of vblank.

uint8 bPPU::vram_mmio_read(uint16 addr) {
  uint8 data;

  if(regs.display_disabled == true) {
    data = memory::vram[addr];
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    uint16 ls = ((system.region() == System::NTSC ? 525 : 625) >> 1) - 1;
    if(interlace() && !cpu.field()) ls++;

    if(v == ls && h == 1362) {
      data = 0x00;
    } else if(v < (!overscan() ? 224 : 239)) {
      data = 0x00;
    } else if(v == (!overscan() ? 224 : 239)) {
      if(h == 1362) {
        data = memory::vram[addr];
      } else {
        data = 0x00;
      }
    } else {
      data = memory::vram[addr];
    }
  }

  return data;
}

void bPPU::vram_mmio_write(uint16 addr, uint8 data) {
  if(regs.display_disabled == true) {
    memory::vram[addr] = data;
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    if(v == 0) {
      if(h <= 4) {
        memory::vram[addr] = data;
      } else if(h == 6) {
        memory::vram[addr] = cpu.regs.mdr;
      } else {
        //no write
      }
    } else if(v < (!overscan() ? 225 : 240)) {
      //no write
    } else if(v == (!overscan() ? 225 : 240)) {
      if(h <= 4) {
        //no write
      } else {
        memory::vram[addr] = data;
      }
    } else {
      memory::vram[addr] = data;
    }
  }
}

//NOTE: OAM accesses during active display are rerouted to 0x0218 ... this can be considered
//a hack. The actual address varies during rendering, as the S-PPU reads in data itself for
//processing. Unfortunately, we have yet to determine how this works. The algorithm cannot be
//reverse engineered using a scanline renderer such as this, and at this time, there does not
//exist a more accurate SNES PPU emulator to work from. The only known game to actually access
//OAM during active display is Uniracers. It expects accesses to map to offset 0x0218.
//It was decided by public consensus to map writes to this address to match Uniracers, primarily
//because it is the only game observed to do this, but also because mapping to this address does
//not contradict any of our findings, because we have no findings whatsoever on this behavior.
//Think of this what you will, I openly admit that this is a hack. But it is more accurate than
//writing to the 'expected' address set by $2102,$2103, and will catch problems in software that
//accidentally accesses OAM during active display by virtue of not returning the expected data.

uint8 bPPU::oam_mmio_read(uint16 addr) {
  addr &= 0x03ff;
  if(addr & 0x0200) addr &= 0x021f;
  uint8 data;

  if(regs.display_disabled == true) {
    data = memory::oam[addr];
  } else {
    if(cpu.vcounter() < (!overscan() ? 225 : 240)) {
      data = memory::oam[0x0218];
    } else {
      data = memory::oam[addr];
    }
  }

  return data;
}

void bPPU::oam_mmio_write(uint16 addr, uint8 data) {
  addr &= 0x03ff;
  if(addr & 0x0200) addr &= 0x021f;

  sprite_list_valid = false;

  if(regs.display_disabled == true) {
    memory::oam[addr] = data;
  } else {
    if(cpu.vcounter() < (!overscan() ? 225 : 240)) {
      memory::oam[0x0218] = data;
    } else {
      memory::oam[addr] = data;
    }
  }
}

//NOTE: CGRAM writes during hblank are valid. During active display, the actual address the
//data is written to varies, as the S-PPU itself changes the address. Like OAM, we do not know
//the exact algorithm used, but we have zero known examples of any commercial software that
//attempts to do this. Therefore, the addresses are mapped to 0x01ff. There is nothing special
//about this address, it is simply more accurate to invalidate the 'expected' address than not.

uint8 bPPU::cgram_mmio_read(uint16 addr) {
  addr &= 0x01ff;
  uint8 data;

  if(regs.display_disabled == true) {
    data = memory::cgram[addr];
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    if(v < (!overscan() ? 225 : 240) && h >= 128 && h < 1096) {
      data = memory::cgram[0x01ff] & 0x7f;
    } else {
      data = memory::cgram[addr];
    }
  }

  if(addr & 1) data &= 0x7f;
  return data;
}

void bPPU::cgram_mmio_write(uint16 addr, uint8 data) {
  addr &= 0x01ff;
  if(addr & 1) data &= 0x7f;

  if(regs.display_disabled == true) {
    memory::cgram[addr] = data;
  } else {
    uint16 v = cpu.vcounter();
    uint16 h = cpu.hcounter();
    if(v < (!overscan() ? 225 : 240) && h >= 128 && h < 1096) {
      memory::cgram[0x01ff] = data & 0x7f;
    } else {
      memory::cgram[addr] = data;
    }
  }
}

#endif
