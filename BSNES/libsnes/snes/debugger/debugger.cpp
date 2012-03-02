#ifdef SYSTEM_CPP

Debugger debugger;

void Debugger::breakpoint_test(Debugger::Breakpoint::Source::e source, Debugger::Breakpoint::Mode::e mode, unsigned addr, uint8 data) {
  for(unsigned i = 0; i < Breakpoints; i++) {
    if(breakpoint[i].enabled == false) continue;

    bool source_wram = ((breakpoint[i].addr & 0x40e000) == 0x000000) || ((breakpoint[i].addr & 0xffe000) == 0x7e0000);
    bool offset_wram = ((addr & 0x40e000) == 0x000000) || ((addr & 0xffe000) == 0x7e0000);

    if(source == Debugger::Breakpoint::Source::CPUBus && source_wram && offset_wram) {
      //shadow S-CPU WRAM addresses ($00-3f|80-bf:0000-1fff mirrors $7e:0000-1fff)
      if((breakpoint[i].addr & 0x1fff) != (addr & 0x1fff)) continue;
    } else {
      if(breakpoint[i].addr != addr) continue;
    }

    if(breakpoint[i].data != -1 && breakpoint[i].data != data) continue;
    if(breakpoint[i].source.i != source) continue;
    if(breakpoint[i].mode.i != mode) continue;

    breakpoint[i].counter++;
    breakpoint_hit = i;
    break_event.i = BreakEvent::BreakpointHit;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
    break;
  }
}

uint8 Debugger::read(Debugger::MemorySource::e source, unsigned addr) {
  switch(source) {
    case MemorySource::CPUBus: {
      //do not read from memory-mapped registers that could affect program behavior
      if(((addr - 0x2000) & 0x40c000) == 0x000000) break;  //$00-3f:2000-5fff MMIO
      return bus.read(addr & 0xffffff);
    } break;

    case MemorySource::APUBus: {
      if((addr & 0xffc0) == 0xffc0) return smp.iplrom[addr & 0x3f];
      return smp.apuram[addr & 0xffff];
    } break;

    case MemorySource::APURAM: {
      return smp.apuram[addr & 0xffff];
    } break;

    case MemorySource::VRAM: {
      return ppu.vram[addr & 0xffff];
    } break;

    case MemorySource::OAM: {
      if(addr & 0x0200) return ppu.oam[0x0200 + (addr & 0x1f)];
      return ppu.oam[addr & 0x01ff];
    } break;

    case MemorySource::CGRAM: {
      return ppu.cgram[addr & 0x01ff];
    } break;
  }

  return 0x00;
}

void Debugger::write(Debugger::MemorySource::e source, unsigned addr, uint8 data) {
  switch(source) {
    case MemorySource::CPUBus: {
      //do not write to memory-mapped registers that could affect program behavior
      if(((addr - 0x2000) & 0x40c000) == 0x000000) break;  //$00-3f:2000-5fff MMIO
      cartridge.rom.write_protect(false);
      bus.write(addr & 0xffffff, data);
      cartridge.rom.write_protect(true);
    } break;

    case MemorySource::APUBus: {
      break;
    }

    case MemorySource::APURAM: {
      smp.apuram[addr & 0xffff] = data;
    } break;

    case MemorySource::VRAM: {
      ppu.vram[addr & 0xffff] = data;
    } break;

    case MemorySource::OAM: {
      if(addr & 0x0200) ppu.oam[0x0200 + (addr & 0x1f)] = data;
      else ppu.oam[addr & 0x01ff] = data;
    } break;

    case MemorySource::CGRAM: {
      ppu.cgram[addr & 0x01ff] = data;
    } break;
  }
}

Debugger::Debugger() {
  break_event.i = BreakEvent::None;

  for(unsigned n = 0; n < Breakpoints; n++) {
    breakpoint[n].enabled = false;
    breakpoint[n].addr = 0;
    breakpoint[n].data = -1;
    breakpoint[n].mode.i = Breakpoint::Mode::Exec;
    breakpoint[n].source.i = Breakpoint::Source::CPUBus;
    breakpoint[n].counter = 0;
  }
  breakpoint_hit = 0;
}

#endif
