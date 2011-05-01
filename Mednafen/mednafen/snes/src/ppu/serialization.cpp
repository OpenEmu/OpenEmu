#ifdef PPU_CPP

void PPUcounter::serialize(serializer &s) {
  s.integer(status.interlace);
  s.integer(status.field);
  s.integer(status.vcounter);
  s.integer(status.hcounter);

  s.array(history.field);
  s.array(history.vcounter);
  s.array(history.hcounter);
  s.integer(history.index);
}

void PPU::serialize(serializer &s) {
  PPUcounter::serialize(s);

  s.integer(status.render_output);
  s.integer(status.frame_executed);
  s.integer(status.frames_updated);
  s.integer(status.frames_rendered);
  s.integer(status.frames_executed);

  s.integer(ppu1_version);
  s.integer(ppu2_version);
}

#endif
