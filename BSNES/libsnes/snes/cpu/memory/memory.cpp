#ifdef CPU_CPP

uint8 CPU::port_read(uint2 port) const { return status.port[port]; }
void CPU::port_write(uint2 port, uint8 data) { status.port[port] = data; }

void CPU::op_io() {
  status.clock_count = 6;
  dma_edge();
  add_clocks(6);
  alu_edge();
}

uint8 CPU::op_read(uint32 addr) {
  status.clock_count = speed(addr);
  dma_edge();
  add_clocks(status.clock_count - 4);
  regs.mdr = bus.read(addr);
  add_clocks(4);
  alu_edge();
  return regs.mdr;
}

void CPU::op_write(uint32 addr, uint8 data) {
  alu_edge();
  status.clock_count = speed(addr);
  dma_edge();
  add_clocks(status.clock_count);
  bus.write(addr, regs.mdr = data);
}

unsigned CPU::speed(unsigned addr) const {
  if(addr & 0x408000) {
    if(addr & 0x800000) return status.rom_speed;
    return 8;
  }
  if((addr + 0x6000) & 0x4000) return 8;
  if((addr - 0x4000) & 0x7e00) return 6;
  return 12;
}

#endif
