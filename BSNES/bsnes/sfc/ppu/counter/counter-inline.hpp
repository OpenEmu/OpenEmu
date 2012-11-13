//this should only be called by CPU::PPUcounter::tick();
//keeps track of previous counter positions in history table
void PPUcounter::tick() {
  status.hcounter += 2;  //increment by smallest unit of time
  if(status.hcounter >= 1360 && status.hcounter == lineclocks()) {
    status.hcounter = 0;
    vcounter_tick();
  }

  history.index = (history.index + 1) & 2047;
  history.field   [history.index] = status.field;
  history.vcounter[history.index] = status.vcounter;
  history.hcounter[history.index] = status.hcounter;
}

//this should only be called by PPU::PPUcounter::tick(n);
//allows stepping by more than the smallest unit of time
void PPUcounter::tick(unsigned clocks) {
  status.hcounter += clocks;
  if(status.hcounter >= lineclocks()) {
    status.hcounter -= lineclocks();
    vcounter_tick();
  }
}

//internal
void PPUcounter::vcounter_tick() {
  if(++status.vcounter == 128) status.interlace = ppu.interlace();

  if((system.region() == System::Region::NTSC && status.interlace == false && status.vcounter == 262)
  || (system.region() == System::Region::NTSC && status.interlace == true  && status.vcounter == 263)
  || (system.region() == System::Region::NTSC && status.interlace == true  && status.vcounter == 262 && status.field == 1)
  || (system.region() == System::Region::PAL  && status.interlace == false && status.vcounter == 312)
  || (system.region() == System::Region::PAL  && status.interlace == true  && status.vcounter == 313)
  || (system.region() == System::Region::PAL  && status.interlace == true  && status.vcounter == 312 && status.field == 1)
  ) {
    status.vcounter = 0;
    status.field = !status.field;
  }
  if(scanline) scanline();
}

bool   PPUcounter::field   () const { return status.field; }
uint16 PPUcounter::vcounter() const { return status.vcounter; }
uint16 PPUcounter::hcounter() const { return status.hcounter; }

bool   PPUcounter::field   (unsigned offset) const { return history.field   [(history.index - (offset >> 1)) & 2047]; }
uint16 PPUcounter::vcounter(unsigned offset) const { return history.vcounter[(history.index - (offset >> 1)) & 2047]; }
uint16 PPUcounter::hcounter(unsigned offset) const { return history.hcounter[(history.index - (offset >> 1)) & 2047]; }

//one PPU dot = 4 CPU clocks
//
//PPU dots 323 and 327 are 6 CPU clocks long.
//this does not apply to NTSC non-interlace scanline 240 on odd fields. this is
//because the PPU skips one dot to alter the color burst phase of the video signal.
//
//dot 323 range = { 1292, 1294, 1296 }
//dot 327 range = { 1310, 1312, 1314 }

uint16 PPUcounter::hdot() const {
  if(system.region() == System::Region::NTSC && status.interlace == false && vcounter() == 240 && field() == 1) {
    return (hcounter() >> 2);
  } else {
    return (hcounter() - ((hcounter() > 1292) << 1) - ((hcounter() > 1310) << 1)) >> 2;
  }
}

uint16 PPUcounter::lineclocks() const {
  if(system.region() == System::Region::NTSC && status.interlace == false &&  vcounter() == 240 &&  field() == 1) return 1360;
  return 1364;
}

void PPUcounter::reset() {
  status.interlace = false;
  status.field     = 0;
  status.vcounter  = 0;
  status.hcounter  = 0;
  history.index    = 0;

  for(unsigned i = 0; i < 2048; i++) {
    history.field   [i] = 0;
    history.vcounter[i] = 0;
    history.hcounter[i] = 0;
  }
}
