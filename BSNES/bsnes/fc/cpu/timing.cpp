uint8 CPU::op_read(uint16 addr) {
  if(status.oam_dma_pending) {
    status.oam_dma_pending = false;
    op_read(addr);
    oam_dma();
  }

  while(status.rdy_line == 0) {
    regs.mdr = bus.read(status.rdy_addr ? status.rdy_addr() : addr);
    add_clocks(12);
  }

  regs.mdr = bus.read(addr);
  add_clocks(12);
  return regs.mdr;
}

void CPU::op_write(uint16 addr, uint8 data) {
  bus.write(addr, regs.mdr = data);
  add_clocks(12);
}

void CPU::last_cycle() {
  status.interrupt_pending = ((status.irq_line | status.irq_apu_line) & ~regs.p.i) | status.nmi_pending;
}

void CPU::nmi(uint16 &vector) {
  if(status.nmi_pending) {
    status.nmi_pending = false;
    vector = 0xfffa;
  }
}

void CPU::oam_dma() {
  for(unsigned n = 0; n < 256; n++) {
    uint8 data = op_read((status.oam_dma_page << 8) + n);
    op_write(0x2004, data);
  }
}

void CPU::set_nmi_line(bool line) {
  //edge-sensitive (0->1)
  if(!status.nmi_line && line) status.nmi_pending = true;
  status.nmi_line = line;
}

void CPU::set_irq_line(bool line) {
  //level-sensitive
  status.irq_line = line;
}

void CPU::set_irq_apu_line(bool line) {
  //level-sensitive
  status.irq_apu_line = line;
}

void CPU::set_rdy_line(bool line) {
  status.rdy_line = line;
}

void CPU::set_rdy_addr(optional<uint16> addr) {
  status.rdy_addr = addr;
}
