void SMP::tick() {
  timer0.tick();
  timer1.tick();
  timer2.tick();

  clock += cycle_step_cpu;
  dsp.clock -= 24;
  synchronize_dsp();
}

void SMP::op_io() {
  #if defined(CYCLE_ACCURATE)
  tick();
  #endif
}

uint8 SMP::op_read(uint16 addr) {
  #if defined(CYCLE_ACCURATE)
  tick();
  #endif
  if((addr & 0xfff0) == 0x00f0) return mmio_read(addr);
  if(addr >= 0xffc0 && status.iplrom_enable) return iplrom[addr & 0x3f];
  return apuram[addr];
}

void SMP::op_write(uint16 addr, uint8 data) {
  #if defined(CYCLE_ACCURATE)
  tick();
  #endif
  if((addr & 0xfff0) == 0x00f0) mmio_write(addr, data);
  apuram[addr] = data;  //all writes go to RAM, even MMIO writes
}

void SMP::op_step() {
  #define op_readpc() op_read(regs.pc++)
  #define op_readdp(addr) op_read((regs.p.p << 8) + addr)
  #define op_writedp(addr, data) op_write((regs.p.p << 8) + addr, data)
  #define op_readaddr(addr) op_read(addr)
  #define op_writeaddr(addr, data) op_write(addr, data)
  #define op_readstack() op_read(0x0100 | ++regs.sp)
  #define op_writestack(data) op_write(0x0100 | regs.sp--, data)

  #if defined(CYCLE_ACCURATE)

  if(opcode_cycle == 0) {
    opcode_number = op_readpc();
    opcode_cycle++;
  } else switch(opcode_number) {
    #include "core/opcycle_misc.cpp"
    #include "core/opcycle_mov.cpp"
    #include "core/opcycle_pc.cpp"
    #include "core/opcycle_read.cpp"
    #include "core/opcycle_rmw.cpp"
  }

  #else

  unsigned opcode = op_readpc();
  switch(opcode) {
    #include "core/op_misc.cpp"
    #include "core/op_mov.cpp"
    #include "core/op_pc.cpp"
    #include "core/op_read.cpp"
    #include "core/op_rmw.cpp"
  }

  //TODO: untaken branches should consume less cycles

  timer0.tick(cycle_count_table[opcode]);
  timer1.tick(cycle_count_table[opcode]);
  timer2.tick(cycle_count_table[opcode]);

  clock += cycle_table_cpu[opcode];
  dsp.clock -= cycle_table_dsp[opcode];
  synchronize_dsp();

  #endif
}

const unsigned SMP::cycle_count_table[256] = {
  #define c 12
//0 1 2 3   4 5 6 7   8 9 A B   C D E F
  2,8,4,7,  3,4,3,6,  2,6,5,4,  5,4,6,8,  //0
  4,8,4,7,  4,5,5,6,  5,5,6,5,  2,2,4,6,  //1
  2,8,4,7,  3,4,3,6,  2,6,5,4,  5,4,7,4,  //2
  4,8,4,7,  4,5,5,6,  5,5,6,5,  2,2,3,8,  //3

  2,8,4,7,  3,4,3,6,  2,6,4,4,  5,4,6,6,  //4
  4,8,4,7,  4,5,5,6,  5,5,4,5,  2,2,4,3,  //5
  2,8,4,7,  3,4,3,6,  2,6,4,4,  5,4,7,5,  //6
  4,8,4,7,  4,5,5,6,  5,5,5,5,  2,2,3,6,  //7

  2,8,4,7,  3,4,3,6,  2,6,5,4,  5,2,4,5,  //8
  4,8,4,7,  4,5,5,6,  5,5,5,5,  2,2,c,5,  //9
  3,8,4,7,  3,4,3,6,  2,6,4,4,  5,2,4,4,  //A
  4,8,4,7,  4,5,5,6,  5,5,5,5,  2,2,3,4,  //B

  3,8,4,7,  4,5,4,7,  2,5,6,4,  5,2,4,9,  //C
  4,8,4,7,  5,6,6,7,  4,5,5,5,  2,2,8,3,  //D
  2,8,4,7,  3,4,3,6,  2,4,5,3,  4,3,4,1,  //E
  4,8,4,7,  4,5,5,6,  3,4,5,4,  2,2,6,1,  //F

  #undef c
};
