#ifdef APU_CPP

void APU::serialize(serializer &s) {
  Processor::serialize(s);

  s.array(mmio_data);
  s.integer(sequencer_base);
  s.integer(sequencer_step);

  square1.serialize(s);
  square2.serialize(s);
  wave.serialize(s);
  noise.serialize(s);
  master.serialize(s);
}

#endif
