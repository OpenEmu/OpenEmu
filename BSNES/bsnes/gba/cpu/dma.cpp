void CPU::dma_run() {
  for(unsigned n = 0; n < 4; n++) {
    auto &dma = regs.dma[n];
    if(dma.pending) {
      dma.pending = false;
      dma_transfer(dma);
      if(dma.control.irq) regs.irq.flag.dma[n] = 1;
      if(dma.control.drq && n == 3) regs.irq.flag.cartridge = 1;
    }
  }
}

void CPU::dma_transfer(Registers::DMA &dma) {
  unsigned size = dma.control.size ? Word : Half;
  unsigned seek = dma.control.size ? 4 : 2;

  sequential() = false;
  do {
    step(bus.speed(dma.run.source, size));
    uint32 word = bus.read(dma.run.source, size);

    step(bus.speed(dma.run.target, size));
    bus.write(dma.run.target, size, word);

    sequential() = true;

    switch(dma.control.sourcemode) {
    case 0: dma.run.source += seek; break;
    case 1: dma.run.source -= seek; break;
    }

    switch(dma.control.targetmode) {
    case 0: dma.run.target += seek; break;
    case 1: dma.run.target -= seek; break;
    case 3: dma.run.target += seek; break;
    }
  } while(--dma.run.length);
  sequential() = false;

  if(dma.control.targetmode == 3) dma.run.target = dma.target;
  if(dma.control.repeat == 1) dma.run.length = dma.length;
  if(dma.control.repeat == 0) dma.control.enable = false;
}

void CPU::dma_vblank() {
  for(auto &dma : regs.dma) {
    if(dma.control.enable && dma.control.timingmode == 1) dma.pending = true;
  }
}

void CPU::dma_hblank() {
  for(auto &dma : regs.dma) {
    if(dma.control.enable && dma.control.timingmode == 2) dma.pending = true;
  }
}

void CPU::dma_hdma() {
  auto &dma = regs.dma[3];
  if(dma.control.enable && dma.control.timingmode == 3) dma.pending = true;
}
