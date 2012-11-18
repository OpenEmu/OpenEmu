#ifdef PROCESSOR_ARM_HPP

void ARM::Processor::power() {
  r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = 0;
  usr.r8 = usr.r9 = usr.r10 = usr.r11 = usr.r12 = usr.sp = usr.lr = 0;
  fiq.r8 = fiq.r9 = fiq.r10 = fiq.r11 = fiq.r12 = fiq.sp = fiq.lr = 0;
  irq.sp = irq.lr = 0;
  svc.sp = svc.lr = 0;
  abt.sp = abt.lr = 0;
  und.sp = und.lr = 0;
  pc = 0;

  carryout = false;
  sequential = false;
  irqline = false;

  cpsr = 0;
  spsr = nullptr;
  fiq.spsr = 0;
  irq.spsr = 0;
  svc.spsr = 0;
  abt.spsr = 0;
  und.spsr = 0;

  r[0] = &r0;
  r[1] = &r1;
  r[2] = &r2;
  r[3] = &r3;
  r[4] = &r4;
  r[5] = &r5;
  r[6] = &r6;
  r[7] = &r7;

  r[15] = &pc;
}

void ARM::Processor::setMode(Mode mode) {
  cpsr.m = 0x10 | (unsigned)mode;

  if(mode == Mode::FIQ) {
    r[ 8] = &fiq.r8;
    r[ 9] = &fiq.r9;
    r[10] = &fiq.r10;
    r[11] = &fiq.r11;
    r[12] = &fiq.r12;
  } else {
    r[ 8] = &usr.r8;
    r[ 9] = &usr.r9;
    r[10] = &usr.r10;
    r[11] = &usr.r11;
    r[12] = &usr.r12;
  }

  switch(mode) {
  case Mode::FIQ: r[13] = &fiq.sp; r[14] = &fiq.lr; spsr = &fiq.spsr; break;
  case Mode::IRQ: r[13] = &irq.sp; r[14] = &irq.lr; spsr = &irq.spsr; break;
  case Mode::SVC: r[13] = &svc.sp; r[14] = &svc.lr; spsr = &svc.spsr; break;
  case Mode::ABT: r[13] = &abt.sp; r[14] = &abt.lr; spsr = &abt.spsr; break;
  case Mode::UND: r[13] = &und.sp; r[14] = &und.lr; spsr = &und.spsr; break;
  default:        r[13] = &usr.sp; r[14] = &usr.lr; spsr = nullptr;   break;
  }
}

void ARM::pipeline_step() {
  pipeline.execute = pipeline.decode;
  pipeline.decode = pipeline.fetch;

  if(cpsr().t == 0) {
    r(15).data += 4;
    pipeline.fetch.address = r(15) & ~3;
    pipeline.fetch.instruction = read(pipeline.fetch.address, Word);
  } else {
    r(15).data += 2;
    pipeline.fetch.address = r(15) & ~1;
    pipeline.fetch.instruction = read(pipeline.fetch.address, Half);
  }
}

#endif
