void APU::Square1::runsweep(bool update) {
  if(sweep.enable == false) return;

  sweep.negate = sweep.direction;
  unsigned delta = shadowfrequency >> sweep.shift;
  signed updatefrequency = shadowfrequency + (sweep.negate ? -delta : delta);

  if(updatefrequency > 2047) {
    enable = false;
  } else if(sweep.shift && update) {
    shadowfrequency = updatefrequency;
    frequency = updatefrequency;
    period = 4 * (2048 - frequency);
  }
}

void APU::Square1::clocksweep() {
  if(enable && sweep.frequency && --sweep.period == 0) {
    sweep.period = sweep.frequency;
    runsweep(1);
    runsweep(0);
  }
}

uint8 APU::Square1::read(unsigned addr) const {
  switch(addr) {
  case 0: return (sweep.shift << 0) | (sweep.direction << 3) | (sweep.frequency << 4);
  case 1: return (length << 0) | (duty << 6);
  case 2: return (envelope.frequency << 0) | (envelope.direction << 3) | (envelope.volume << 4);
  case 3: return (frequency << 0);
  case 4: return (frequency >> 8) | (counter << 6) | (initialize << 7);
  }
}

void APU::Square1::write(unsigned addr, uint8 byte) {
  switch(addr) {
  case 0:  //NR10
    if(sweep.negate && sweep.direction && !(byte & 0x08)) enable = false;
    sweep.shift     = byte >> 0;
    sweep.direction = byte >> 3;
    sweep.frequency = byte >> 4;
    break;

  case 1:  //NR11
    length = byte >> 0;
    duty   = byte >> 6;
    break;

  case 2:  //NR12
    envelope.frequency = byte >> 0;
    envelope.direction = byte >> 3;
    envelope.volume    = byte >> 4;
    if(envelope.dacenable() == false) enable = false;
    break;

  case 3:  //NR13
    frequency = (frequency & 0xff00) | (byte << 0);
    break;

  case 4:  //NR14
    frequency = (frequency & 0x00ff) | (byte << 8);
    counter    = byte >> 6;
    initialize = byte >> 7;

    if(initialize) {
      enable = envelope.dacenable();
      period = 4 * (2048 - frequency);
      envelope.period = envelope.frequency;
      volume = envelope.volume;
      shadowfrequency = frequency;
      sweep.period = sweep.frequency;
      sweep.enable = sweep.period || sweep.shift;
      sweep.negate = false;
      if(sweep.shift) runsweep(0);
    }

    break;
  }
}

void APU::Square1::power() {
  envelope.frequency = 0;
  envelope.direction = 0;
  envelope.direction = 0;
  envelope.period = 0;
  sweep.shift = 0;
  sweep.direction = 0;
  sweep.frequency = 0;
  sweep.enable = 0;
  sweep.negate = 0;
  sweep.period = 0;
  enable = 0;
  length = 0;
  duty = 0;
  frequency = 0;
  counter = 0;
  initialize = 0;
  shadowfrequency = 0;
  signal = 0;
  output = 0;
  period = 0;
  phase = 0;
  volume = 0;
}
