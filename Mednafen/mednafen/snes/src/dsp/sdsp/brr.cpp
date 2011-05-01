#ifdef SDSP_CPP

void sDSP::brr_decode(voice_t &v) {
  //state.t_brr_byte = ram[v.brr_addr + v.brr_offset] cached from previous clock cycle
  int nybbles = (state.t_brr_byte << 8) + memory::apuram[(uint16)(v.brr_addr + v.brr_offset + 1)];

  const int filter = (state.t_brr_header >> 2) & 3;
  const int scale  = (state.t_brr_header >> 4);

  //decode four samples
  for(unsigned i = 0; i < 4; i++) {
    //bits 12-15 = current nybble; sign extend, then shift right to 4-bit precision
    //result: s = 4-bit sign-extended sample value
    int s = (int16)nybbles >> 12;
    nybbles <<= 4; //slide nybble so that on next loop iteration, bits 12-15 = current nybble

    if(scale <= 12) {
      s <<= scale;
      s >>= 1;
    } else {
      s &= ~0x7ff;
    }

    //apply IIR filter (2 is the most commonly used)
    const int p1 = v.buffer[v.buf_pos - 1];
    const int p2 = v.buffer[v.buf_pos - 2] >> 1;

    switch(filter) {
      case 0: break; //no filter

      case 1: {
        //s += p1 * 0.46875
        s += p1 >> 1;
        s += (-p1) >> 5;
      } break;

      case 2: {
        //s += p1 * 0.953125 - p2 * 0.46875
        s += p1;
        s -= p2;
        s += p2 >> 4;
        s += (p1 * -3) >> 6;
      } break;

      case 3: {
        //s += p1 * 0.8984375 - p2 * 0.40625
        s += p1;
        s -= p2;
        s += (p1 * -13) >> 7;
        s += (p2 * 3) >> 4;
      } break;
    }

    //adjust and write sample
    s = sclamp<16>(s);
    s = (int16)(s << 1);
    v.buffer.write(v.buf_pos++, s);
    if(v.buf_pos >= brr_buf_size) v.buf_pos = 0;
  }
}

#endif
