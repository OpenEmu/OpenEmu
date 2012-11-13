#ifdef SUFAMITURBO_CPP

void SufamiTurbo::serialize(serializer &s) {
  s.array(slotA.ram.data(), slotA.ram.size());
  s.array(slotB.ram.data(), slotB.ram.size());
}

#endif
