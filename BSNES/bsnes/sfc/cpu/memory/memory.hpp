void op_io();
uint8 op_read(uint32 addr);
void op_write(uint32 addr, uint8 data);
alwaysinline unsigned speed(unsigned addr) const;

uint8 disassembler_read(uint32 addr);
