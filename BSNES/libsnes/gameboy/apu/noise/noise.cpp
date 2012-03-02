#ifdef APU_CPP

bool APU::Noise::dac_enable() {
  return (envelope_volume || envelope_direction);
}

void APU::Noise::run() {
  if(period && --period == 0) {
    period = divisor << frequency;
    if(frequency < 14) {
      bool bit = (lfsr ^ (lfsr >> 1)) & 1;
      lfsr = (lfsr >> 1) ^ (bit << (narrow_lfsr ? 6 : 14));
    }
  }

  uint4 sample = (lfsr & 1) ? (uint4)0 : volume;
  if(enable == false) sample = 0;

  output = sample;
}

void APU::Noise::clock_length() {
  if(counter && length) {
    if(--length == 0) enable = false;
  }
}

void APU::Noise::clock_envelope() {
  if(enable && envelope_frequency && --envelope_period == 0) {
    envelope_period = envelope_frequency;
    if(envelope_direction == 0 && volume >  0) volume--;
    if(envelope_direction == 1 && volume < 15) volume++;
  }
}

void APU::Noise::write(unsigned r, uint8 data) {
  if(r == 1) {  //$ff20  NR41
    length = 64 - (data & 0x3f);
  }

  if(r == 2) {  //$ff21  NR42
    envelope_volume = data >> 4;
    envelope_direction = data & 0x08;
    envelope_frequency = data & 0x07;
    if(dac_enable() == false) enable = false;
  }

  if(r == 3) {  //$ff22  NR43
    frequency = data >> 4;
    narrow_lfsr = data & 0x08;
    divisor = (data & 0x07) << 4;
    if(divisor == 0) divisor = 8;
    period = divisor << frequency;
  }

  if(r == 4) {  //$ff34  NR44
    bool initialize = data & 0x80;
    counter = data & 0x40;

    if(initialize) {
      enable = dac_enable();
      lfsr = ~0U;
      envelope_period = envelope_frequency;
      volume = envelope_volume;
      if(length == 0) length = 64;
    }
  }
}

void APU::Noise::power() {
  enable = 0;

  envelope_volume = 0;
  envelope_direction = 0;
  envelope_frequency = 0;
  frequency = 0;
  narrow_lfsr = 0;
  divisor = 0;
  counter = 0;

  output = 0;
  length = 0;
  envelope_period = 0;
  volume = 0;
  period = 0;
  lfsr = 0;
}

void APU::Noise::serialize(serializer &s) {
  s.integer(enable);

  s.integer(envelope_volume);
  s.integer(envelope_direction);
  s.integer(envelope_frequency);
  s.integer(frequency);
  s.integer(narrow_lfsr);
  s.integer(divisor);
  s.integer(counter);

  s.integer(output);
  s.integer(length);
  s.integer(envelope_period);
  s.integer(volume);
  s.integer(period);
  s.integer(lfsr);
}

#endif
