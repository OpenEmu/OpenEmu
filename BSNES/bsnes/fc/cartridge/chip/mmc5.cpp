struct MMC5 : Chip {

enum class Revision : unsigned {
  MMC5,
  MMC5B,
} revision;

uint8 exram[1024];

//programmable registers

uint2 prg_mode;  //$5100
uint2 chr_mode;  //$5101

uint2 prgram_write_protect[2];  //$5102,$5103

uint2 exram_mode;         //$5104
uint2 nametable_mode[4];  //$5105
uint8 fillmode_tile;      //$5106
uint8 fillmode_color;     //$5107

bool ram_select;            //$5113
uint2 ram_bank;             //$5113
uint8 prg_bank[4];          //$5114-5117
uint10 chr_sprite_bank[8];  //$5120-5127
uint10 chr_bg_bank[4];      //$5128-512b
uint2 chr_bank_hi;          //$5130

bool vs_enable;      //$5200
bool vs_side;        //$5200
uint5 vs_tile;       //$5200
uint8 vs_scroll;     //$5201
uint8 vs_bank;       //$5202

uint8 irq_line;      //$5203
bool irq_enable;     //$5204

uint8 multiplicand;  //$5205
uint8 multiplier;    //$5206

//status registers

unsigned cpu_cycle_counter;
unsigned irq_counter;
bool irq_pending;
bool in_frame;

unsigned vcounter;
unsigned hcounter;
uint16 chr_access[4];
bool chr_active;
bool sprite_8x16;

uint8 exbank;
uint8 exattr;

bool vs_fetch;
uint8 vs_vpos;
uint8 vs_hpos;

void main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    //scanline() resets this; if no scanlines detected, enter video blanking period
    if(++cpu_cycle_counter >= 200) blank();  //113-114 normal; ~2500 across Vblank period

    cpu.set_irq_line(irq_enable && irq_pending);
    tick();
  }
}

void scanline(unsigned y) {
//used for testing only, to verify MMC5 scanline detection is accurate:
//if(y != vcounter && y <= 240) print(y, " vs ", vcounter, "\n");
}

uint8 prg_access(bool write, unsigned addr, uint8 data = 0x00) {
  unsigned bank;

  if((addr & 0xe000) == 0x6000) {
    bank = (ram_select << 2) | ram_bank;
    addr &= 0x1fff;
  } else if(prg_mode == 0) {
    bank = prg_bank[3] & ~3;
    addr &= 0x7fff;
  } else if(prg_mode == 1) {
    if((addr & 0xc000) == 0x8000) bank = (prg_bank[1] & ~1);
    if((addr & 0xe000) == 0xc000) bank = (prg_bank[3] & ~1);
    addr &= 0x3fff;
  } else if(prg_mode == 2) {
    if((addr & 0xe000) == 0x8000) bank = (prg_bank[1] & ~1) | 0;
    if((addr & 0xe000) == 0xa000) bank = (prg_bank[1] & ~1) | 1;
    if((addr & 0xe000) == 0xc000) bank = (prg_bank[2]);
    if((addr & 0xe000) == 0xe000) bank = (prg_bank[3]);
    addr &= 0x1fff;
  } else if(prg_mode == 3) {
    if((addr & 0xe000) == 0x8000) bank = prg_bank[0];
    if((addr & 0xe000) == 0xa000) bank = prg_bank[1];
    if((addr & 0xe000) == 0xc000) bank = prg_bank[2];
    if((addr & 0xe000) == 0xe000) bank = prg_bank[3];
    addr &= 0x1fff;
  }

  bool rom = bank & 0x80;
  bank &= 0x7f;

  if(write == false) {
    if(rom) {
      return board.prgrom.read((bank << 13) | addr);
    } else {
      return board.prgram.read((bank << 13) | addr);
    }
  } else {
    if(rom) {
      board.prgrom.write((bank << 13) | addr, data);
    } else {
      if(prgram_write_protect[0] == 2 && prgram_write_protect[1] == 1) {
        board.prgram.write((bank << 13) | addr, data);
      }
    }
    return 0x00;
  }
}

