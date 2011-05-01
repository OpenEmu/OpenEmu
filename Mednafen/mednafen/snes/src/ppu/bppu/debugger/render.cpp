#ifdef BPPU_CPP

//render_line_modeN() taken from src/ppu/bppu/render/render.cpp
//modified to support layer disable; accomplished by setting priority to zero
//a priority of zero won't override the back layer, effectively nullifying it
//for speed, rendering loop is skipped entirely if all priorities are disabled
//
//note: render_line_(bg|oam|mode7) cannot be virtualized as they are templates

void bPPUDebugger::render_line_mode0() {
  unsigned pri0, pri1, pri2, pri3;

  pri0 = bg1_enabled[0] ?  8 : 0;
  pri1 = bg1_enabled[1] ? 11 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<0, BG1, COLORDEPTH_4>(pri0, pri1);

  pri0 = bg2_enabled[0] ?  7 : 0;
  pri1 = bg2_enabled[1] ? 10 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<0, BG2, COLORDEPTH_4>(pri0, pri1);

  pri0 = bg3_enabled[0] ? 2 : 0;
  pri1 = bg3_enabled[1] ? 5 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<0, BG3, COLORDEPTH_4>(pri0, pri1);

  pri0 = bg4_enabled[0] ? 1 : 0;
  pri1 = bg4_enabled[1] ? 4 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<0, BG4, COLORDEPTH_4>(pri0, pri1);

  pri0 = oam_enabled[0] ?  3 : 0;
  pri1 = oam_enabled[1] ?  6 : 0;
  pri2 = oam_enabled[2] ?  9 : 0;
  pri3 = oam_enabled[3] ? 12 : 0;
  if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
}

void bPPUDebugger::render_line_mode1() {
  unsigned pri0, pri1, pri2, pri3;

  if(regs.bg3_priority) {
    pri0 = bg1_enabled[0] ? 5 : 0;
    pri1 = bg1_enabled[1] ? 8 : 0;
    if(pri0 | pri1) bPPU::render_line_bg<1, BG1, COLORDEPTH_16>(pri0, pri1);

    pri0 = bg2_enabled[0] ? 4 : 0;
    pri1 = bg2_enabled[1] ? 7 : 0;
    if(pri0 | pri1) bPPU::render_line_bg<1, BG2, COLORDEPTH_16>(pri0, pri1);

    pri0 = bg3_enabled[0] ?  1 : 0;
    pri1 = bg3_enabled[1] ? 10 : 0;
    if(pri0 | pri1) bPPU::render_line_bg<1, BG3, COLORDEPTH_4>(pri0, pri1);

    pri0 = oam_enabled[0] ? 2 : 0;
    pri1 = oam_enabled[1] ? 3 : 0;
    pri2 = oam_enabled[2] ? 6 : 0;
    pri3 = oam_enabled[3] ? 9 : 0;
    if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
  } else {
    pri0 = bg1_enabled[0] ? 6 : 0;
    pri1 = bg1_enabled[1] ? 9 : 0;
    if(pri0 | pri1) bPPU::render_line_bg<1, BG1, COLORDEPTH_16>(pri0, pri1);

    pri0 = bg2_enabled[0] ? 5 : 0;
    pri1 = bg2_enabled[1] ? 8 : 0;
    if(pri0 | pri1) bPPU::render_line_bg<1, BG2, COLORDEPTH_16>(pri0, pri1);

    pri0 = bg3_enabled[0] ? 1 : 0;
    pri1 = bg3_enabled[1] ? 3 : 0;
    if(pri0 | pri1) bPPU::render_line_bg<1, BG3, COLORDEPTH_4>(pri0, pri1);

    pri0 = oam_enabled[0] ?  2 : 0;
    pri1 = oam_enabled[1] ?  4 : 0;
    pri2 = oam_enabled[2] ?  7 : 0;
    pri3 = oam_enabled[3] ? 10 : 0;
    bPPU::render_line_oam(pri0, pri1, pri2, pri3);
  }
}

void bPPUDebugger::render_line_mode2() {
  unsigned pri0, pri1, pri2, pri3;

  pri0 = bg1_enabled[0] ? 3 : 0;
  pri1 = bg1_enabled[1] ? 7 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<2, BG1, COLORDEPTH_16>(pri0, pri1);

  pri0 = bg2_enabled[0] ? 1 : 0;
  pri1 = bg2_enabled[1] ? 5 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<2, BG2, COLORDEPTH_16>(pri0, pri1);

  pri0 = oam_enabled[0] ? 2 : 0;
  pri1 = oam_enabled[1] ? 4 : 0;
  pri2 = oam_enabled[2] ? 6 : 0;
  pri3 = oam_enabled[3] ? 8 : 0;
  if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
}

