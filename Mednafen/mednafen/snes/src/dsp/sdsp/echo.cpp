#ifdef SDSP_CPP

int sDSP::calc_fir(int i, bool channel) {
  int s = state.echo_hist[channel][state.echo_hist_pos + i + 1];
  return (s * (int8)REG(fir + i * 0x10)) >> 6;
}

int sDSP::echo_output(bool channel) {
  int output = (int16)((state.t_main_out[channel] * (int8)REG(mvoll + channel * 0x10)) >> 7)
             + (int16)((state.t_echo_in [channel] * (int8)REG(evoll + channel * 0x10)) >> 7);
  return sclamp<16>(output);
}

void sDSP::echo_read(bool channel) {
  unsigned addr = state.t_echo_ptr + channel * 2;
  uint8 lo = memory::apuram[(uint16)(addr + 0)];
  uint8 hi = memory::apuram[(uint16)(addr + 1)];
  int s = (int16)((hi << 8) + lo);
  state.echo_hist[channel].write(state.echo_hist_pos, s >> 1);
}

void sDSP::echo_write(bool channel) {
  if(!(state.t_echo_disabled & 0x20)) {
    unsigned addr = state.t_echo_ptr + channel * 2;
    int s = state.t_echo_out[channel];
    memory::apuram[(uint16)(addr + 0)] = s;
    memory::apuram[(uint16)(addr + 1)] = s >> 8;
  }

  state.t_echo_out[channel] = 0;
}

void sDSP::echo_22() {
  //history
  state.echo_hist_pos++;
  if(state.echo_hist_pos >= echo_hist_size) state.echo_hist_pos = 0;

  state.t_echo_ptr = (uint16)((state.t_esa << 8) + state.echo_offset);
  echo_read(0);

  //FIR
  int l = calc_fir(0, 0);
  int r = calc_fir(0, 1);

  state.t_echo_in[0] = l;
  state.t_echo_in[1] = r;
}

void sDSP::echo_23() {
  int l = calc_fir(1, 0) + calc_fir(2, 0);
  int r = calc_fir(1, 1) + calc_fir(2, 1);

  state.t_echo_in[0] += l;
  state.t_echo_in[1] += r;

  echo_read(1);
}

void sDSP::echo_24() {
  int l = calc_fir(3, 0) + calc_fir(4, 0) + calc_fir(5, 0);
  int r = calc_fir(3, 1) + calc_fir(4, 1) + calc_fir(5, 1);

  state.t_echo_in[0] += l;
  state.t_echo_in[1] += r;
}

void sDSP::echo_25() {
  int l = state.t_echo_in[0] + calc_fir(6, 0);
  int r = state.t_echo_in[1] + calc_fir(6, 1);

  l = (int16)l;
  r = (int16)r;

  l += (int16)calc_fir(7, 0);
  r += (int16)calc_fir(7, 1);

  state.t_echo_in[0] = sclamp<16>(l) & ~1;
  state.t_echo_in[1] = sclamp<16>(r) & ~1;
}

void sDSP::echo_26() {
  //left output volumes
  //(save sample for next clock so we can output both together)
  state.t_main_out[0] = echo_output(0);

  //echo feedback
  int l = state.t_echo_out[0] + (int16)((state.t_echo_in[0] * (int8)REG(efb)) >> 7);
  int r = state.t_echo_out[1] + (int16)((state.t_echo_in[1] * (int8)REG(efb)) >> 7);

  state.t_echo_out[0] = sclamp<16>(l) & ~1;
  state.t_echo_out[1] = sclamp<16>(r) & ~1;
}

void sDSP::echo_27() {
  //output
  int outl = state.t_main_out[0];
  int outr = echo_output(1);
  state.t_main_out[0] = 0;
  state.t_main_out[1] = 0;

  //TODO: global muting isn't this simple
  //(turns DAC on and off or something, causing small ~37-sample pulse when first muted)
  if(REG(flg) & 0x40) {
    outl = 0;
    outr = 0;
  }

  //output sample to DAC
  audio.sample(outl, outr);
}

void sDSP::echo_28() {
  state.t_echo_disabled = REG(flg);
}

void sDSP::echo_29() {
  state.t_esa = REG(esa);

  if(!state.echo_offset) state.echo_length = (REG(edl) & 0x0f) << 11;

  state.echo_offset += 4;
  if(state.echo_offset >= state.echo_length) state.echo_offset = 0;

  //write left echo
  echo_write(0);

  state.t_echo_disabled = REG(flg);
}

void sDSP::echo_30() {
  //write right echo
  echo_write(1);
}

#endif