uint8 prg_read(unsigned addr) {
  if((addr & 0xfc00) == 0x5c00) {
    if(exram_mode >= 2) return exram[addr & 0x03ff];
    return cpu.mdr();
  }

  if(addr >= 0x6000) {
    return prg_access(0, addr);
  }

  switch(addr) {
  case 0x5204: {
    uint8 result = (irq_pending << 7) | (in_frame << 6);
    irq_pending = false;
    return result;
  }
  case 0x5205: return (multiplier * multiplicand) >> 0;
  case 0x5206: return (multiplier * multiplicand) >> 8;
  }
}

void prg_write(unsigned addr, uint8 data) {
  if((addr & 0xfc00) == 0x5c00) {
    //writes 0x00 *during* Vblank (not during screen rendering ...)
    if(exram_mode == 0 || exram_mode == 1) exram[addr & 0x03ff] = in_frame ? data : 0x00;
    if(exram_mode == 2) exram[addr & 0x03ff] = data;
    return;
  }

  if(addr >= 0x6000) {
    prg_access(1, addr, data);
    return;
  }

  switch(addr) {
  case 0x2000:
    sprite_8x16 = data & 0x20;
    break;

  case 0x2001:
    //if BG+sprites are disabled; enter video blanking period
    if((data & 0x18) == 0) blank();
    break;

  case 0x5100: prg_mode = data & 3; break;
  case 0x5101: chr_mode = data & 3; break;

  case 0x5102: prgram_write_protect[0] = data & 3; break;
  case 0x5103: prgram_write_protect[1] = data & 3; break;

  case 0x5104:
    exram_mode = data & 3;
    break;

  case 0x5105:
    nametable_mode[0] = (data & 0x03) >> 0;
    nametable_mode[1] = (data & 0x0c) >> 2;
    nametable_mode[2] = (data & 0x30) >> 4;
    nametable_mode[3] = (data & 0xc0) >> 6;
    break;

  case 0x5106:
    fillmode_tile = data;
    break;

  case 0x5107:
    fillmode_color = data & 3;
    fillmode_color |= fillmode_color << 2;
    fillmode_color |= fillmode_color << 4;
    break;

  case 0x5113:
    ram_select = data & 0x04;
    ram_bank = data & 0x03;
    break;

  case 0x5114: prg_bank[0] = data; break;
  case 0x5115: prg_bank[1] = data; break;
  case 0x5116: prg_bank[2] = data; break;
  case 0x5117: prg_bank[3] = data | 0x80; break;

  case 0x5120: chr_sprite_bank[0] = (chr_bank_hi << 8) | data; chr_active = 0; break;
  case 0x5121: chr_sprite_bank[1] = (chr_bank_hi << 8) | data; chr_active = 0; break;
  case 0x5122: chr_sprite_bank[2] = (chr_bank_hi << 8) | data; chr_active = 0; break;
  case 0x5123: chr_sprite_bank[3] = (chr_bank_hi << 8) | data; chr_active = 0; break;
  case 0x5124: chr_sprite_bank[4] = (chr_bank_hi << 8) | data; chr_active = 0; break;
  case 0x5125: chr_sprite_bank[5] = (chr_bank_hi << 8) | data; chr_active = 0; break;
  case 0x5126: chr_sprite_bank[6] = (chr_bank_hi << 8) | data; chr_active = 0; break;
  case 0x5127: chr_sprite_bank[7] = (chr_bank_hi << 8) | data; chr_active = 0; break;

  case 0x5128: chr_bg_bank[0] = (chr_bank_hi << 8) | data; chr_active = 1; break;
  case 0x5129: chr_bg_bank[1] = (chr_bank_hi << 8) | data; chr_active = 1; break;
  case 0x512a: chr_bg_bank[2] = (chr_bank_hi << 8) | data; chr_active = 1; break;
  case 0x512b: chr_bg_bank[3] = (chr_bank_hi << 8) | data; chr_active = 1; break;

  case 0x5130:
    chr_bank_hi = data & 3;
    break;

  case 0x5200:
    vs_enable = data & 0x80;
    vs_side = data & 0x40;
    vs_tile = data & 0x1f;
    break;

  case 0x5201:
    vs_scroll = data;
    break;

  case 0x5202:
    vs_bank = data;
    break;

  case 0x5203:
    irq_line = data;
    break;

  case 0x5204:
    irq_enable = data & 0x80;
    break;

  case 0x5205:
    multiplicand = data;
    break;

  case 0x5206:
    multiplier = data;
    break;
  }
}

