void PPU::render_forceblank() {
  uint32 *line = output + regs.vcounter * 240;
  uint16 *last = blur + regs.vcounter * 240;
  for(unsigned x = 0; x < 240; x++) {
    line[x] = video.palette[(0x7fff + last[x] - ((0x7fff ^ last[x]) & 0x0421)) >> 1];
    last[x] = 0x7fff;
  }
}

void PPU::render_screen() {
  uint32 *line = output + regs.vcounter * 240;
  uint16 *last = blur + regs.vcounter * 240;

  if(regs.bg[0].control.mosaic) render_mosaic_background(BG0);
  if(regs.bg[1].control.mosaic) render_mosaic_background(BG1);
  if(regs.bg[2].control.mosaic) render_mosaic_background(BG2);
  if(regs.bg[3].control.mosaic) render_mosaic_background(BG3);
  render_mosaic_object();

  for(unsigned x = 0; x < 240; x++) {
    Registers::WindowFlags flags;
    flags = ~0;  //enable all layers if no windows are enabled

    //determine active window
    if(regs.control.enablewindow[In0] || regs.control.enablewindow[In1] || regs.control.enablewindow[Obj]) {
      flags = (uint8)regs.windowflags[Out];
      if(regs.control.enablewindow[Obj] && windowmask[Obj][x]) flags = (uint8)regs.windowflags[Obj];
      if(regs.control.enablewindow[In1] && windowmask[In1][x]) flags = (uint8)regs.windowflags[In1];
      if(regs.control.enablewindow[In0] && windowmask[In0][x]) flags = (uint8)regs.windowflags[In0];
    }

    //priority sorting: find topmost two pixels
    unsigned a = 5, b = 5;
    for(signed p = 3; p >= 0; p--) {
      for(signed l = 5; l >= 0; l--) {
        if(layer[l][x].enable && layer[l][x].priority == p && flags.enable[l]) {
          b = a;
          a = l;
        }
      }
    }

    auto &above = layer[a];
    auto &below = layer[b];
    bool blendabove = regs.blend.control.above[a];
    bool blendbelow = regs.blend.control.below[b];
    unsigned color = above[x].color;

    //perform blending, if needed
    if(flags.enable[SFX] == false) {
    } else if(above[x].translucent && blendbelow) {
      color = blend(above[x].color, regs.blend.eva, below[x].color, regs.blend.evb);
    } else if(regs.blend.control.mode == 1 && blendabove && blendbelow) {
      color = blend(above[x].color, regs.blend.eva, below[x].color, regs.blend.evb);
    } else if(regs.blend.control.mode == 2 && blendabove) {
      color = blend(above[x].color, 16 - regs.blend.evy, 0x7fff, regs.blend.evy);
    } else if(regs.blend.control.mode == 3 && blendabove) {
      color = blend(above[x].color, 16 - regs.blend.evy, 0x0000, regs.blend.evy);
    }

    //output pixel; blend with previous pixel to simulate GBA LCD blur
    line[x] = video.palette[(color + last[x] - ((color ^ last[x]) & 0x0421)) >> 1];
    last[x] = color;
  }
}

void PPU::render_window(unsigned w) {
  unsigned y = regs.vcounter;

  unsigned y1 = regs.window[w].y1, y2 = regs.window[w].y2;
  unsigned x1 = regs.window[w].x1, x2 = regs.window[w].x2;

  if(y2 < y1 || y2 > 160) y2 = 160;
  if(x2 < x1 || x2 > 240) x2 = 240;

  if(y >= y1 && y < y2) {
    for(unsigned x = x1; x < x2; x++) {
      windowmask[w][x] = true;
    }
  }
}

unsigned PPU::blend(unsigned above, unsigned eva, unsigned below, unsigned evb) {
  uint5 ar = above >> 0, ag = above >> 5, ab = above >> 10;
  uint5 br = below >> 0, bg = below >> 5, bb = below >> 10;

  unsigned r = (ar * eva + br * evb) >> 4;
  unsigned g = (ag * eva + bg * evb) >> 4;
  unsigned b = (ab * eva + bb * evb) >> 4;

  return min(31, r) << 0 | min(31, g) << 5 | min(31, b) << 10;
}
