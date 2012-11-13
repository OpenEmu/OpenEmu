#ifdef CARTRIDGE_CPP

void Cartridge::serialize(serializer &s) {
  s.array(ram.data(), ram.size());
}

#endif
