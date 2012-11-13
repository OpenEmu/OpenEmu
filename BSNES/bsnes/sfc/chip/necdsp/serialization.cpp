#ifdef NECDSP_CPP

void NECDSP::serialize(serializer &s) {
  uPD96050::serialize(s);
  Thread::serialize(s);
}

#endif
