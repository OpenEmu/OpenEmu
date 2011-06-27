#ifdef SRTC_CPP

void SRTC::serialize(serializer &s) {
  s.integer(rtc_mode);
  s.integer(rtc_index);
}

#endif
