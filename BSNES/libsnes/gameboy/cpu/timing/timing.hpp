void add_clocks(unsigned clocks);
void timer_262144hz();
void timer_65536hz();
void timer_16384hz();
void timer_8192hz();
void timer_4096hz();
void hblank();

//opcode.cpp
void op_io();
uint8 op_read(uint16 addr);
void op_write(uint16 addr, uint8 data);
void cycle_edge();
