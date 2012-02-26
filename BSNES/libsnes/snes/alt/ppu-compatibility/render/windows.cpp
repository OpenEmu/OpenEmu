#ifdef PPU_CPP

//screen: 0 = main, 1 = sub
void PPU::build_window_table(uint8 bg, bool screen) {
  bool set = 1, clr = 0;
  uint8 *table = (screen == 0 ? window[bg].main : window[bg].sub);

  if(bg != COL) {
    if(screen == 0 && regs.window_enabled[bg] == false) {
      memset(table, 0, 256);
      return;
    }
    if(screen == 1 && regs.sub_window_enabled[bg] == false) {
      memset(table, 0, 256);
      return;
    }
  } else {
    switch(screen == 0 ? regs.color_mask : regs.colorsub_mask) {
      case 0: memset(table, 1, 256); return;  //always
      case 3: memset(table, 0, 256); return;  //never
      case 1: set = 1, clr = 0; break;        //inside window only
      case 2: set = 0, clr = 1; break;        //outside window only
    }
  }

  const uint16 window1_left  = regs.window1_left;
  const uint16 window1_right = regs.window1_right;
  const uint16 window2_left  = regs.window2_left;
  const uint16 window2_right = regs.window2_right;

  if(regs.window1_enabled[bg] == false && regs.window2_enabled[bg] == false) {
    memset(table, clr, 256);
    return;
  }

  if(regs.window1_enabled[bg] == true && regs.window2_enabled[bg] == false) {
    if(regs.window1_invert[bg] == true) set ^= clr ^= set ^= clr;
    for(unsigned x = 0; x < 256; x++) {
      table[x] = (x >= window1_left && x <= window1_right) ? set : clr;
    }
    return;
  }

  if(regs.window1_enabled[bg] == false && regs.window2_enabled[bg] == true) {
    if(regs.window2_invert[bg] == true) set ^= clr ^= set ^= clr;
    for(unsigned x = 0; x < 256; x++) {
      table[x] = (x >= window2_left && x <= window2_right) ? set : clr;
    }
    return;
  }

  for(unsigned x = 0; x < 256; x++) {
    bool w1_mask = (x >= window1_left && x <= window1_right) ^ regs.window1_invert[bg];
    bool w2_mask = (x >= window2_left && x <= window2_right) ^ regs.window2_invert[bg];

    switch(regs.window_mask[bg]) {
      case 0: table[x] = (w1_mask | w2_mask) == 1 ? set : clr; break;  //or
      case 1: table[x] = (w1_mask & w2_mask) == 1 ? set : clr; break;  //and
      case 2: table[x] = (w1_mask ^ w2_mask) == 1 ? set : clr; break;  //xor
      case 3: table[x] = (w1_mask ^ w2_mask) == 0 ? set : clr; break;  //xnor
    }
  }
}

void PPU::build_window_tables(uint8 bg) {
  build_window_table(bg, 0);
  build_window_table(bg, 1);
}

#endif
