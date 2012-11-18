void APU::runsequencer() {
  auto &r = sequencer;

  if(r.base == 0) {  //512hz
    if(r.step == 0 || r.step == 2 || r.step == 4 || r.step == 6) {  //256hz
      square1.clocklength();
      square2.clocklength();
      wave.clocklength();
      noise.clocklength();
    }
    if(r.step == 2 || r.step == 6) {  //128hz
      square1.clocksweep();
    }
    if(r.step == 7) {  //64hz
      square1.clockenvelope();
      square2.clockenvelope();
      noise.clockenvelope();
    }
    r.step++;
  }
  r.base++;

  if(r.enable[0]) square1.run();
  if(r.enable[1]) square2.run();
  if(r.enable[2])    wave.run();
  if(r.enable[3])   noise.run();
}

uint8 APU::Sequencer::read(unsigned addr) const {
  switch(addr) {
  case 0: return (rvolume << 0) | (lvolume << 4);
  case 1: return (
    (renable[0] << 0)
  | (renable[1] << 1)
  | (renable[2] << 2)
  | (renable[3] << 3)
  | (lenable[0] << 4)
  | (lenable[1] << 5)
  | (lenable[2] << 6)
  | (lenable[3] << 7)
  );
  case 2: return (
    (enable[0]    << 0)
  | (enable[1]    << 1)
  | (enable[2]    << 2)
  | (enable[3]    << 3)
  | (masterenable << 7)
  );
  }
}

void APU::Sequencer::write(unsigned addr, uint8 byte) {
  switch(addr) {
  case 0:  //NR50
    rvolume = byte >> 0;
    lvolume = byte >> 4;
    break;

  case 1:  //NR51
    renable[0] = byte >> 0;
    renable[1] = byte >> 1;
    renable[2] = byte >> 2;
    renable[3] = byte >> 3;
    lenable[0] = byte >> 4;
    lenable[1] = byte >> 5;
    lenable[2] = byte >> 6;
    lenable[3] = byte >> 7;
    break;

  case 2:  //NR52
    enable[0]    = byte >> 0;
    enable[1]    = byte >> 1;
    enable[2]    = byte >> 2;
    enable[3]    = byte >> 3;
    masterenable = byte >> 7;
    break;
  }
}

void APU::Sequencer::power() {
  lvolume = 0;
  rvolume = 0;
  for(auto &n : lenable) n = 0;
  for(auto &n : renable) n = 0;
  for(auto &n : enable) n = 0;
  masterenable = 0;
  base = 0;
  step = 0;
  lsample = 0;
  rsample = 0;
}
