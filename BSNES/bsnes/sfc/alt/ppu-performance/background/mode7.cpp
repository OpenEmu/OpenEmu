#ifdef PPU_CPP

#define Clip(x) (((x) & 0x2000) ? ((x) | ~0x03ff) : ((x) & 0x03ff))

void PPU::Background::render_mode7() {
  signed px, py;
  signed tx, ty, tile, palette;

  signed a = sclip<16>(self.regs.m7a);
  signed b = sclip<16>(self.regs.m7b);
  signed c = sclip<16>(self.regs.m7c);
  signed d = sclip<16>(self.regs.m7d);

  signed cx = sclip<13>(self.regs.m7x);
  signed cy = sclip<13>(self.regs.m7y);
  signed hofs = sclip<13>(self.regs.mode7_hoffset);
  signed vofs = sclip<13>(self.regs.mode7_voffset);

  signed y = (self.regs.mode7_vflip == false ? self.vcounter() : 255 - self.vcounter());

  uint16 *mosaic_x, *mosaic_y;
  if(id == ID::BG1) {
    mosaic_x = mosaic_table[self.bg1.regs.mosaic];
    mosaic_y = mosaic_table[self.bg1.regs.mosaic];
  } else {
    mosaic_x = mosaic_table[self.bg2.regs.mosaic];
    mosaic_y = mosaic_table[self.bg1.regs.mosaic];
  }

  unsigned priority0 = (priority0_enable ? regs.priority0 : 0);
  unsigned priority1 = (priority1_enable ? regs.priority1 : 0);
  if(priority0 + priority1 == 0) return;

  signed psx = ((a * Clip(hofs - cx)) & ~63) + ((b * Clip(vofs - cy)) & ~63) + ((b * mosaic_y[y]) & ~63) + (cx << 8);
  signed psy = ((c * Clip(hofs - cx)) & ~63) + ((d * Clip(vofs - cy)) & ~63) + ((d * mosaic_y[y]) & ~63) + (cy << 8);
  for(signed x = 0; x < 256; x++) {
    px = (psx + (a * mosaic_x[x])) >> 8;
    py = (psy + (c * mosaic_x[x])) >> 8;

    switch(self.regs.mode7_repeat) {
      case 0: case 1: {
        px &= 1023;
        py &= 1023;
        tx = ((px >> 3) & 127);
        ty = ((py >> 3) & 127);
        tile = ppu.vram[(ty * 128 + tx) << 1];
        palette = ppu.vram[(((tile << 6) + ((py & 7) << 3) + (px & 7)) << 1) + 1];
        break;
      }

      case 2: {
        if((px | py) & ~1023) {
          palette = 0;
        } else {
          px &= 1023;
          py &= 1023;
          tx = ((px >> 3) & 127);
          ty = ((py >> 3) & 127);
          tile = ppu.vram[(ty * 128 + tx) << 1];
          palette = ppu.vram[(((tile << 6) + ((py & 7) << 3) + (px & 7)) << 1) + 1];
        }
        break;
      }

      case 3: {
        if((px | py) & ~1023) {
          tile = 0;
        } else {
          px &= 1023;
          py &= 1023;
          tx = ((px >> 3) & 127);
          ty = ((py >> 3) & 127);
          tile = ppu.vram[(ty * 128 + tx) << 1];
        }
        palette = ppu.vram[(((tile << 6) + ((py & 7) << 3) + (px & 7)) << 1) + 1];
        break;
      }
    }

    unsigned priority;
    if(id == ID::BG1) {
      priority = priority0;
    } else {
      priority = (palette & 0x80 ? priority1 : priority0);
      palette &= 0x7f;
    }

    if(palette == 0) continue;
    unsigned plot_x = (self.regs.mode7_hflip == false ? x : 255 - x);

    unsigned color;
    if(self.screen.regs.direct_color && id == ID::BG1) {
      color = self.screen.get_direct_color(0, palette);
    } else {
      color = self.screen.get_palette(palette);
    }

    if(regs.main_enable && !window.main[plot_x]) self.screen.output.plot_main(plot_x, color, priority, id);
    if(regs.sub_enable && !window.sub[plot_x]) self.screen.output.plot_sub(plot_x, color, priority, id);
  }
}

#undef Clip

#endif
