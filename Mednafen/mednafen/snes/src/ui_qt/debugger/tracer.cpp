#include "tracer.moc"
Tracer *tracer;

void Tracer::stepCpu() {
  if(traceCpu) {
    unsigned addr = SNES::cpu.regs.pc;
    if(!traceMask || !(traceMaskCPU[addr >> 3] & (0x80 >> (addr & 7)))) {
      char text[256];
      SNES::cpu.disassemble_opcode(text, addr);
      tracefile.print(string() << text << "\n");
    }
    traceMaskCPU[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSmp() {
  if(traceSmp) {
    unsigned addr = SNES::smp.regs.pc;
    if(!traceMask || !(traceMaskSMP[addr >> 3] & (0x80 >> (addr & 7)))) {
      char text[256];
      SNES::smp.disassemble_opcode(text, addr);
      tracefile.print(string() << text << "\n");
    }
    traceMaskSMP[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::setCpuTraceState(int state) {
  traceCpu = (state == Qt::Checked);

  if(traceCpu && !tracefile.open()) {
    tracefile.open(string() << config().path.data << "trace.log", file::mode_write);
  } else if(!traceCpu && !traceSmp && tracefile.open()) {
    tracefile.close();
  }
}

void Tracer::setSmpTraceState(int state) {
  traceSmp = (state == Qt::Checked);

  if(traceSmp && !tracefile.open()) {
    tracefile.open(string() << config().path.data << "trace.log", file::mode_write);
  } else if(!traceCpu && !traceSmp && tracefile.open()) {
    tracefile.close();
  }
}

void Tracer::setTraceMaskState(int state) {
  traceMask = (state == Qt::Checked);

  if(traceMask) {
    //flush all bitmasks once enabled
    memset(traceMaskCPU, 0x00, (1 << 24) >> 3);
    memset(traceMaskSMP, 0x00, (1 << 16) >> 3);
  }
}

Tracer::Tracer() {
  traceCpu = false;
  traceSmp = false;
  traceMask = false;

  traceMaskCPU = new uint8[(1 << 24) >> 3]();
  traceMaskSMP = new uint8[(1 << 16) >> 3]();

  SNES::cpu.step_event = bind(&Tracer::stepCpu, this);
  SNES::smp.step_event = bind(&Tracer::stepSmp, this);
}

Tracer::~Tracer() {
  delete[] traceMaskCPU;
  delete[] traceMaskSMP;
  if(tracefile.open()) tracefile.close();
}
