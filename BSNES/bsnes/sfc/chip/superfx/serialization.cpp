#ifdef SUPERFX_CPP

void SuperFX::serialize(serializer &s) {
  GSU::serialize(s);
  Thread::serialize(s);

  s.integer(clockmode);
  s.integer(instruction_counter);

  s.integer(cache_access_speed);
  s.integer(memory_access_speed);
  s.integer(r15_modified);
}

#endif
