void PPU::serialize(serializer &s) {
  Thread::serialize(s);

  s.array(vram, 96 * 1024);
  s.array(pram, 512);

  s.integer(regs.control.bgmode);
  s.integer(regs.control.cgbmode);
  s.integer(regs.control.frame);
  s.integer(regs.control.hblank);
  s.integer(regs.control.objmapping);
  s.integer(regs.control.forceblank);
  for(auto &flag : regs.control.enable) s.integer(flag);
  for(auto &flag : regs.control.enablewindow) s.integer(flag);

  s.integer(regs.greenswap);

  s.integer(regs.status.vblank);
  s.integer(regs.status.hblank);
  s.integer(regs.status.vcoincidence);
  s.integer(regs.status.irqvblank);
  s.integer(regs.status.irqhblank);
  s.integer(regs.status.irqvcoincidence);
  s.integer(regs.status.vcompare);

  s.integer(regs.vcounter);

  for(auto &bg : regs.bg) {
    s.integer(bg.control.priority);
    s.integer(bg.control.characterbaseblock);
    s.integer(bg.control.mosaic);
    s.integer(bg.control.colormode);
    s.integer(bg.control.screenbaseblock);
    s.integer(bg.control.affinewrap);
    s.integer(bg.control.screensize);
    s.integer(bg.hoffset);
    s.integer(bg.voffset);
    s.integer(bg.pa);
    s.integer(bg.pb);
    s.integer(bg.pc);
    s.integer(bg.pd);
    s.integer(bg.x);
    s.integer(bg.y);
    s.integer(bg.lx);
    s.integer(bg.ly);
    s.integer(bg.vmosaic);
    s.integer(bg.hmosaic);
    s.integer(bg.id);
  }

  for(auto &window : regs.window) {
    s.integer(window.x1);
    s.integer(window.x2);
    s.integer(window.y1);
    s.integer(window.y2);
  }

  for(auto &windowflags : regs.windowflags) {
    for(auto &flag : windowflags.enable) s.integer(flag);
  }

  s.integer(regs.mosaic.bghsize);
  s.integer(regs.mosaic.bgvsize);
  s.integer(regs.mosaic.objhsize);
  s.integer(regs.mosaic.objvsize);

  for(auto &flag : regs.blend.control.above) s.integer(flag);
  for(auto &flag : regs.blend.control.below) s.integer(flag);
  s.integer(regs.blend.control.mode);
  s.integer(regs.blend.eva);
  s.integer(regs.blend.evb);
  s.integer(regs.blend.evy);

  for(unsigned l = 0; l < 6; l++) {
    for(unsigned p = 0; p < 240; p++) {
      auto &pixel = layer[l][p];
      s.integer(pixel.enable);
      s.integer(pixel.priority);
      s.integer(pixel.color);
      s.integer(pixel.translucent);
      s.integer(pixel.mosaic);
    }
  }

  for(unsigned w = 0; w < 3; w++) {
    for(unsigned p = 0; p < 240; p++) {
      s.integer(windowmask[w][p]);
    }
  }

  for(auto &value : vmosaic) s.integer(value);
  for(auto &value : hmosaic) s.integer(value);

  for(auto &obj : object) {
    s.integer(obj.y);
    s.integer(obj.affine);
    s.integer(obj.affinesize);
    s.integer(obj.mode);
    s.integer(obj.mosaic);
    s.integer(obj.colors);
    s.integer(obj.shape);

    s.integer(obj.x);
    s.integer(obj.affineparam);
    s.integer(obj.hflip);
    s.integer(obj.vflip);
    s.integer(obj.size);

    s.integer(obj.character);
    s.integer(obj.priority);
    s.integer(obj.palette);

    s.integer(obj.width);
    s.integer(obj.height);
  }

  for(auto &par : objectparam) {
    s.integer(par.pa);
    s.integer(par.pb);
    s.integer(par.pc);
    s.integer(par.pd);
  }
}