unsigned chr_sprite_addr(unsigned addr) {
  if(chr_mode == 0) {
    auto bank = chr_sprite_bank[7];
    return (bank * 0x2000) + (addr & 0x1fff);
  }

  if(chr_mode == 1) {
    auto bank = chr_sprite_bank[(addr / 0x1000) * 4 + 3];
    return (bank * 0x1000) + (addr & 0x0fff);
  }

  if(chr_mode == 2) {
    auto bank = chr_sprite_bank[(addr / 0x0800) * 2 + 1];
    return (bank * 0x0800) + (addr & 0x07ff);
  }

  if(chr_mode == 3) {
    auto bank = chr_sprite_bank[(addr / 0x0400)];
    return (bank * 0x0400) + (addr & 0x03ff);
  }
}

unsigned chr_bg_addr(unsigned addr) {
  addr &= 0x0fff;

  if(chr_mode == 0) {
    auto bank = chr_bg_bank[3];
    return (bank * 0x2000) + (addr & 0x0fff);
  }

  if(chr_mode == 1) {
    auto bank = chr_bg_bank[3];
    return (bank * 0x1000) + (addr & 0x0fff);
  }

  if(chr_mode == 2) {
    auto bank = chr_bg_bank[(addr / 0x0800) * 2 + 1];
    return (bank * 0x0800) + (addr & 0x07ff);
  }

  if(chr_mode == 3) {
    auto bank = chr_bg_bank[(addr / 0x0400)];
    return (bank * 0x0400) + (addr & 0x03ff);
  }
}

unsigned chr_vs_addr(unsigned addr) {
  return (vs_bank * 0x1000) + (addr & 0x0ff8) + (vs_vpos & 7);
}

void blank() {
  in_frame = false;
}

void scanline() {
  hcounter = 0;

  if(in_frame == false) {
    in_frame = true;
    irq_pending = false;
    vcounter = 0;
  } else {
    if(vcounter == irq_line) irq_pending = true;
    vcounter++;
  }

  cpu_cycle_counter = 0;
}

uint8 ciram_read(unsigned addr) {
  if(vs_fetch && (hcounter & 2) == 0) return exram[vs_vpos / 8 * 32 + vs_hpos / 8];
  if(vs_fetch && (hcounter & 2) != 0) return exram[vs_vpos / 32 * 8 + vs_hpos / 32 + 0x03c0];

  switch(nametable_mode[(addr >> 10) & 3]) {
  case 0: return ppu.ciram_read(0x0000 | (addr & 0x03ff));
  case 1: return ppu.ciram_read(0x0400 | (addr & 0x03ff));
  case 2: return exram_mode < 2 ? exram[addr & 0x03ff] : 0x00;
  case 3: return (hcounter & 2) == 0 ? fillmode_tile : fillmode_color;
  }
}

uint8 chr_read(unsigned addr) {
  chr_access[0] = chr_access[1];
  chr_access[1] = chr_access[2];
  chr_access[2] = chr_access[3];
  chr_access[3] = addr;

  //detect two unused nametable fetches at end of each scanline
  if((chr_access[0] & 0x2000) == 0
  && (chr_access[1] & 0x2000)
  && (chr_access[2] & 0x2000)
  && (chr_access[3] & 0x2000)) scanline();

  if(in_frame == false) {
    vs_fetch = false;
    if(addr & 0x2000) return ciram_read(addr);
    return board.chrrom.read(chr_active ? chr_bg_addr(addr) : chr_sprite_addr(addr));
  }

  bool bg_fetch = (hcounter < 256 || hcounter >= 320);
  uint8 result = 0x00;

  if((hcounter & 7) == 0) {
    vs_hpos  = hcounter >= 320 ? hcounter - 320 : hcounter + 16;
    vs_vpos  = vcounter + vs_scroll;
    vs_fetch = vs_enable && bg_fetch && exram_mode < 2
    && (vs_side ? vs_hpos / 8 >= vs_tile : vs_hpos / 8 < vs_tile);
    if(vs_vpos >= 240) vs_vpos -= 240;

    result = ciram_read(addr);

    exbank = (chr_bank_hi << 6) | (exram[addr & 0x03ff] & 0x3f);
    exattr = exram[addr & 0x03ff] >> 6;
    exattr |= exattr << 2;
    exattr |= exattr << 4;
  } else if((hcounter & 7) == 2) {
    result = ciram_read(addr);
    if(bg_fetch && exram_mode == 1) result = exattr;
  } else {
    if(vs_fetch) result = board.chrrom.read(chr_vs_addr(addr));
    else if(sprite_8x16 ? bg_fetch : chr_active) result = board.chrrom.read(chr_bg_addr(addr));
    else result = board.chrrom.read(chr_sprite_addr(addr));
    if(bg_fetch && exram_mode == 1) result = board.chrrom.read(exbank * 0x1000 + (addr & 0x0fff));
  }

  hcounter += 2;
  return result;
}

