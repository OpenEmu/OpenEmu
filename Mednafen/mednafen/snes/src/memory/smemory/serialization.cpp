#ifdef SMEMORY_CPP

void sBus::serialize(serializer &s) {
  s.array(memory::wram.data(), memory::wram.size());
  s.array(memory::apuram.data(), memory::apuram.size());
  s.array(memory::vram.data(), memory::vram.size());
  s.array(memory::oam.data(), memory::oam.size());
  s.array(memory::cgram.data(), memory::cgram.size());
}

#endif
