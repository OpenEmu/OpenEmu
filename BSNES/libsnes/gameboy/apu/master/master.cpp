#ifdef APU_CPP

void APU::Master::run() {
  if(enable == false) {
    center = 0;
    left   = 0;
    right  = 0;
    return;
  }

  signed sample = 0;
  sample += apu.square1.output;
  sample += apu.square2.output;
  sample +=    apu.wave.output;
  sample +=   apu.noise.output;
  center = (sample * 512) - 16384;

  sample = 0;
  if(channel1_left_enable) sample += apu.square1.output;
  if(channel2_left_enable) sample += apu.square2.output;
  if(channel3_left_enable) sample +=    apu.wave.output;
  if(channel4_left_enable) sample +=   apu.noise.output;
  left = (sample * 512) - 16384;

  switch(left_volume) {
    case 0: left >>= 3;                       break;  // 12.5%
    case 1: left >>= 2;                       break;  // 25.0%
    case 2: left = (left >> 2) + (left >> 3); break;  // 37.5%
    case 3: left >>= 1;                       break;  // 50.0%
    case 4: left = (left >> 1) + (left >> 3); break;  // 62.5%
    case 5: left -= (left >> 2);              break;  // 75.0%
    case 6: left -= (left >> 3);              break;  // 87.5%
  //case 7:                                   break;  //100.0%
  }

  sample = 0;
  if(channel1_right_enable) sample += apu.square1.output;
  if(channel2_right_enable) sample += apu.square2.output;
  if(channel3_right_enable) sample +=    apu.wave.output;
  if(channel4_right_enable) sample +=   apu.noise.output;
  right = (sample * 512) - 16384;

  switch(right_volume) {
    case 0: right >>= 3;                         break;  // 12.5%
    case 1: right >>= 2;                         break;  // 25.0%
    case 2: right = (right >> 2) + (right >> 3); break;  // 37.5%
    case 3: right >>= 1;                         break;  // 50.0%
    case 4: right = (right >> 1) + (right >> 3); break;  // 62.5%
    case 5: right -= (right >> 2);               break;  // 75.0%
    case 6: right -= (right >> 3);               break;  // 87.5%
  //case 7:                                      break;  //100.0%
  }
}

void APU::Master::write(unsigned r, uint8 data) {
  if(r == 0) {  //$ff24  NR50
    left_in_enable  = data & 0x80;
    left_volume     = (data >> 4) & 7;
    right_in_enable = data & 0x08;
    right_volume    = (data >> 0) & 7;
  }

  if(r == 1) {  //$ff25  NR51
    channel4_left_enable  = data & 0x80;
    channel3_left_enable  = data & 0x40;
    channel2_left_enable  = data & 0x20;
    channel1_left_enable  = data & 0x10;
    channel4_right_enable = data & 0x08;
    channel3_right_enable = data & 0x04;
    channel2_right_enable = data & 0x02;
    channel1_right_enable = data & 0x01;
  }

  if(r == 2) {  //$ff26  NR52
    enable = data & 0x80;
  }
}

void APU::Master::power() {
  left_in_enable = 0;
  left_volume = 0;
  right_in_enable = 0;
  right_volume = 0;
  channel4_left_enable  = 0;
  channel3_left_enable  = 0;
  channel2_left_enable  = 0;
  channel1_left_enable  = 0;
  channel4_right_enable = 0;
  channel3_right_enable = 0;
  channel2_right_enable = 0;
  channel1_right_enable = 0;
  enable = 0;

  center = 0;
  left   = 0;
  right  = 0;
}

void APU::Master::serialize(serializer &s) {
  s.integer(left_in_enable);
  s.integer(left_volume);
  s.integer(right_in_enable);
  s.integer(right_volume);
  s.integer(channel4_left_enable);
  s.integer(channel3_left_enable);
  s.integer(channel2_left_enable);
  s.integer(channel1_left_enable);
  s.integer(channel4_right_enable);
  s.integer(channel3_right_enable);
  s.integer(channel2_right_enable);
  s.integer(channel1_right_enable);
  s.integer(enable);

  s.integer(center);
  s.integer(left);
  s.integer(right);
}

#endif
