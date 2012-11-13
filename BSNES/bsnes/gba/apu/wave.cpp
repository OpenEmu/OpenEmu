void APU::Wave::run() {
  if(period && --period == 0) {
    period = 2 * (2048 - frequency);
    patternsample = pattern[patternbank * 16 + patternaddr++];
    if(patternaddr == 0) patternbank ^= mode;
  }

  output = patternsample;
  static unsigned multiplier[] = { 0, 4, 2, 1, 3, 3, 3, 3};
  output = (output * multiplier[volume]) / 4;
  if(enable == false) output = 0;
}

void APU::Wave::clocklength() {
  if(enable && counter) {
    if(++length == 0) enable = false;
  }
}

uint8 APU::Wave::read(unsigned addr) const {
  switch(addr) {
  case 0: return (mode << 5) | (bank << 6) | (dacenable << 7);
  case 1: return (length << 0);
  case 2: return (volume << 5);
  case 3: return (frequency >> 0);
  case 4: return (frequency >> 8) | (counter << 6) | (initialize << 7);
  }
}

void APU::Wave::write(unsigned addr, uint8 byte) {
  switch(addr) {
  case 0:  //NR30
    mode      = byte >> 5;
    bank      = byte >> 6;
    dacenable = byte >> 7;
    if(dacenable == false) enable = false;
    break;

  case 1:  //NR31
    length = byte >> 0;
    break;

  case 2:  //NR32
    volume = byte >> 5;
    break;

  case 3:  //NR33
    frequency = (frequency & 0xff00) | (byte << 0);
    break;

  case 4:  //NR34
    frequency = (frequency & 0x00ff) | (byte << 8);
    counter    = byte >> 6;
    initialize = byte >> 7;

    if(initialize) {
      enable = dacenable;
      period = 2 * (2048 - frequency);
      patternaddr = 0;
      patternbank = mode ? (uint1)0 : bank;
    }

    break;
  }
}

uint8 APU::Wave::readram(unsigned addr) const {
  uint8 byte = 0;
  byte |= pattern[addr * 2 + 0] << 0;
  byte |= pattern[addr * 2 + 1] << 4;
  return byte;
}

void APU::Wave::writeram(unsigned addr, uint8 byte) {
  pattern[addr * 2 + 0] = byte >> 0;
  pattern[addr * 2 + 1] = byte >> 4;
}

void APU::Wave::power() {
  mode = 0;
  bank = 0;
  dacenable = 0;
  length = 0;
  volume = 0;
  frequency = 0;
  counter = 0;
  initialize = 0;
  for(auto &sample : pattern) sample = 0;
  enable = 0;
  output = 0;
  patternaddr = 0;
  patternbank = 0;
  patternsample = 0;
  period = 0;
}
