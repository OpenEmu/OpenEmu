CPU::Registers::DMAControl::operator uint16() const {
  return (
    (targetmode <<  5)
  | (sourcemode <<  7)
  | (repeat     <<  9)
  | (size       << 10)
  | (drq        << 11)
  | (timingmode << 12)
  | (irq        << 14)
  | (enable     << 15)
  );
}

uint16 CPU::Registers::DMAControl::operator=(uint16 source) {
  targetmode = source >>  5;
  sourcemode = source >>  7;
  repeat     = source >>  9;
  size       = source >> 10;
  drq        = source >> 11;
  timingmode = source >> 12;
  irq        = source >> 14;
  enable     = source >> 15;
  return operator uint16();
}

CPU::Registers::TimerControl::operator uint16() const {
  return (
    (frequency << 0)
  | (cascade   << 2)
  | (irq       << 6)
  | (enable    << 7)
  );
}

uint16 CPU::Registers::TimerControl::operator=(uint16 source) {
  frequency = source >> 0;
  cascade   = source >> 2;
  irq       = source >> 6;
  enable    = source >> 7;
  return operator uint16();
}

CPU::Registers::SerialControl::operator uint16() const {
  return (
    (shiftclockselect      <<  0)
  | (shiftclockfrequency   <<  1)
  | (transferenablereceive <<  2)
  | (transferenablesend    <<  3)
  | (startbit              <<  7)
  | (transferlength        << 12)
  | (irqenable             << 14)
  );
}

uint16 CPU::Registers::SerialControl::operator=(uint16 source) {
  shiftclockselect      = source >>  0;
  shiftclockfrequency   = source >>  1;
  transferenablereceive = source >>  2;
  transferenablesend    = source >>  3;
  startbit              = source >>  7;
  transferlength        = source >> 12;
  irqenable             = source >> 14;
  return operator uint16();
}

CPU::Registers::KeypadControl::operator uint16() const {
  return (
    (flag[0]   <<  0)
  | (flag[1]   <<  1)
  | (flag[2]   <<  2)
  | (flag[3]   <<  3)
  | (flag[4]   <<  4)
  | (flag[5]   <<  5)
  | (flag[6]   <<  6)
  | (flag[7]   <<  7)
  | (flag[8]   <<  8)
  | (flag[9]   <<  9)
  | (enable    << 14)
  | (condition << 15)
  );
}

uint16 CPU::Registers::KeypadControl::operator=(uint16 source) {
  flag[0]   = source >>  0;
  flag[1]   = source >>  1;
  flag[2]   = source >>  2;
  flag[3]   = source >>  3;
  flag[4]   = source >>  4;
  flag[5]   = source >>  5;
  flag[6]   = source >>  6;
  flag[7]   = source >>  7;
  flag[8]   = source >>  8;
  flag[9]   = source >>  9;
  enable    = source >> 14;
  condition = source >> 15;
  return operator uint16();
}

CPU::Registers::JoybusSettings::operator uint16() const {
  return (
    (sc        <<  0)
  | (sd        <<  1)
  | (si        <<  2)
  | (so        <<  3)
  | (scmode    <<  4)
  | (sdmode    <<  5)
  | (simode    <<  6)
  | (somode    <<  7)
  | (irqenable <<  8)
  | (mode      << 14)
  );
}

uint16 CPU::Registers::JoybusSettings::operator=(uint16 source) {
  sc        = source >>  0;
  sd        = source >>  1;
  si        = source >>  2;
  so        = source >>  3;
  scmode    = source >>  4;
  sdmode    = source >>  5;
  simode    = source >>  6;
  somode    = source >>  7;
  irqenable = source >>  8;
  mode      = source >> 14;
  return operator uint16();
}

CPU::Registers::JoybusControl::operator uint16() const {
  return (
    (resetsignal     << 0)
  | (receivecomplete << 1)
  | (sendcomplete    << 2)
  | (irqenable       << 6)
  );
}

uint16 CPU::Registers::JoybusControl::operator=(uint16 source) {
  resetsignal     = source >> 0;
  receivecomplete = source >> 1;
  sendcomplete    = source >> 2;
  irqenable       = source >> 6;
  return operator uint16();
}

CPU::Registers::JoybusStatus::operator uint16() const {
  return (
    (receiveflag << 1)
  | (sendflag    << 3)
  | (generalflag << 4)
  );
}

uint16 CPU::Registers::JoybusStatus::operator=(uint16 source) {
  receiveflag = source >> 1;
  sendflag    = source >> 3;
  generalflag = source >> 4;
  return operator uint16();
}

CPU::Registers::Interrupt::operator uint16() const {
  return (
    (vblank       <<  0)
  | (hblank       <<  1)
  | (vcoincidence <<  2)
  | (timer[0]     <<  3)
  | (timer[1]     <<  4)
  | (timer[2]     <<  5)
  | (timer[3]     <<  6)
  | (serial       <<  7)
  | (dma[0]       <<  8)
  | (dma[1]       <<  9)
  | (dma[2]       << 10)
  | (dma[3]       << 11)
  | (keypad       << 12)
  | (cartridge    << 13)
  );
}

uint16 CPU::Registers::Interrupt::operator=(uint16 source) {
  vblank       = source >>  0;
  hblank       = source >>  1;
  vcoincidence = source >>  2;
  timer[0]     = source >>  3;
  timer[1]     = source >>  4;
  timer[2]     = source >>  5;
  timer[3]     = source >>  6;
  serial       = source >>  7;
  dma[0]       = source >>  8;
  dma[1]       = source >>  9;
  dma[2]       = source >> 10;
  dma[3]       = source >> 11;
  keypad       = source >> 12;
  cartridge    = source >> 13;
  return operator uint16();
}

CPU::Registers::WaitControl::operator uint16() const {
  return (
    (nwait[3] <<  0)
  | (nwait[0] <<  2)
  | (swait[0] <<  4)
  | (nwait[1] <<  5)
  | (swait[1] <<  7)
  | (nwait[2] <<  8)
  | (swait[2] << 10)
  | (phi      << 11)
  | (prefetch << 14)
  | (gametype << 15)
  );
}

uint16 CPU::Registers::WaitControl::operator=(uint16 source) {
  nwait[3] = (source >>  0) & 3;
  nwait[0] = (source >>  2) & 3;
  swait[0] = (source >>  4) & 1;
  nwait[1] = (source >>  5) & 3;
  swait[1] = (source >>  7) & 1;
  nwait[2] = (source >>  8) & 3;
  swait[2] = (source >> 10) & 1;
  phi      = (source >> 11) & 3;
  prefetch = (source >> 14) & 1;
  gametype = (source >> 15) & 1;
  swait[3] = nwait[3];
  return operator uint16();
}

CPU::Registers::MemoryControl::operator uint32() const {
  return (
    (disable   <<  0)
  | (unknown1  <<  1)
  | (ewram     <<  5)
  | (ewramwait << 24)
  | (unknown2  << 28)
  );
}

uint32 CPU::Registers::MemoryControl::operator=(uint32 source) {
  disable   = source >>  0;
  unknown1  = source >>  1;
  ewram     = source >>  5;
  ewramwait = source >> 24;
  unknown2  = source >> 28;
  return operator uint32();
}
