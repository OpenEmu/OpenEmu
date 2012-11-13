uint8 APU::read(uint32 addr) {
  switch(addr) {

  //NR10
  case 0x04000060: return square1.read(0);
  case 0x04000061: return 0u;

  //NR11 + NR12
  case 0x04000062: return square1.read(1);
  case 0x04000063: return square1.read(2);

  //NR13 + NR14
  case 0x04000064: return square1.read(3);
  case 0x04000065: return square1.read(4);

  //NR21 + NR22
  case 0x04000068: return square2.read(1);
  case 0x04000069: return square2.read(2);

  //NR23 + NR24
  case 0x0400006c: return square2.read(3);
  case 0x0400006d: return square2.read(4);

  //NR30
  case 0x04000070: return wave.read(0);
  case 0x04000071: return 0u;

  //NR31 + NR32
  case 0x04000072: return wave.read(1);
  case 0x04000073: return wave.read(2);

  //NR33 + NR34
  case 0x04000074: return wave.read(3);
  case 0x04000075: return wave.read(4);

  //NR41 + NR42
  case 0x04000078: return noise.read(1);
  case 0x04000079: return noise.read(2);

  //NR43 + NR44
  case 0x0400007c: return noise.read(3);
  case 0x0400007d: return noise.read(4);

  //NR50 + NR51
  case 0x04000080: return sequencer.read(0);
  case 0x04000081: return sequencer.read(1);

  //NR52
  case 0x04000084: return sequencer.read(2);
  case 0x04000085: return 0u;

  //SOUNDBIAS
  case 0x04000088: return regs.bias >> 0;
  case 0x04000089: return regs.bias >> 8;

  //WAVE_RAM0_L
  case 0x04000090: return wave.readram( 0);
  case 0x04000091: return wave.readram( 1);

  //WAVE_RAM0_H
  case 0x04000092: return wave.readram( 2);
  case 0x04000093: return wave.readram( 3);

  //WAVE_RAM1_L
  case 0x04000094: return wave.readram( 4);
  case 0x04000095: return wave.readram( 5);

  //WAVE_RAM1_H
  case 0x04000096: return wave.readram( 6);
  case 0x04000097: return wave.readram( 7);

  //WAVE_RAM2_L
  case 0x04000098: return wave.readram( 8);
  case 0x04000099: return wave.readram( 9);

  //WAVE_RAM2_H
  case 0x0400009a: return wave.readram(10);
  case 0x0400009b: return wave.readram(11);

  //WAVE_RAM3_L
  case 0x0400009c: return wave.readram(12);
  case 0x0400009d: return wave.readram(13);

  //WAVE_RAM3_H
  case 0x0400009e: return wave.readram(14);
  case 0x0400009f: return wave.readram(15);

  }

  return 0u;
}

void APU::write(uint32 addr, uint8 byte) {
  switch(addr) {

  //NR10
  case 0x04000060: return square1.write(0, byte);
  case 0x04000061: return;

  //NR11 + NR12
  case 0x04000062: return square1.write(1, byte);
  case 0x04000063: return square1.write(2, byte);

  //NR13 + NR14
  case 0x04000064: return square1.write(3, byte);
  case 0x04000065: return square1.write(4, byte);

  //NR21 + NR22
  case 0x04000068: return square2.write(1, byte);
  case 0x04000069: return square2.write(2, byte);

  //NR23 + NR24
  case 0x0400006c: return square2.write(3, byte);
  case 0x0400006d: return square2.write(4, byte);

  //NR30
  case 0x04000070: return wave.write(0, byte);
  case 0x04000071: return;

  //NR31 + NR32
  case 0x04000072: return wave.write(1, byte);
  case 0x04000073: return wave.write(2, byte);

  //NR33 + NR34
  case 0x04000074: return wave.write(3, byte);
  case 0x04000075: return wave.write(4, byte);

  //NR41 + NR42
  case 0x04000078: return noise.write(1, byte);
  case 0x04000079: return noise.write(2, byte);

  //NR43 + NR44
  case 0x0400007c: return noise.write(3, byte);
  case 0x0400007d: return noise.write(4, byte);

  //NR50 + NR51
  case 0x04000080: return sequencer.write(0, byte);
  case 0x04000081: return sequencer.write(1, byte);

  //SOUND_CNT_H
  case 0x04000082:
    sequencer.volume = byte >> 0;
    fifo[0].volume   = byte >> 2;
    fifo[1].volume   = byte >> 3;
    return;
  case 0x04000083:
    fifo[0].renable = byte >> 0;
    fifo[0].lenable = byte >> 1;
    fifo[0].timer   = byte >> 2;
    if(byte & 1 << 3) fifo[0].reset();
    fifo[1].renable = byte >> 4;
    fifo[1].lenable = byte >> 5;
    fifo[1].timer   = byte >> 6;
    if(byte & 1 << 7) fifo[1].reset();
    return;

  //NR52
  case 0x04000084: return sequencer.write(2, byte);
  case 0x04000085: return;

  //SOUNDBIAS
  case 0x04000088: regs.bias = (regs.bias & 0xff00) | (byte << 0); return;
  case 0x04000089: regs.bias = (regs.bias & 0x00ff) | (byte << 8); return;

  //WAVE_RAM0_L
  case 0x04000090: return wave.writeram( 0, byte);
  case 0x04000091: return wave.writeram( 1, byte);

  //WAVE_RAM0_H
  case 0x04000092: return wave.writeram( 2, byte);
  case 0x04000093: return wave.writeram( 3, byte);

  //WAVE_RAM1_L
  case 0x04000094: return wave.writeram( 4, byte);
  case 0x04000095: return wave.writeram( 5, byte);

  //WAVE_RAM1_H
  case 0x04000096: return wave.writeram( 6, byte);
  case 0x04000097: return wave.writeram( 7, byte);

  //WAVE_RAM2_L
  case 0x04000098: return wave.writeram( 8, byte);
  case 0x04000099: return wave.writeram( 9, byte);

  //WAVE_RAM2_H
  case 0x0400009a: return wave.writeram(10, byte);
  case 0x0400009b: return wave.writeram(11, byte);

  //WAVE_RAM3_L
  case 0x0400009c: return wave.writeram(12, byte);
  case 0x0400009d: return wave.writeram(13, byte);

  //WAVE_RAM3_H
  case 0x0400009e: return wave.writeram(14, byte);
  case 0x0400009f: return wave.writeram(15, byte);

  //FIFO_A_L
  //FIFO_A_H
  case 0x040000a0: case 0x040000a1:
  case 0x040000a2: case 0x040000a3:
    return fifo[0].write(byte);

  //FIFO_B_L
  //FIFO_B_H
  case 0x040000a4: case 0x040000a5:
  case 0x040000a6: case 0x040000a7:
    return fifo[1].write(byte);
  }
}