void bPPUDebugger::render_line_mode3() {
  unsigned pri0, pri1, pri2, pri3;

  pri0 = bg1_enabled[0] ? 3 : 0;
  pri1 = bg1_enabled[1] ? 7 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<3, BG1, COLORDEPTH_256>(pri0, pri1);

  pri0 = bg2_enabled[0] ? 1 : 0;
  pri1 = bg2_enabled[1] ? 5 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<3, BG2, COLORDEPTH_16>(pri0, pri1);

  pri0 = oam_enabled[0] ? 2 : 0;
  pri1 = oam_enabled[1] ? 4 : 0;
  pri2 = oam_enabled[2] ? 6 : 0;
  pri3 = oam_enabled[3] ? 8 : 0;
  if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
}

void bPPUDebugger::render_line_mode4() {
  unsigned pri0, pri1, pri2, pri3;

  pri0 = bg1_enabled[0] ? 3 : 0;
  pri1 = bg1_enabled[1] ? 7 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<4, BG1, COLORDEPTH_256>(pri0, pri1);

  pri0 = bg2_enabled[0] ? 1 : 0;
  pri1 = bg2_enabled[1] ? 5 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<4, BG2, COLORDEPTH_4>(pri0, pri1);

  pri0 = oam_enabled[0] ? 2 : 0;
  pri1 = oam_enabled[1] ? 4 : 0;
  pri2 = oam_enabled[2] ? 6 : 0;
  pri3 = oam_enabled[3] ? 8 : 0;
  if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
}

void bPPUDebugger::render_line_mode5() {
  unsigned pri0, pri1, pri2, pri3;

  pri0 = bg1_enabled[0] ? 3 : 0;
  pri1 = bg1_enabled[1] ? 7 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<5, BG1, COLORDEPTH_16>(pri0, pri1);

  pri0 = bg2_enabled[0] ? 1 : 0;
  pri1 = bg2_enabled[1] ? 5 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<5, BG2, COLORDEPTH_4>(pri0, pri1);

  pri0 = oam_enabled[0] ? 2 : 0;
  pri1 = oam_enabled[1] ? 4 : 0;
  pri2 = oam_enabled[2] ? 6 : 0;
  pri3 = oam_enabled[3] ? 8 : 0;
  if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
}

void bPPUDebugger::render_line_mode6() {
  unsigned pri0, pri1, pri2, pri3;

  pri0 = bg1_enabled[0] ? 2 : 0;
  pri1 = bg1_enabled[1] ? 5 : 0;
  if(pri0 | pri1) bPPU::render_line_bg<6, BG1, COLORDEPTH_16>(pri0, pri1);

  pri0 = oam_enabled[0] ? 1 : 0;
  pri1 = oam_enabled[1] ? 3 : 0;
  pri2 = oam_enabled[2] ? 4 : 0;
  pri3 = oam_enabled[3] ? 6 : 0;
  if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
}

void bPPUDebugger::render_line_mode7() {
  unsigned pri0, pri1, pri2, pri3;

  if(regs.mode7_extbg == false) {
    pri0 = bg1_enabled[0] ? 2 : 0;
    pri1 = bg1_enabled[1] ? 2 : 0;
    if(pri0 | pri1) bPPU::render_line_mode7<BG1>(pri0, pri1);

    pri0 = oam_enabled[0] ? 1 : 0;
    pri1 = oam_enabled[1] ? 3 : 0;
    pri2 = oam_enabled[2] ? 4 : 0;
    pri3 = oam_enabled[3] ? 5 : 0;
    if(pri0 | pri1 | pri2 | pri3) bPPU::render_line_oam(pri0, pri1, pri2, pri3);
  } else {
    pri0 = bg1_enabled[0] ? 3 : 0;
    pri1 = bg1_enabled[1] ? 3 : 0;
    if(pri0 | pri1) bPPU::render_line_mode7<BG1>(pri0, pri1);

    pri0 = bg2_enabled[0] ? 1 : 0;
    pri1 = bg2_enabled[1] ? 5 : 0;
    if(pri0 | pri1) bPPU::render_line_mode7<BG2>(pri0, pri1);

    pri0 = oam_enabled[0] ? 2 : 0;
    pri1 = oam_enabled[1] ? 4 : 0;
    pri2 = oam_enabled[2] ? 6 : 0;
    pri3 = oam_enabled[3] ? 7 : 0;
    bPPU::render_line_oam(pri0, pri1, pri2, pri3);
  }
}

#endif
