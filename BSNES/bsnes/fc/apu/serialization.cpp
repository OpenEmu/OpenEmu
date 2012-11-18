void APU::serialize(serializer &s) {
  Thread::serialize(s);

  filter.serialize(s);

  pulse[0].serialize(s);
  pulse[1].serialize(s);
  triangle.serialize(s);
  dmc.serialize(s);
  frame.serialize(s);

  s.integer(enabled_channels);
  s.integer(cartridge_sample);
}

void APU::Filter::serialize(serializer &s) {
  s.integer(hipass_strong);
  s.integer(hipass_weak);
  s.integer(lopass);
}

void APU::FrameCounter::serialize(serializer &s) {
  s.integer(irq_pending);

  s.integer(mode);
  s.integer(counter);
  s.integer(divider);
}
