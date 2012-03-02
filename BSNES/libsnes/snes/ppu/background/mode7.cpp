#ifdef PPU_CPP

signed PPU::Background::clip(signed n) {
  //13-bit sign extend: --s---nnnnnnnnnn -> ssssssnnnnnnnnnn
  return n & 0x2000 ? (n | ~1023) : (n & 1023);
}

//H = 60
void PPU::Background::begin_mode7() {
  cache.hoffset = self.regs.mode7_hoffset;
  cache.voffset = self.regs.mode7_voffset;
}

void PPU::Background::run_mode7() {
  signed a = sclip<16>(self.regs.m7a);
  signed b = sclip<16>(self.regs.m7b);
  signed c = sclip<16>(self.regs.m7c);
  signed d = sclip<16>(self.regs.m7d);

  signed cx = sclip<13>(self.regs.m7x);
  signed cy = sclip<13>(self.regs.m7y);
  signed hoffset = sclip<13>(cache.hoffset);
  signed voffset = sclip<13>(cache.voffset);

  if(Background::x++ & ~255) return;
  unsigned x = mosaic.hoffset;
  unsigned y = self.bg1.mosaic.voffset;  //BG2 vertical mosaic uses BG1 mosaic size

  if(--mosaic.hcounter == 0) {
    mosaic.hcounter = regs.mosaic + 1;
    mosaic.hoffset += regs.mosaic + 1;
  }

  if(self.regs.mode7_hflip) x = 255 - x;
  if(self.regs.mode7_vflip) y = 255 - y;

  signed psx = ((a * clip(hoffset - cx)) & ~63) + ((b * clip(voffset - cy)) & ~63) + ((b * y) & ~63) + (cx << 8);
  signed psy = ((c * clip(hoffset - cx)) & ~63) + ((d * clip(voffset - cy)) & ~63) + ((d * y) & ~63) + (cy << 8);

  signed px = psx + (a * x);
  signed py = psy + (c * x);

  //mask pseudo-FP bits
  px >>= 8;
  py >>= 8;

  unsigned tile;
  unsigned palette;
  switch(self.regs.mode7_repeat) {
    //screen repetition outside of screen area
    case 0:
    case 1: {
      px &= 1023;
      py &= 1023;
      tile = ppu.vram[((py >> 3) * 128 + (px >> 3)) << 1];
      palette = ppu.vram[(((tile << 6) + ((py & 7) << 3) + (px & 7)) << 1) + 1];
      break;
    }

    //palette color 0 outside of screen area
    case 2: {
      if((px | py) & ~1023) {
        palette = 0;
      } else {
        px &= 1023;
        py &= 1023;
        tile = ppu.vram[((py >> 3) * 128 + (px >> 3)) << 1];
        palette = ppu.vram[(((tile << 6) + ((py & 7) << 3) + (px & 7)) << 1) + 1];
      }
      break;
    }

    //character 0 repetition outside of screen area
    case 3: {
      if((px | py) & ~1023) {
        tile = 0;
      } else {
        px &= 1023;
        py &= 1023;
        tile = ppu.vram[((py >> 3) * 128 + (px >> 3)) << 1];
      }
      palette = ppu.vram[(((tile << 6) + ((py & 7) << 3) + (px & 7)) << 1) + 1];
      break;
    }
  }

  unsigned priority;
  if(id == ID::BG1) {
    priority = regs.priority0;
  } else if(id == ID::BG2) {
    priority = (palette & 0x80 ? regs.priority1 : regs.priority0);
    palette &= 0x7f;
  }

  if(palette == 0) return;

  if(regs.main_enable) {
    output.main.palette = palette;
    output.main.priority = priority;
    output.main.tile = 0;
  }

  if(regs.sub_enable) {
    output.sub.palette = palette;
    output.sub.priority = priority;
    output.sub.tile = 0;
  }
}

#endif
