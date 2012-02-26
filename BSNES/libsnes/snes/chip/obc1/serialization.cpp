#ifdef OBC1_CPP

void OBC1::serialize(serializer &s) {
  s.integer(status.address);
  s.integer(status.baseptr);
  s.integer(status.shift);
}

#endif
