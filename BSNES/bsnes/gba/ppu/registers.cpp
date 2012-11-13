PPU::Registers::Control::operator uint16() const {
  return (
    (bgmode            <<  0)
  | (cgbmode           <<  3)
  | (frame             <<  4)
  | (hblank            <<  5)
  | (objmapping        <<  6)
  | (forceblank        <<  7)
  | (enable[BG0]       <<  8)
  | (enable[BG1]       <<  9)
  | (enable[BG2]       << 10)
  | (enable[BG3]       << 11)
  | (enable[OBJ]       << 12)
  | (enablewindow[In0] << 13)
  | (enablewindow[In1] << 14)
  | (enablewindow[Obj] << 15)
  );
}

uint16 PPU::Registers::Control::operator=(uint16 source) {
  bgmode            = source >>  0;
  cgbmode           = source >>  3;
  frame             = source >>  4;
  hblank            = source >>  5;
  objmapping        = source >>  6;
  forceblank        = source >>  7;
  enable[BG0]       = source >>  8;
  enable[BG1]       = source >>  9;
  enable[BG2]       = source >> 10;
  enable[BG3]       = source >> 11;
  enable[OBJ]       = source >> 12;
  enablewindow[In0] = source >> 13;
  enablewindow[In1] = source >> 14;
  enablewindow[Obj] = source >> 15;
  return operator uint16();
}

PPU::Registers::Status::operator uint16() const {
  return (
    (vblank          << 0)
  | (hblank          << 1)
  | (vcoincidence    << 2)
  | (irqvblank       << 3)
  | (irqhblank       << 4)
  | (irqvcoincidence << 5)
  | (vcompare        << 8)
  );
}

uint16 PPU::Registers::Status::operator=(uint16 source) {
  vblank          = source >> 0;
  hblank          = source >> 1;
  vcoincidence    = source >> 2;
  irqvblank       = source >> 3;
  irqhblank       = source >> 4;
  irqvcoincidence = source >> 5;
  vcompare        = source >> 8;
  return operator uint16();
}

PPU::Registers::BackgroundControl::operator uint16() const {
  return (
    (priority           <<  0)
  | (characterbaseblock <<  2)
  | (mosaic             <<  6)
  | (colormode          <<  7)
  | (screenbaseblock    <<  8)
  | (affinewrap         << 13)
  | (screensize         << 14)
  );
}

uint16 PPU::Registers::BackgroundControl::operator=(uint16 source) {
  priority           = source >>  0;
  characterbaseblock = source >>  2;
  mosaic             = source >>  6;
  colormode          = source >>  7;
  screenbaseblock    = source >>  8;
  affinewrap         = source >> 13;
  screensize         = source >> 14;
  return operator uint16();
}

PPU::Registers::WindowFlags::operator uint8() const {
  return (
    (enable[BG0] << 0)
  | (enable[BG1] << 1)
  | (enable[BG2] << 2)
  | (enable[BG3] << 3)
  | (enable[OBJ] << 4)
  | (enable[SFX] << 5)
  );
}

uint8 PPU::Registers::WindowFlags::operator=(uint8 source) {
  enable[BG0] = source >> 0;
  enable[BG1] = source >> 1;
  enable[BG2] = source >> 2;
  enable[BG3] = source >> 3;
  enable[OBJ] = source >> 4;
  enable[SFX] = source >> 5;
  return operator uint8();
}

PPU::Registers::BlendControl::operator uint16() const {
  return (
    (above[BG0] <<  0)
  | (above[BG1] <<  1)
  | (above[BG2] <<  2)
  | (above[BG3] <<  3)
  | (above[OBJ] <<  4)
  | (above[SFX] <<  5)
  | (mode       <<  6)
  | (below[BG0] <<  8)
  | (below[BG1] <<  9)
  | (below[BG2] << 10)
  | (below[BG3] << 11)
  | (below[OBJ] << 12)
  | (below[SFX] << 13)
  );
}

uint16 PPU::Registers::BlendControl::operator=(uint16 source) {
  above[BG0] = source >>  0;
  above[BG1] = source >>  1;
  above[BG2] = source >>  2;
  above[BG3] = source >>  3;
  above[OBJ] = source >>  4;
  above[SFX] = source >>  5;
  mode       = source >>  6;
  below[BG0] = source >>  8;
  below[BG1] = source >>  9;
  below[BG2] = source >> 10;
  below[BG3] = source >> 11;
  below[OBJ] = source >> 12;
  below[SFX] = source >> 13;
  return operator uint16();
}
