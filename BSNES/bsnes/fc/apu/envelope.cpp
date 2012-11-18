unsigned APU::Envelope::volume() const {
  return use_speed_as_volume ? speed : decay_volume;
}

void APU::Envelope::clock() {
  if(reload_decay) {
    reload_decay = false;
    decay_volume = 0x0f;
    decay_counter = speed + 1;
    return;
  }

  if(--decay_counter == 0) {
    decay_counter = speed + 1;
    if(decay_volume || loop_mode) decay_volume--;
  }
}

void APU::Envelope::power() {
}

void APU::Envelope::reset() {
  speed = 0;
  use_speed_as_volume = 0;
  loop_mode = 0;
  reload_decay = 0;
  decay_counter = 0;
  decay_volume = 0;
}

void APU::Envelope::serialize(serializer &s) {
  s.integer(speed);
  s.integer(use_speed_as_volume);
  s.integer(loop_mode);

  s.integer(reload_decay);
  s.integer(decay_counter);
  s.integer(decay_volume);
}
