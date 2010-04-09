#ifdef BPPU_CPP

uint8 bPPUDebug::vram_mmio_read(uint16 addr) {
  uint8 data = bPPU::vram_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::VRAM, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void bPPUDebug::vram_mmio_write(uint16 addr, uint8 data) {
  bPPU::vram_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::VRAM, Debugger::Breakpoint::Write, addr, data);
}

uint8 bPPUDebug::oam_mmio_read(uint16 addr) {
  uint8 data = bPPU::oam_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::OAM, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void bPPUDebug::oam_mmio_write(uint16 addr, uint8 data) {
  bPPU::oam_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::OAM, Debugger::Breakpoint::Write, addr, data);
}

uint8 bPPUDebug::cgram_mmio_read(uint16 addr) {
  uint8 data = bPPU::cgram_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::CGRAM, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void bPPUDebug::cgram_mmio_write(uint16 addr, uint8 data) {
  bPPU::cgram_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::CGRAM, Debugger::Breakpoint::Write, addr, data);
}

#endif
