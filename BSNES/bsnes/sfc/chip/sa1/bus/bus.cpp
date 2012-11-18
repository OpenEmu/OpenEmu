#ifdef SA1_CPP

//ROM / RAM access from the S-CPU

unsigned SA1::CPUIRAM::size() const {
  return sa1.iram.size();
}

uint8 SA1::CPUIRAM::read(unsigned addr) {
  cpu.synchronize_coprocessors();
  return sa1.iram.read(addr);
}

void SA1::CPUIRAM::write(unsigned addr, uint8 data) {
  cpu.synchronize_coprocessors();
  sa1.iram.write(addr, data);
}

unsigned SA1::CPUBWRAM::size() const {
  return cartridge.ram.size();
}

uint8 SA1::CPUBWRAM::read(unsigned addr) {
  cpu.synchronize_coprocessors();
  if(dma) return sa1.dma_cc1_read(addr);
  return cartridge.ram.read(addr);
}

void SA1::CPUBWRAM::write(unsigned addr, uint8 data) {
  cpu.synchronize_coprocessors();
  cartridge.ram.write(addr, data);
}

#endif
