#ifdef S21FX_CPP

void S21fx::serialize(serializer &s) {
  s.integer(mmio.status);
  s.integer(mmio.shift_register);

  s.integer(mmio.data_offset);
  s.integer(mmio.audio_offset);
  s.integer(mmio.audio_track);
  s.integer(mmio.audio_volume_left);
  s.integer(mmio.audio_volume_right);
  s.integer(mmio.audio_repeat);
  s.integer(mmio.audio_pause);

  //flush file handles and indices, as a different track may be playing,
  //or the file offsets may be at the wrong location ...

  if(datafile.open()) datafile.close();
  if(datafile.open(string() << basepath << "21fx.bin", file::mode_read)) {
    datafile.seek(mmio.data_offset);
  }

  if(audiofile.open()) audiofile.close();
  char track[16];
  sprintf(track, "%.5u", mmio.audio_track);
  if(audiofile.open(string() << basepath << "audio" << track << ".wav", file::mode_read)) {
    audiofile.seek(mmio.audio_offset);
  }
}

#endif
