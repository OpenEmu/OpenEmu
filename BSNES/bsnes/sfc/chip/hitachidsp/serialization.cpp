#ifdef HITACHIDSP_CPP

void HitachiDSP::serialize(serializer &s) {
  HG51B::serialize(s);
  Thread::serialize(s);

  s.integer(mmio.dma);
  s.integer(mmio.dma_source);
  s.integer(mmio.dma_length);
  s.integer(mmio.dma_target);
  s.integer(mmio.r1f48);
  s.integer(mmio.program_offset);
  s.integer(mmio.r1f4c);
  s.integer(mmio.page_number);
  s.integer(mmio.program_counter);
  s.integer(mmio.r1f50);
  s.integer(mmio.r1f51);
  s.integer(mmio.r1f52);
  s.array(mmio.vector);
}

#endif
