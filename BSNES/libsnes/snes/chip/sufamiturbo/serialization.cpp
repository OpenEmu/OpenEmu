#ifdef SUFAMITURBO_CPP

void SufamiTurbo::serialize(serializer &s) {
  if(slotA.ram.data()) s.array(slotA.ram.data(), slotA.ram.size());
  if(slotB.ram.data()) s.array(slotB.ram.data(), slotB.ram.size());
}

#endif
