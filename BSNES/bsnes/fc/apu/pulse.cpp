void APU::Pulse::clock_length() {
  if(envelope.loop_mode == 0) {
    if(length_counter) length_counter--;
  }
}

uint8 APU::Pulse::clock() {
  if(sweep.check_period() == false) return 0;
  if(length_counter == 0) return 0;

  static const unsigned duty_table[] = { 1, 2, 4, 6 };
  uint8 result = (duty_counter < duty_table[duty]) ? envelope.volume() : 0;
  if(sweep.pulse_period < 0x008) result = 0;

  if(--period_counter == 0) {
    period_counter = (sweep.pulse_period + 1) * 2;
    duty_counter++;
  }

  return result;
}

void APU::Pulse::power() {
  envelope.power();
  sweep.power();
}

void APU::Pulse::reset() {
  envelope.reset();
  sweep.reset();

  length_counter = 0;

  duty = 0;
  duty_counter = 0;
  period = 0;
  period_counter = 1;
}

void APU::Pulse::serialize(serializer &s) {
  s.integer(length_counter);

  envelope.serialize(s);
  sweep.serialize(s);

  s.integer(duty);
  s.integer(duty_counter);

  s.integer(period);
  s.integer(period_counter);
}
