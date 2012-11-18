APU::Registers::SoundBias::operator uint16() const {
  return (
    (level     <<  0)
  | (amplitude << 14)
  );
}

uint16 APU::Registers::SoundBias::operator=(uint16 source) {
  level     = (source >>  0) & 1023;
  amplitude = (source >> 14) &    3;
  return operator uint16();
}
