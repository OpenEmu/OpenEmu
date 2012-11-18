void ARM::PSR::serialize(serializer &s) {
  s.integer(n);
  s.integer(z);
  s.integer(c);
  s.integer(v);
  s.integer(i);
  s.integer(f);
  s.integer(t);
  s.integer(m);
}

void ARM::serialize(serializer &s) {
  s.integer(processor.r0.data);
  s.integer(processor.r1.data);
  s.integer(processor.r2.data);
  s.integer(processor.r3.data);
  s.integer(processor.r4.data);
  s.integer(processor.r5.data);
  s.integer(processor.r6.data);
  s.integer(processor.r7.data);

  s.integer(processor.usr.r8.data);
  s.integer(processor.usr.r9.data);
  s.integer(processor.usr.r10.data);
  s.integer(processor.usr.r11.data);
  s.integer(processor.usr.r12.data);
  s.integer(processor.usr.sp.data);
  s.integer(processor.usr.lr.data);

  s.integer(processor.fiq.r8.data);
  s.integer(processor.fiq.r9.data);
  s.integer(processor.fiq.r10.data);
  s.integer(processor.fiq.r11.data);
  s.integer(processor.fiq.r12.data);
  s.integer(processor.fiq.sp.data);
  s.integer(processor.fiq.lr.data);
  processor.fiq.spsr.serialize(s);

  s.integer(processor.irq.sp.data);
  s.integer(processor.irq.lr.data);
  processor.irq.spsr.serialize(s);

  s.integer(processor.svc.sp.data);
  s.integer(processor.svc.lr.data);
  processor.svc.spsr.serialize(s);

  s.integer(processor.abt.sp.data);
  s.integer(processor.abt.lr.data);
  processor.abt.spsr.serialize(s);

  s.integer(processor.und.sp.data);
  s.integer(processor.und.lr.data);
  processor.und.spsr.serialize(s);

  s.integer(processor.pc.data);
  processor.cpsr.serialize(s);
  s.integer(processor.carryout);
  s.integer(processor.sequential);
  s.integer(processor.irqline);

  s.integer(pipeline.reload);
  s.integer(pipeline.execute.address);
  s.integer(pipeline.execute.instruction);
  s.integer(pipeline.decode.address);
  s.integer(pipeline.decode.instruction);
  s.integer(pipeline.fetch.address);
  s.integer(pipeline.fetch.instruction);

  s.integer(crash);

  processor.setMode((Processor::Mode)cpsr().m);
}
