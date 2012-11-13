#ifdef CPU_CPP

uint8 CPU::mmio_read(unsigned addr) {
  if((addr & 0xffc0) == 0x2140) {
    synchronize_smp();
    return smp.port_read(addr & 3);
  }

  switch(addr & 0xffff) {
    case 0x2180: {
      uint8 result = bus.read(0x7e0000 | status.wram_addr);
      status.wram_addr = (status.wram_addr + 1) & 0x01ffff;
      return result;
    }

    case 0x4016: {
      uint8 result = regs.mdr & 0xfc;
      result |= input.port1->data() & 3;
      return result;
    }

    case 0x4017: {
      uint8 result = (regs.mdr & 0xe0) | 0x1c;
      result |= input.port2->data() & 3;
      return result;
    }

    case 0x4210: {
      uint8 result = (regs.mdr & 0x70);
      result |= status.nmi_line << 7;
      result |= 0x02;  //CPU revision
      status.nmi_line = false;
      return result;
    }

    case 0x4211: {
      uint8 result = (regs.mdr & 0x7f);
      result |= status.irq_line << 7;
      status.irq_line = false;
      return result;
    }

    case 0x4212: {
      uint8 result = (regs.mdr & 0x3e);
      unsigned vbstart = ppu.overscan() == false ? 225 : 240;

      if(vcounter() >= vbstart && vcounter() <= vbstart + 2) result |= 0x01;
      if(hcounter() <= 2 || hcounter() >= 1096) result |= 0x40;
      if(vcounter() >= vbstart) result |= 0x80;

      return result;
    }

    case 0x4213: return status.pio;

    case 0x4214: return status.rddiv >> 0;
    case 0x4215: return status.rddiv >> 8;
    case 0x4216: return status.rdmpy >> 0;
    case 0x4217: return status.rdmpy >> 8;

    case 0x4218: return status.joy1l;
    case 0x4219: return status.joy1h;
    case 0x421a: return status.joy2l;
    case 0x421b: return status.joy2h;
    case 0x421c: return status.joy3l;
    case 0x421d: return status.joy3h;
    case 0x421e: return status.joy4l;
    case 0x421f: return status.joy4h;
  }

  if((addr & 0xff80) == 0x4300) {
    unsigned i = (addr >> 4) & 7;
    switch(addr & 0xff8f) {
      case 0x4300: {
        return (channel[i].direction << 7)
             | (channel[i].indirect << 6)
             | (channel[i].unused << 5)
             | (channel[i].reverse_transfer << 4)
             | (channel[i].fixed_transfer << 3)
             | (channel[i].transfer_mode << 0);
      }

      case 0x4301: return channel[i].dest_addr;
      case 0x4302: return channel[i].source_addr >> 0;
      case 0x4303: return channel[i].source_addr >> 8;
      case 0x4304: return channel[i].source_bank;
      case 0x4305: return channel[i].transfer_size >> 0;
      case 0x4306: return channel[i].transfer_size >> 8;
      case 0x4307: return channel[i].indirect_bank;
      case 0x4308: return channel[i].hdma_addr >> 0;
      case 0x4309: return channel[i].hdma_addr >> 8;
      case 0x430a: return channel[i].line_counter;
      case 0x430b: case 0x430f: return channel[i].unknown;
    }
  }

  return regs.mdr;
}

