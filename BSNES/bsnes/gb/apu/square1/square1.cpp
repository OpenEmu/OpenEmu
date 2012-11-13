#ifdef APU_CPP

bool APU::Square1::dac_enable() {
  return (envelope_volume || envelope_direction);
}

void APU::Square1::run() {
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

void APU::Square1::sweep(bool update) {
  if(sweep_enable == false) return;

  sweep_negate = sweep_direction;
  unsigned delta = frequency_shadow >> sweep_shift;
  signed freq = frequency_shadow + (sweep_negate ? -delta : delta);

  if(freq > 2047) {
    enable = false;
  } else if(sweep_shift && update) {
    frequency_shadow = freq;
    frequency = freq & 2047;
    period = 4 * (2048 - frequency);
  }
}

void APU::Square1::clock_length() {
//if(counter && length) {
//  if(--length == 0) enable = false;
//}

  if(counter && enable) {
    if(++length == 0) enable = false;
  }
}

void APU::Square1::clock_sweep() {
  if(enable && sweep_frequency && --sweep_period == 0) {
    sweep_period = sweep_frequency;
    sweep(1);
    sweep(0);
  }
}

void APU::Square1::clock_envelope() {
  if(enable && envelope_frequency && --envelope_period == 0) {
    envelope_period = envelope_frequency;
    if(envelope_direction == 0 && volume >  0) volume--;
    if(envelope_direction == 1 && volume < 15) volume++;
  }
}

void APU::Square1::write(unsigned r, uint8 data) {
  if(r == 0) {  //$ff10  NR10
    if(sweep_negate && sweep_direction && !(data & 0x08)) enable = false;
    sweep_frequency = (data >> 4) & 7;
    sweep_direction = data & 0x08;
    sweep_shift = data & 0x07;
  }

  if(r == 1) {  //$ff11  NR11
    duty = data >> 6;
  //length = 64 - (data & 0x3f);
    length = data & 0x3f;
  }

  if(r == 2) {  //$ff12  NR12
    envelope_volume = data >> 4;
    envelope_direction = data & 0x08;
    envelope_frequency = data & 0x07;
    if(dac_enable() == false) enable = false;
  }

  if(r == 3) {  //$ff13  NR13
    frequency = (frequency & 0x0700) | data;
  }

  if(r == 4) {  //$ff14  NR14
    bool initialize = data & 0x80;
    counter = data & 0x40;
    frequency = ((data & 7) << 8) | (frequency & 0x00ff);

    if(initialize) {
      enable = dac_enable();
      period = 4 * (2048 - frequency);
      envelope_period = envelope_frequency;
      volume = envelope_volume;
      frequency_shadow = frequency;
      sweep_period = sweep_frequency;
      sweep_enable = sweep_period || sweep_shift;
      sweep_negate = false;
      if(sweep_shift) sweep(0);
    //if(length == 0) length = 64;
    }
  }
}

void APU::Square1::power() {
  enable = 0;

  sweep_frequency = 0;
  sweep_direction = 0;
  sweep_shift = 0;
  sweep_negate = 0;
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
  sweep_period = 0;
  frequency_shadow = 0;
  sweep_enable = 0;
  volume = 0;
}

void APU::Square1::serialize(serializer &s) {
  s.integer(enable);

  s.integer(sweep_frequency);
  s.integer(sweep_direction);
  s.integer(sweep_shift);
  s.integer(sweep_negate);
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
  s.integer(sweep_period);
  s.integer(frequency_shadow);
  s.integer(sweep_enable);
  s.integer(volume);
}

#endif
