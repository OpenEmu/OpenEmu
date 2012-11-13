#ifdef PPU_CPP

void PPU::Window::scanline() {
  x = 0;
}

void PPU::Window::run() {
  bool main, sub;
  one = (x >= regs.one_left && x <= regs.one_right);
  two = (x >= regs.two_left && x <= regs.two_right);
  x++;

  test(
    main, sub,
    regs.bg1_one_enable, regs.bg1_one_invert,
    regs.bg1_two_enable, regs.bg1_two_invert,
    regs.bg1_mask, regs.bg1_main_enable, regs.bg1_sub_enable
  );
  if(main) self.bg1.output.main.priority = 0;
  if(sub) self.bg1.output.sub.priority = 0;

  test(
    main, sub,
    regs.bg2_one_enable, regs.bg2_one_invert,
    regs.bg2_two_enable, regs.bg2_two_invert,
    regs.bg2_mask, regs.bg2_main_enable, regs.bg2_sub_enable
  );
  if(main) self.bg2.output.main.priority = 0;
  if(sub) self.bg2.output.sub.priority = 0;

  test(
    main, sub,
    regs.bg3_one_enable, regs.bg3_one_invert,
    regs.bg3_two_enable, regs.bg3_two_invert,
    regs.bg3_mask, regs.bg3_main_enable, regs.bg3_sub_enable
  );
  if(main) self.bg3.output.main.priority = 0;
  if(sub) self.bg3.output.sub.priority = 0;

  test(
    main, sub,
    regs.bg4_one_enable, regs.bg4_one_invert,
    regs.bg4_two_enable, regs.bg4_two_invert,
    regs.bg4_mask, regs.bg4_main_enable, regs.bg4_sub_enable
  );
  if(main) self.bg4.output.main.priority = 0;
  if(sub) self.bg4.output.sub.priority = 0;

  test(
    main, sub,
    regs.oam_one_enable, regs.oam_one_invert,
    regs.oam_two_enable, regs.oam_two_invert,
    regs.oam_mask, regs.oam_main_enable, regs.oam_sub_enable
  );
  if(main) self.sprite.output.main.priority = 0;
  if(sub) self.sprite.output.sub.priority = 0;

  test(
    main, sub,
    regs.col_one_enable, regs.col_one_invert,
    regs.col_two_enable, regs.col_two_invert,
    regs.col_mask, true, true
  );

  switch(regs.col_main_mask) {
    case 0: main = true; break;
    case 1: break;
    case 2: main = !main; break;
    case 3: main = false; break;
  }

  switch(regs.col_sub_mask) {
    case 0: sub = true; break;
    case 1: break;
    case 2: sub = !sub; break;
    case 3: sub = false; break;
  }

  output.main.color_enable = main;
  output.sub.color_enable = sub;
}

void PPU::Window::test(
  bool &main, bool &sub,
  bool one_enable, bool one_invert,
  bool two_enable, bool two_invert,
  uint8 mask, bool main_enable, bool sub_enable
) {
  bool one = Window::one ^ one_invert;
  bool two = Window::two ^ two_invert;
  bool output;

  if(one_enable == false && two_enable == false) {
    output = false;
  } else if(one_enable == true && two_enable == false) {
    output = one;
  } else if(one_enable == false && two_enable == true) {
    output = two;
  } else switch(mask) {
    case 0: output = (one | two) == 1; break;
    case 1: output = (one & two) == 1; break;
    case 2: output = (one ^ two) == 1; break;
    case 3: output = (one ^ two) == 0; break;
  }

  main = main_enable ? output : false;
  sub = sub_enable ? output : false;
}

void PPU::Window::reset() {
  regs.bg1_one_enable = random(false);
  regs.bg1_one_invert = random(false);
  regs.bg1_two_enable = random(false);
  regs.bg1_two_invert = random(false);
  regs.bg2_one_enable = random(false);
  regs.bg2_one_invert = random(false);
  regs.bg2_two_enable = random(false);
  regs.bg2_two_invert = random(false);
  regs.bg3_one_enable = random(false);
  regs.bg3_one_invert = random(false);
  regs.bg3_two_enable = random(false);
  regs.bg3_two_invert = random(false);
  regs.bg4_one_enable = random(false);
  regs.bg4_one_invert = random(false);
  regs.bg4_two_enable = random(false);
  regs.bg4_two_invert = random(false);
  regs.oam_one_enable = random(false);
  regs.oam_one_invert = random(false);
  regs.oam_two_enable = random(false);
  regs.oam_two_invert = random(false);
  regs.col_one_enable = random(false);
  regs.col_one_invert = random(false);
  regs.col_two_enable = random(false);
  regs.col_two_invert = random(false);
  regs.one_left = random(0x00);
  regs.one_right = random(0x00);
  regs.two_left = random(0x00);
  regs.two_right = random(0x00);
  regs.bg1_mask = random(0);
  regs.bg2_mask = random(0);
  regs.bg3_mask = random(0);
  regs.bg4_mask = random(0);
  regs.oam_mask = random(0);
  regs.col_mask = random(0);
  regs.bg1_main_enable = random(false);
  regs.bg1_sub_enable = random(false);
  regs.bg2_main_enable = random(false);
  regs.bg2_sub_enable = random(false);
  regs.bg3_main_enable = random(false);
  regs.bg3_sub_enable = random(false);
  regs.bg4_main_enable = random(false);
  regs.bg4_sub_enable = random(false);
  regs.oam_main_enable = random(false);
  regs.oam_sub_enable = random(false);
  regs.col_main_mask = random(0);
  regs.col_sub_mask = random(0);

  output.main.color_enable = 0;
  output.sub.color_enable = 0;

  x = 0;
  one = 0;
  two = 0;
}

PPU::Window::Window(PPU &self) : self(self) {
}

#endif