void CPU::mmio_write(unsigned addr, uint8 data) {
  if((addr & 0xffc0) == 0x2140) {
    synchronize_smp();
    port_write(addr & 3, data);
    return;
  }

  switch(addr & 0xffff) {
    case 0x2180: {
      bus.write(0x7e0000 | status.wram_addr, data);
      status.wram_addr = (status.wram_addr + 1) & 0x01ffff;
      return;
    }

    case 0x2181: {
      status.wram_addr = (status.wram_addr & 0x01ff00) | (data << 0);
      return;
    }

    case 0x2182: {
      status.wram_addr = (status.wram_addr & 0x0100ff) | (data << 8);
      return;
    }

    case 0x2183: {
      status.wram_addr = (status.wram_addr & 0x00ffff) | ((data & 1) << 16);
      return;
    }

    case 0x4016: {
      input.port1->latch(data & 1);
      input.port2->latch(data & 1);
      return;
    }

    case 0x4200: {
      bool nmi_enabled = status.nmi_enabled;
      bool virq_enabled = status.virq_enabled;
      bool hirq_enabled = status.hirq_enabled;

      status.nmi_enabled = data & 0x80;
      status.virq_enabled = data & 0x20;
      status.hirq_enabled = data & 0x10;
      status.auto_joypad_poll_enabled = data & 0x01;

      if(!nmi_enabled && status.nmi_enabled && status.nmi_line) {
        status.nmi_transition = true;
      }

      if(status.virq_enabled && !status.hirq_enabled && status.irq_line) {
        status.irq_transition = true;
      }

      if(!status.virq_enabled && !status.hirq_enabled) {
        status.irq_line = false;
        status.irq_transition = false;
      }

      status.irq_lock = true;
      return;
    }

    case 0x4201: {
      if((status.pio & 0x80) && !(data & 0x80)) ppu.latch_counters();
      status.pio = data;
    }

    case 0x4202: {
      status.wrmpya = data;
      return;
    }

    case 0x4203: {
      status.wrmpyb = data;
      status.rdmpy = status.wrmpya * status.wrmpyb;
      return;
    }

    case 0x4204: {
      status.wrdiva = (status.wrdiva & 0xff00) | (data << 0);
      return;
    }

    case 0x4205: {
      status.wrdiva = (data << 8) | (status.wrdiva & 0x00ff);
      return;
    }

    case 0x4206: {
      status.wrdivb = data;
      status.rddiv = status.wrdivb ? status.wrdiva / status.wrdivb : 0xffff;
      status.rdmpy = status.wrdivb ? status.wrdiva % status.wrdivb : status.wrdiva;
      return;
    }

    case 0x4207: {
      status.htime = (status.htime & 0x0100) | (data << 0);
      return;
    }

    case 0x4208: {
      status.htime = ((data & 1) << 8) | (status.htime & 0x00ff);
      return;
    }

    case 0x4209: {
      status.vtime = (status.vtime & 0x0100) | (data << 0);
      return;
    }

    case 0x420a: {
      status.vtime = ((data & 1) << 8) | (status.vtime & 0x00ff);
      return;
    }

    case 0x420b: {
      for(unsigned i = 0; i < 8; i++) channel[i].dma_enabled = data & (1 << i);
      if(data) dma_run();
      return;
    }

    case 0x420c: {
      for(unsigned i = 0; i < 8; i++) channel[i].hdma_enabled = data & (1 << i);
      return;
    }

    case 0x420d: {
      status.rom_speed = data & 1 ? 6 : 8;
      return;
    }
  }

  if((addr & 0xff80) == 0x4300) {
    unsigned i = (addr >> 4) & 7;
    switch(addr & 0xff8f) {
      case 0x4300: {
        channel[i].direction = data & 0x80;
        channel[i].indirect = data & 0x40;
        channel[i].unused = data & 0x20;
        channel[i].reverse_transfer = data & 0x10;
        channel[i].fixed_transfer = data & 0x08;
        channel[i].transfer_mode = data & 0x07;
        return;
      }

      case 0x4301: {
        channel[i].dest_addr = data;
        return;
      }

      case 0x4302: {
        channel[i].source_addr = (channel[i].source_addr & 0xff00) | (data << 0);
        return;
      }

      case 0x4303: {
        channel[i].source_addr = (data << 8) | (channel[i].source_addr & 0x00ff);
        return;
      }

      case 0x4304: {
        channel[i].source_bank = data;
        return;
      }

      case 0x4305: {
        channel[i].transfer_size = (channel[i].transfer_size & 0xff00) | (data << 0);
        return;
      }

      case 0x4306: {
        channel[i].transfer_size = (data << 8) | (channel[i].transfer_size & 0x00ff);
        return;
      }

      case 0x4307: {
        channel[i].indirect_bank = data;
        return;
      }

      case 0x4308: {
        channel[i].hdma_addr = (channel[i].hdma_addr & 0xff00) | (data << 0);
        return;
      }

      case 0x4309: {
        channel[i].hdma_addr = (data << 8) | (channel[i].hdma_addr & 0x00ff);
        return;
      }

      case 0x430a: {
        channel[i].line_counter = data;
        return;
      }

      case 0x430b: case 0x430f: {
        channel[i].unknown = data;
        return;
      }
    }
  }
}

#endif
