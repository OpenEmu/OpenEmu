#ifdef MSU1_CPP

void MSU1::serialize(serializer &s) {
  Thread::serialize(s);

  s.integer(mmio.data_offset);
  s.integer(mmio.audio_offset);
  s.integer(mmio.audio_loop_offset);

  s.integer(mmio.audio_track);
  s.integer(mmio.audio_volume);

  s.integer(mmio.data_busy);
  s.integer(mmio.audio_busy);
  s.integer(mmio.audio_repeat);
  s.integer(mmio.audio_play);

  if(datafile.open()) datafile.close();
  if(datafile.open({interface->path(0), "msu1.rom"}, file::mode::read)) {
    datafile.seek(mmio.data_offset);
  }

  if(audiofile.open()) audiofile.close();
  if(audiofile.open({interface->path(0), "track-", mmio.audio_track, ".pcm"}, file::mode::read)) {
    audiofile.seek(mmio.audio_offset);
  }
}

#endif
