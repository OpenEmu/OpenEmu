#ifdef PPU_CPP

void PPU::LayerWindow::render(bool screen) {
  uint8 *output;
  if(screen == 0) {
    output = main;
    if(main_enable == false) {
      memset(output, 0, 256);
      return;
    }
  } else {
    output = sub;
    if(sub_enable == false) {
      memset(output, 0, 256);
      return;
    }
  }

  if(one_enable == false && two_enable == false) {
    memset(output, 0, 256);
    return;
  }

  if(one_enable == true && two_enable == false) {
    bool set = 1 ^ one_invert, clr = !set;
    for(unsigned x = 0; x < 256; x++) {
      output[x] = (x >= ppu.regs.window_one_left && x <= ppu.regs.window_one_right) ? set : clr;
    }
    return;
  }

  if(one_enable == false && two_enable == true) {
    bool set = 1 ^ two_invert, clr = !set;
    for(unsigned x = 0; x < 256; x++) {
      output[x] = (x >= ppu.regs.window_two_left && x <= ppu.regs.window_two_right) ? set : clr;
    }
    return;
  }

  for(unsigned x = 0; x < 256; x++) {
    bool one_mask = (x >= ppu.regs.window_one_left && x <= ppu.regs.window_one_right) ^ one_invert;
    bool two_mask = (x >= ppu.regs.window_two_left && x <= ppu.regs.window_two_right) ^ two_invert;
    switch(mask) {
      case 0: output[x] = one_mask | two_mask == 1; break;
      case 1: output[x] = one_mask & two_mask == 1; break;
      case 2: output[x] = one_mask ^ two_mask == 1; break;
      case 3: output[x] = one_mask ^ two_mask == 0; break;
    }
  }
}

//

void PPU::ColorWindow::render(bool screen) {
  uint8 *output = (screen == 0 ? main : sub);
  bool set = 1, clr = 0;

  switch(screen == 0 ? main_mask : sub_mask) {
    case 0: memset(output, 1, 256); return;  //always
    case 1: set = 1, clr = 0; break;         //inside window only
    case 2: set = 0, clr = 1; break;         //outside window only
    case 3: memset(output, 0, 256); return;  //never
  }

  if(one_enable == false && two_enable == false) {
    memset(output, clr, 256);
    return;
  }

  if(one_enable == true && two_enable == false) {
    if(one_invert) { set ^= 1; clr ^= 1; }
    for(unsigned x = 0; x < 256; x++) {
      output[x] = (x >= ppu.regs.window_one_left && x <= ppu.regs.window_one_right) ? set : clr;
    }
    return;
  }

  if(one_enable == false && two_enable == true) {
    if(two_invert) { set ^= 1; clr ^= 1; }
    for(unsigned x = 0; x < 256; x++) {
      output[x] = (x >= ppu.regs.window_two_left && x <= ppu.regs.window_two_right) ? set : clr;
    }
    return;
  }

  for(unsigned x = 0; x < 256; x++) {
    bool one_mask = (x >= ppu.regs.window_one_left && x <= ppu.regs.window_one_right) ^ one_invert;
    bool two_mask = (x >= ppu.regs.window_two_left && x <= ppu.regs.window_two_right) ^ two_invert;
    switch(mask) {
      case 0: output[x] = one_mask | two_mask == 1 ? set : clr; break;
      case 1: output[x] = one_mask & two_mask == 1 ? set : clr; break;
      case 2: output[x] = one_mask ^ two_mask == 1 ? set : clr; break;
      case 3: output[x] = one_mask ^ two_mask == 0 ? set : clr; break;
    }
  }
}

#endif
