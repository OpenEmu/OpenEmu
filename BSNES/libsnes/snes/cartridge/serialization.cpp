#ifdef CARTRIDGE_CPP

void Cartridge::serialize(serializer &s) {
  foreach(ram, nvram) {
    if(ram.size) s.array(ram.data, ram.size);
  }
}

#endif
