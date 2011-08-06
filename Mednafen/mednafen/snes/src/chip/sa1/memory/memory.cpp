#ifdef SA1_CPP

//ROM, I-RAM and MMIO registers are accessed at ~10.74MHz (2 clock ticks)
//BW-RAM is accessed at ~5.37MHz (4 clock ticks)
//tick() == 2 clock ticks
//note: bus conflict delays are not emulated at this time

void SA1::op_io() {
  tick();
}

uint8 SA1::op_read(unsigned addr) {
  tick();
  if(((addr & 0x40e000) == 0x006000) || ((addr & 0xd00000) == 0x400000)) tick();
  return sa1bus.read(addr);
}

void SA1::op_write(unsigned addr, uint8 data) {
  tick();
  if(((addr & 0x40e000) == 0x006000) || ((addr & 0xd00000) == 0x400000)) tick();
  sa1bus.write(addr, data);
}

#endif