void chr_write(unsigned addr, uint8 data) {
  if(addr & 0x2000) {
    switch(nametable_mode[(addr >> 10) & 3]) {
    case 0: return ppu.ciram_write(0x0000 | (addr & 0x03ff), data);
    case 1: return ppu.ciram_write(0x0400 | (addr & 0x03ff), data);
    case 2: exram[addr & 0x03ff] = data; break;
    }
  }
}

void power() {
}

void reset() {
  for(auto &n : exram) n = 0xff;

  prg_mode = 3;
  chr_mode = 0;
  for(auto &n : prgram_write_protect) n = 0;
  exram_mode = 0;
  for(auto &n : nametable_mode) n = 0;
  fillmode_tile = 0;
  fillmode_color = 0;
  ram_select = 0;
  ram_bank = 0;
  prg_bank[0] = 0x00;
  prg_bank[1] = 0x00;
  prg_bank[2] = 0x00;
  prg_bank[3] = 0xff;
  for(auto &n : chr_sprite_bank) n = 0;
  for(auto &n : chr_bg_bank) n = 0;
  chr_bank_hi = 0;
  vs_enable = 0;
  vs_side = 0;
  vs_tile = 0;
  vs_scroll = 0;
  vs_bank = 0;
  irq_line = 0;
  irq_enable = 0;
  multiplicand = 0;
  multiplier = 0;

  cpu_cycle_counter = 0;
  irq_counter = 0;
  irq_pending = 0;
  in_frame = 0;
  vcounter = 0;
  hcounter = 0;
  for(auto &n : chr_access) n = 0;
  chr_active = 0;
  sprite_8x16 = 0;

  exbank = 0;
  exattr = 0;

  vs_fetch = 0;
  vs_vpos = 0;
  vs_hpos = 0;
}

void serialize(serializer &s) {
  s.array(exram);

  s.integer(prg_mode);
  s.integer(chr_mode);
  for(auto &n : prgram_write_protect) s.integer(n);
  s.integer(exram_mode);
  for(auto &n : nametable_mode) s.integer(n);
  s.integer(fillmode_tile);
  s.integer(fillmode_color);
  s.integer(ram_select);
  s.integer(ram_bank);
  for(auto &n : prg_bank) s.integer(n);
  for(auto &n : chr_sprite_bank) s.integer(n);
  for(auto &n : chr_bg_bank) s.integer(n);
  s.integer(chr_bank_hi);
  s.integer(vs_enable);
  s.integer(vs_side);
  s.integer(vs_tile);
  s.integer(vs_scroll);
  s.integer(vs_bank);
  s.integer(irq_line);
  s.integer(irq_enable);
  s.integer(multiplicand);
  s.integer(multiplier);

  s.integer(cpu_cycle_counter);
  s.integer(irq_counter);
  s.integer(irq_pending);
  s.integer(in_frame);

  s.integer(vcounter);
  s.integer(hcounter);
  for(auto &n : chr_access) s.integer(n);
  s.integer(chr_active);
  s.integer(sprite_8x16);

  s.integer(exbank);
  s.integer(exattr);

  s.integer(vs_fetch);
  s.integer(vs_vpos);
  s.integer(vs_hpos);
}

MMC5(Board &board) : Chip(board) {
  revision = Revision::MMC5;
}

};
