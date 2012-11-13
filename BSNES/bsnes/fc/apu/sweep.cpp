bool APU::Sweep::check_period() {
  if(pulse_period > 0x7ff) return false;

  if(decrement == 0) {
    if((pulse_period + (pulse_period >> shift)) & 0x800) return false;
  }

  return true;
}

void APU::Sweep::clock(unsigned channel) {
  if(--counter == 0) {
    counter = period + 1;
    if(enable && shift && pulse_period > 8) {
      signed delta = pulse_period >> shift;

      if(decrement) {
        pulse_period -= delta;
        if(channel == 0) pulse_period--;
      } else if((pulse_period + delta) < 0x800) {
        pulse_period += delta;
      }
    }
  }

  if(reload) {
    reload = false;
    counter = period + 1;
  }
}

void APU::Sweep::power() {
  shift = 0;
  decrement = 0;
  period = 0;
  counter = 1;
  enable = 0;
  reload = 0;
  pulse_period = 0;
}

void APU::Sweep::reset() {
}

void APU::Sweep::serialize(serializer &s) {
  s.integer(shift);
  s.integer(decrement);
  s.integer(period);
  s.integer(counter);
  s.integer(enable);
  s.integer(reload);
  s.integer(pulse_period);
}
