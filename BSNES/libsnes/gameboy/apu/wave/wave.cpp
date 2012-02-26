#ifdef APU_CPP

void APU::Wave::run() {
  if(period && --period == 0) {
    period = 2 * (2048 - frequency);
    pattern_sample = pattern[++pattern_offset];
  }

  uint4 sample = pattern_sample >> volume_shift;
  if(enable == false) sample = 0;

  output = sample;
}

void APU::Wave::clock_length() {
  if(counter && length) {
    if(--length == 0) enable = false;
  }
}

void APU::Wave::write(unsigned r, uint8 data) {
  if(r == 0) {  //$ff1a  NR30
    dac_enable = data & 0x80;
    if(dac_enable == false) enable = false;
  }

  if(r == 1) {  //$ff1b  NR31
    length = 256 - data;
  }

  if(r == 2) {  //$ff1c  NR32
    switch((data >> 5) & 3) {
      case 0: volume_shift = 4; break;  //  0%
      case 1: volume_shift = 0; break;  //100%
      case 2: volume_shift = 1; break;  // 50%
      case 3: volume_shift = 2; break;  // 25%
    }
  }

  if(r == 3) {  //$ff1d  NR33
    frequency = (frequency & 0x0700) | data;
  }

  if(r == 4) {  //$ff1e  NR34
    bool initialize = data & 0x80;
    counter = data & 0x40;
    frequency = ((data & 7) << 8) | (frequency & 0x00ff);

    if(initialize) {
      enable = dac_enable;
      pattern_offset = 0;
      if(length == 0) length = 256;
    }
  }

  period = 2 * (2048 - frequency);
}

void APU::Wave::write_pattern(unsigned p, uint8 data) {
  p <<= 1;
  pattern[p + 0] = (data >> 4) & 15;
  pattern[p + 1] = (data >> 0) & 15;
}

void APU::Wave::power() {
  enable = 0;

  dac_enable = 0;
  volume_shift = 0;
  frequency = 0;
  counter = 0;

  random_lfsr r;
  foreach(n, pattern) n = r() & 15;

  output = 0;
  length = 0;
  period = 0;
  pattern_offset = 0;
  pattern_sample = 0;
}

void APU::Wave::serialize(serializer &s) {
  s.integer(enable);

  s.integer(dac_enable);
  s.integer(volume_shift);
  s.integer(frequency);
  s.integer(counter);
  s.array(pattern);

  s.integer(output);
  s.integer(length);
  s.integer(period);
  s.integer(pattern_offset);
  s.integer(pattern_sample);
}

#endif
