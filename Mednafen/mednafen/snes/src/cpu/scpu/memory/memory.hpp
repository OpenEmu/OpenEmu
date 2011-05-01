//============================
//CPU<>APU communication ports
//============================

uint8 apu_port[4];
uint8 port_read(uint8 port) { return apu_port[port & 3]; }
void port_write(uint8 port, uint8 data) { apu_port[port & 3] = data; }

//======================
//core CPU bus functions
//======================

void op_io();
debugvirtual uint8 op_read(uint32 addr);
debugvirtual void op_write(uint32 addr, uint8 data);
alwaysinline unsigned speed(unsigned addr) const;
