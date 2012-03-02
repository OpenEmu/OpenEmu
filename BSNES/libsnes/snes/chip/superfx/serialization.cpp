#ifdef SUPERFX_CPP

void SuperFX::serialize(serializer &s) {
  Processor::serialize(s);

  //superfx.hpp
  s.integer(clockmode);
  s.integer(instruction_counter);

  //core/registers.hpp
  s.integer(regs.pipeline);
  s.integer(regs.ramaddr);

  s.integer(regs.r[ 0].data);
  s.integer(regs.r[ 1].data);
  s.integer(regs.r[ 2].data);
  s.integer(regs.r[ 3].data);
  s.integer(regs.r[ 4].data);
  s.integer(regs.r[ 5].data);
  s.integer(regs.r[ 6].data);
  s.integer(regs.r[ 7].data);
  s.integer(regs.r[ 8].data);
  s.integer(regs.r[ 9].data);
  s.integer(regs.r[10].data);
  s.integer(regs.r[11].data);
  s.integer(regs.r[12].data);
  s.integer(regs.r[13].data);
  s.integer(regs.r[14].data);
  s.integer(regs.r[15].data);

  s.integer(regs.sfr.irq);
  s.integer(regs.sfr.b);
  s.integer(regs.sfr.ih);
  s.integer(regs.sfr.il);
  s.integer(regs.sfr.alt2);
  s.integer(regs.sfr.alt1);
  s.integer(regs.sfr.r);
  s.integer(regs.sfr.g);
  s.integer(regs.sfr.ov);
  s.integer(regs.sfr.s);
  s.integer(regs.sfr.cy);
  s.integer(regs.sfr.z);

  s.integer(regs.pbr);
  s.integer(regs.rombr);
  s.integer(regs.rambr);
  s.integer(regs.cbr);
  s.integer(regs.scbr);

  s.integer(regs.scmr.ht);
  s.integer(regs.scmr.ron);
  s.integer(regs.scmr.ran);
  s.integer(regs.scmr.md);

  s.integer(regs.colr);

  s.integer(regs.por.obj);
  s.integer(regs.por.freezehigh);
  s.integer(regs.por.highnibble);
  s.integer(regs.por.dither);
  s.integer(regs.por.transparent);

  s.integer(regs.bramr);
  s.integer(regs.vcr);

  s.integer(regs.cfgr.irq);
  s.integer(regs.cfgr.ms0);

  s.integer(regs.clsr);

  s.integer(regs.romcl);
  s.integer(regs.romdr);

  s.integer(regs.ramcl);
  s.integer(regs.ramar);
  s.integer(regs.ramdr);

  s.integer(regs.sreg);
  s.integer(regs.dreg);

  s.array(cache.buffer);
  s.array(cache.valid);

  for(unsigned i = 0; i < 2; i++) {
    s.integer(pixelcache[i].offset);
    s.integer(pixelcache[i].bitpend);
    s.array(pixelcache[i].data);
  }

  //timing/timing.hpp
  s.integer(cache_access_speed);
  s.integer(memory_access_speed);
  s.integer(r15_modified);
}

#endif
