#ifdef SDD1_CPP

void SDD1::serialize(serializer &s) {
  s.integer(sdd1_enable);
  s.integer(xfer_enable);
  s.array(mmc);

  for(unsigned n = 0; n < 8; n++) {
    s.integer(dma[n].addr);
    s.integer(dma[n].size);
  }

  s.array(buffer.data);
  s.integer(buffer.offset);
  s.integer(buffer.size);
  s.integer(buffer.ready);
}

#endif
