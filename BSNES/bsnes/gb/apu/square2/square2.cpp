#ifdef APU_CPP

bool APU::Square2::dac_enable() {
  return (envelope_volume || envelope_direction);
}

void APU::Square2::run() {
  if(period && --period == 0) {
    period = 4 * (2048 - frequency);
    phase++;
    switch(duty) {
      case 0: duty_output = (phase == 6); break;  //______-_
      case 1: duty_output = (phase >= 6); break;  //______--
      case 2: duty_output = (phase >= 4); break;  //____----
      case 3: duty_output = (phase <= 5); break;  //------__
    }
  }

  uint4 sample = (duty_output ? volume : (uint4)0);
  if(enable == false) sample = 0;

  output = sample;
}

void APU::Square2::clock_length() {
//if(counter && length) {
//  if(--length == 0) enable = false;
//}

  if(counter && enable) {
    if(++length == 0) enable = false;
  }
}

void APU::Square2::clock_envelope() {
  if(enable && envelope_frequency && --envelope_period == 0) {
    envelope_period = envelope_frequency;
    if(envelope_direction == 0 && volume >  0) volume--;
    if(envelope_direction == 1 && volume < 15) volume++;
  }
}

void APU::Square2::write(unsigned r, uint8 data) {
  if(r == 1) {  //$ff16  NR21
    duty = data >> 6;
  //length = 64 - (data & 0x3f);
    length = (data & 0x3f);
  }

  if(r == 2) {  //$ff17  NR22
    envelope_volume = data >> 4;
    envelope_direction = data & 0x08;
    envelope_frequency = data & 0x07;
    if(dac_enable() == false) enable = false;
  }

  if(r == 3) {  //$ff18  NR23
    frequency = (frequency & 0x0700) | data;
  }

  if(r == 4) {  //$ff19  NR24
    bool initialize = data & 0x80;
    counter = data & 0x40;
    frequency = ((data & 7) << 8) | (frequency & 0x00ff);

    if(initialize) {
      enable = dac_enable();
      period = 4 * (2048 - frequency);
      envelope_period = envelope_frequency;
      volume = envelope_volume;
    //if(length == 0) length = 64;
    }
  }
}

void APU::Square2::power() {
  enable = 0;

  duty = 0;
  length = 0;
  envelope_volume = 0;
  envelope_direction = 0;
  envelope_frequency = 0;
  frequency = 0;
  counter = 0;

  output = 0;
  duty_output = 0;
  phase = 0;
  period = 0;
  envelope_period = 0;
  volume = 0;
}

void APU::Square2::serialize(serializer &s) {
  s.integer(enable);

  s.integer(duty);
  s.integer(length);
  s.integer(envelope_volume);
  s.integer(envelope_direction);
  s.integer(envelope_frequency);
  s.integer(frequency);
  s.integer(counter);

  s.integer(output);
  s.integer(duty_output);
  s.integer(phase);
  s.integer(period);
  s.integer(envelope_period);
  s.integer(volume);
}

#endif
