#ifdef CPU_CPP

void CPU::op_io() {
  cycle_edge();
  add_clocks(4);
}

uint8 CPU::op_read(uint16 addr) {
  cycle_edge();
  uint8 r = bus.read(addr);
  add_clocks(4);
  return r;
}

void CPU::op_write(uint16 addr, uint8 data) {
  cycle_edge();
  bus.write(addr, data);
  add_clocks(4);
}

void CPU::cycle_edge() {
  if(r.ei) {
    r.ei = false;
    r.ime = 1;
  }
}

uint8 CPU::debugger_read(uint16 addr) {
  return bus.read(addr);
}

#endif
