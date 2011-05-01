#ifdef SDSP_CPP

void sDSP::misc_27() {
  state.t_pmon = REG(pmon) & ~1; //voice 0 doesn't support PMON
}

void sDSP::misc_28() {
  state.t_non = REG(non);
  state.t_eon = REG(eon);
  state.t_dir = REG(dir);
}

void sDSP::misc_29() {
  state.every_other_sample ^= 1;
  if(state.every_other_sample) {
    state.new_kon &= ~state.kon; //clears KON 63 clocks after it was last read
  }
}

void sDSP::misc_30() {
  if(state.every_other_sample) {
    state.kon = state.new_kon;
    state.t_koff = REG(koff);
  }

  counter_tick();

  //noise
  if(counter_poll(REG(flg) & 0x1f) == true) {
    int feedback = (state.noise << 13) ^ (state.noise << 14);
    state.noise = (feedback & 0x4000) ^ (state.noise >> 1);
  }
}

#endif
