void APU::Square::run() {
  if(period && --period == 0) {
    period = 4 * (2048 - frequency);
    phase++;
    switch(duty) {
    case 0: signal = (phase == 6); break;  //_____-_
    case 1: signal = (phase >= 6); break;  //______--
    case 2: signal = (phase >= 4); break;  //____----
    case 3: signal = (phase <= 5); break;  //------__
    }
  }

  uint4 sample = volume;
  if(enable == false || signal == false) sample = 0;
  output = sample;
}

void APU::Square::clocklength() {
  if(enable && counter) {
    if(++length == 0) enable = false;
  }
}

void APU::Square::clockenvelope() {
  if(enable && envelope.frequency && --envelope.period == 0) {
    envelope.period = envelope.frequency;
    if(envelope.direction == 0 && volume >  0) volume--;
    if(envelope.direction == 1 && volume < 15) volume++;
  }
}
