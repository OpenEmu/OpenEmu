#ifdef SUPERFX_CPP

uint8 SuperFX::mmio_read(unsigned addr) {
  scheduler.sync_cpucop();
  addr &= 0xffff;

  if(addr >= 0x3100 && addr <= 0x32ff) {
    return cache_mmio_read(addr - 0x3100);
  }

  if(addr >= 0x3000 && addr <= 0x301f) {
    return regs.r[(addr >> 1) & 15] >> ((addr & 1) << 3);
  }

  switch(addr) {
    case 0x3030: {
      return regs.sfr >> 0;
    }

    case 0x3031: {
      uint8 r = regs.sfr >> 8;
      regs.sfr.irq = 0;
      cpu.regs.irq = 0;
      return r;
    }

    case 0x3034: {
      return regs.pbr;
    }

    case 0x3036: {
      return regs.rombr;
    }

    case 0x303b: {
      return regs.vcr;
    }

    case 0x303c: {
      return regs.rambr;
    }

    case 0x303e: {
      return regs.cbr >> 0;
    }

    case 0x303f: {
      return regs.cbr >> 8;
    }
  }

  return 0x00;
}

void SuperFX::mmio_write(unsigned addr, uint8 data) {
  scheduler.sync_cpucop();
  addr &= 0xffff;

  if(addr >= 0x3100 && addr <= 0x32ff) {
    return cache_mmio_write(addr - 0x3100, data);
  }

  if(addr >= 0x3000 && addr <= 0x301f) {
    unsigned n = (addr >> 1) & 15;
    if((addr & 1) == 0) {
      regs.r[n] = (regs.r[n] & 0xff00) | data;
    } else {
      regs.r[n] = (data << 8) | (regs.r[n] & 0xff);
    }

    if(addr == 0x301f) regs.sfr.g = 1;
    return;
  }

  switch(addr) {
    case 0x3030: {
      bool g = regs.sfr.g;
      regs.sfr = (regs.sfr & 0xff00) | (data << 0);
      if(g == 1 && regs.sfr.g == 0) {
        regs.cbr = 0x0000;
        cache_flush();
      }
    } break;

    case 0x3031: {
      regs.sfr = (data << 8) | (regs.sfr & 0x00ff);
    } break;

    case 0x3033: {
      regs.bramr = data;
    } break;

    case 0x3034: {
      regs.pbr = data & 0x7f;
      cache_flush();
    } break;

    case 0x3037: {
      regs.cfgr = data;
      update_speed();
    } break;

    case 0x3038: {
      regs.scbr = data;
    } break;

    case 0x3039: {
      regs.clsr = data;
      update_speed();
    } break;

    case 0x303a: {
      regs.scmr = data;
    } break;
  }
}

#endif
