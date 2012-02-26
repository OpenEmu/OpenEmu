#ifdef SDD1_CPP

void SDD1::serialize(serializer &s) {
  s.integer(sdd1_enable);
  s.integer(xfer_enable);
  s.integer(dma_ready);
  s.array(mmc);

  for(unsigned n = 0; n < 8; n++) {
    s.integer(dma[n].addr);
    s.integer(dma[n].size);
  }
}

#endif
