#ifdef PPU_CPP

void PPU::Screen::scanline() {
  output = self.output + self.vcounter() * 1024;
  if(self.display.interlace && self.field()) output += 512;
}

void PPU::Screen::run() {
  if(ppu.vcounter() == 0) return;

  uint32 color;
  if(self.regs.pseudo_hires == false && self.regs.bgmode != 5 && self.regs.bgmode != 6) {
    color = get_pixel(0);
    *output++ = color;
    *output++ = color;
  } else {
    color = get_pixel(1);
    *output++ = color;
    color = get_pixel(0);
    *output++ = color;
  }
}

uint32 PPU::Screen::get_pixel(bool swap) {
  if(ppu.regs.overscan == false && ppu.vcounter() >= 225) return 0x0000;

  enum source_t { BG1, BG2, BG3, BG4, OAM, BACK };
  bool color_enable[] = { regs.bg1_color_enable, regs.bg2_color_enable, regs.bg3_color_enable, regs.bg4_color_enable, regs.oam_color_enable, regs.back_color_enable };

  //===========
  //main screen
  //===========

  unsigned priority_main = 0;
  unsigned color_main;
  unsigned source_main;

  if(self.bg1.output.main.priority) {
    priority_main = self.bg1.output.main.priority;
    if(regs.direct_color && (self.regs.bgmode == 3 || self.regs.bgmode == 4 || self.regs.bgmode == 7)) {
      color_main = get_direct_color(self.bg1.output.main.palette, self.bg1.output.main.tile);
    } else {
      color_main = get_color(self.bg1.output.main.palette);
    }
    source_main = BG1;
  }
  if(self.bg2.output.main.priority > priority_main) {
    priority_main = self.bg2.output.main.priority;
    color_main = get_color(self.bg2.output.main.palette);
    source_main = BG2;
  }
  if(self.bg3.output.main.priority > priority_main) {
    priority_main = self.bg3.output.main.priority;
    color_main = get_color(self.bg3.output.main.palette);
    source_main = BG3;
  }
  if(self.bg4.output.main.priority > priority_main) {
    priority_main = self.bg4.output.main.priority;
    color_main = get_color(self.bg4.output.main.palette);
    source_main = BG4;
  }
  if(self.sprite.output.main.priority > priority_main) {
    priority_main = self.sprite.output.main.priority;
    color_main = get_color(self.sprite.output.main.palette);
    source_main = OAM;
  }
  if(priority_main == 0) {
    color_main = get_color(0);
    source_main = BACK;
  }

  //==========
  //sub screen
  //==========

  unsigned priority_sub = 0;
  unsigned color_sub;
  unsigned source_sub;

  if(self.bg1.output.sub.priority) {
    priority_sub = self.bg1.output.sub.priority;
    if(regs.direct_color && (self.regs.bgmode == 3 || self.regs.bgmode == 4 || self.regs.bgmode == 7)) {
      color_sub = get_direct_color(self.bg1.output.sub.palette, self.bg1.output.sub.tile);
    } else {
      color_sub = get_color(self.bg1.output.sub.palette);
    }
    source_sub = BG1;
  }
  if(self.bg2.output.sub.priority > priority_sub) {
    priority_sub = self.bg2.output.sub.priority;
    color_sub = get_color(self.bg2.output.sub.palette);
    source_sub = BG2;
  }
  if(self.bg3.output.sub.priority > priority_sub) {
    priority_sub = self.bg3.output.sub.priority;
    color_sub = get_color(self.bg3.output.sub.palette);
    source_sub = BG3;
  }
  if(self.bg4.output.sub.priority > priority_sub) {
    priority_sub = self.bg4.output.sub.priority;
    color_sub = get_color(self.bg4.output.sub.palette);
    source_sub = BG4;
  }
  if(self.sprite.output.sub.priority > priority_sub) {
    priority_sub = self.sprite.output.sub.priority;
    color_sub = get_color(self.sprite.output.sub.palette);
    source_sub = OAM;
  }
  if(priority_sub == 0) {
    if(self.regs.pseudo_hires == true || self.regs.bgmode == 5 || self.regs.bgmode == 6) {
      color_sub = get_color(0);
    } else {
      color_sub = (regs.color_b << 10) + (regs.color_g << 5) + (regs.color_r << 0);
    }
    source_sub = BACK;
  }

  if(swap == true) {
    nall::swap(priority_main, priority_sub);
    nall::swap(color_main, color_sub);
    nall::swap(source_main, source_sub);
  }

  uint16 output;
  if(!regs.addsub_mode) {
    source_sub = BACK;
    color_sub = (regs.color_b << 10) + (regs.color_g << 5) + (regs.color_r << 0);
  }

  if(self.window.output.main.color_enable == false) {
    if(self.window.output.sub.color_enable == false) {
      return 0x0000;
    }
    color_main = 0x0000;
  }

  bool color_exempt = (source_main == OAM && self.sprite.output.main.palette < 192);
  if(!color_exempt && color_enable[source_main] && self.window.output.sub.color_enable) {
    bool halve = false;
    if(regs.color_halve && self.window.output.main.color_enable) {
      if(!regs.addsub_mode || source_sub != BACK) halve = true;
    }
    output = addsub(color_main, color_sub, halve);
  } else {
    output = color_main;
  }

  //========
  //lighting
  //========

  if(self.regs.display_disable) return 0;
  return (self.regs.display_brightness << 15) | output;
}

uint16 PPU::Screen::addsub(unsigned x, unsigned y, bool halve) {
  if(!regs.color_mode) {
    if(!halve) {
      unsigned sum = x + y;
      unsigned carry = (sum - ((x ^ y) & 0x0421)) & 0x8420;
      return (sum - carry) | (carry - (carry >> 5));
    } else {
      return (x + y - ((x ^ y) & 0x0421)) >> 1;
    }
  } else {
    unsigned diff = x - y + 0x8420;
    unsigned borrow = (diff - ((x ^ y) & 0x8420)) & 0x8420;
    if(!halve) {
      return   (diff - borrow) & (borrow - (borrow >> 5));
    } else {
      return (((diff - borrow) & (borrow - (borrow >> 5))) & 0x7bde) >> 1;
    }
  }
}

uint16 PPU::Screen::get_color(unsigned palette) {
  palette <<= 1;
  self.regs.cgram_iaddr = palette;
  return ppu.cgram[palette + 0] + (ppu.cgram[palette + 1] << 8);
}

uint16 PPU::Screen::get_direct_color(unsigned palette, unsigned tile) {
  //palette = -------- BBGGGRRR
  //tile    = ---bgr-- --------
  //output  = 0BBb00GG Gg0RRRr0
  return ((palette << 7) & 0x6000) + ((tile >> 0) & 0x1000)
       + ((palette << 4) & 0x0380) + ((tile >> 5) & 0x0040)
       + ((palette << 2) & 0x001c) + ((tile >> 9) & 0x0002);
}

void PPU::Screen::reset() {
  regs.addsub_mode = random(false);
  regs.direct_color = random(false);
  regs.color_mode = random(false);
  regs.color_halve = random(false);
  regs.bg1_color_enable = random(false);
  regs.bg2_color_enable = random(false);
  regs.bg3_color_enable = random(false);
  regs.bg4_color_enable = random(false);
  regs.oam_color_enable = random(false);
  regs.back_color_enable = random(false);
  regs.color_r = random(0);
  regs.color_g = random(0);
  regs.color_b = random(0);
}

PPU::Screen::Screen(PPU &self) : self(self) {
}

#endif
