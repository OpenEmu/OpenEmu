#ifdef DSP_CPP

inline void DSP::voice_output(voice_t &v, bool channel) {
  //apply left/right volume
  int amp = (state.t_output * (int8)VREG(voll + channel)) >> 7;

  //add to output total
  state.t_main_out[channel] += amp;
  state.t_main_out[channel] = sclamp<16>(state.t_main_out[channel]);

  //optionally add to echo total
  if(state.t_eon & v.vbit) {
    state.t_echo_out[channel] += amp;
    state.t_echo_out[channel] = sclamp<16>(state.t_echo_out[channel]);
  }
}

void DSP::voice_1(voice_t &v) {
  state.t_dir_addr = (state.t_dir << 8) + (state.t_srcn << 2);
  state.t_srcn = VREG(srcn);
}

void DSP::voice_2(voice_t &v) {
  //read sample pointer (ignored if not needed)
  uint16 addr = state.t_dir_addr;
  if(!v.kon_delay) addr += 2;
  uint8 lo = smp.apuram[(uint16)(addr + 0)];
  uint8 hi = smp.apuram[(uint16)(addr + 1)];
  state.t_brr_next_addr = ((hi << 8) + lo);

  state.t_adsr0 = VREG(adsr0);

  //read pitch, spread over two clocks
  state.t_pitch = VREG(pitchl);
}

void DSP::voice_3(voice_t &v) {
  voice_3a(v);
  voice_3b(v);
  voice_3c(v);
}

void DSP::voice_3a(voice_t &v) {
  state.t_pitch += (VREG(pitchh) & 0x3f) << 8;
}

void DSP::voice_3b(voice_t &v) {
  state.t_brr_byte   = smp.apuram[(uint16)(v.brr_addr + v.brr_offset)];
  state.t_brr_header = smp.apuram[(uint16)(v.brr_addr)];
}

void DSP::voice_3c(voice_t &v) {
  //pitch modulation using previous voice's output

  if(state.t_pmon & v.vbit) {
    state.t_pitch += ((state.t_output >> 5) * state.t_pitch) >> 10;
  }

  if(v.kon_delay) {
    //get ready to start BRR decoding on next sample
    if(v.kon_delay == 5) {
      v.brr_addr   = state.t_brr_next_addr;
      v.brr_offset = 1;
      v.buf_pos    = 0;
      state.t_brr_header = 0; //header is ignored on this sample
    }

    //envelope is never run during KON
    v.env = 0;
    v.hidden_env = 0;

    //disable BRR decoding until last three samples
    v.interp_pos = 0;
    v.kon_delay--;
    if(v.kon_delay & 3) v.interp_pos = 0x4000;

    //pitch is never added during KON
    state.t_pitch = 0;
  }

  //gaussian interpolation
  int output = gaussian_interpolate(v);

  //noise
  if(state.t_non & v.vbit) {
    output = (int16)(state.noise << 1);
  }

  //apply envelope
  state.t_output = ((output * v.env) >> 11) & ~1;
  v.t_envx_out = v.env >> 4;

  //immediate silence due to end of sample or soft reset
  if(REG(flg) & 0x80 || (state.t_brr_header & 3) == 1) {
    v.env_mode = env_release;
    v.env = 0;
  }

  if(state.every_other_sample) {
    //KOFF
    if(state.t_koff & v.vbit) {
      v.env_mode = env_release;
    }

    //KON
    if(state.kon & v.vbit) {
      v.kon_delay = 5;
      v.env_mode = env_attack;
    }
  }

  //run envelope for next sample
  if(!v.kon_delay) envelope_run(v);
}

void DSP::voice_4(voice_t &v) {
  //decode BRR
  state.t_looped = 0;
  if(v.interp_pos >= 0x4000) {
    brr_decode(v);
    v.brr_offset += 2;
    if(v.brr_offset >= 9) {
      //start decoding next BRR block
      v.brr_addr = (uint16)(v.brr_addr + 9);
      if(state.t_brr_header & 1) {
        v.brr_addr = state.t_brr_next_addr;
        state.t_looped = v.vbit;
      }
      v.brr_offset = 1;
    }
  }

  //apply pitch
  v.interp_pos = (v.interp_pos & 0x3fff) + state.t_pitch;

  //keep from getting too far ahead (when using pitch modulation)
  if(v.interp_pos > 0x7fff) v.interp_pos = 0x7fff;

  //output left
  voice_output(v, 0);
}

void DSP::voice_5(voice_t &v) {
  //output right
  voice_output(v, 1);

  //ENDX, OUTX and ENVX won't update if you wrote to them 1-2 clocks earlier
  state.endx_buf = REG(endx) | state.t_looped;

  //clear bit in ENDX if KON just began
  if(v.kon_delay == 5) state.endx_buf &= ~v.vbit;
}

void DSP::voice_6(voice_t &v) {
  state.outx_buf = state.t_output >> 8;
}

void DSP::voice_7(voice_t &v) {
  //update ENDX
  REG(endx) = (uint8)state.endx_buf;
  state.envx_buf = v.t_envx_out;
}

void DSP::voice_8(voice_t &v) {
  //update OUTX
  VREG(outx) = (uint8)state.outx_buf;
}

void DSP::voice_9(voice_t &v) {
  //update ENVX
  VREG(envx) = (uint8)state.envx_buf;
}

#endif
