#ifdef PPU_CPP

uint8 PPUDebugger::vram_mmio_read(uint16 addr) {
  uint8 data = PPU::vram_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::VRAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void PPUDebugger::vram_mmio_write(uint16 addr, uint8 data) {
  PPU::vram_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::VRAM, Debugger::Breakpoint::Mode::Write, addr, data);
}

uint8 PPUDebugger::oam_mmio_read(uint16 addr) {
  uint8 data = PPU::oam_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::OAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void PPUDebugger::oam_mmio_write(uint16 addr, uint8 data) {
  PPU::oam_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::OAM, Debugger::Breakpoint::Mode::Write, addr, data);
}

uint8 PPUDebugger::cgram_mmio_read(uint16 addr) {
  uint8 data = PPU::cgram_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CGRAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void PPUDebugger::cgram_mmio_write(uint16 addr, uint8 data) {
  PPU::cgram_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CGRAM, Debugger::Breakpoint::Mode::Write, addr, data);
}

PPUDebugger::PPUDebugger() {
}

bool PPUDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  //internal
  item("S-PPU1 MDR", string("0x", hex<2>(regs.ppu1_mdr)));
  item("S-PPU2 MDR", string("0x", hex<2>(regs.ppu2_mdr)));

  //$2100
  item("$2100", "");
  item("Display Disable", regs.display_disabled);
  item("Display Brightness", (unsigned)regs.display_brightness);

  //$2101
  item("$2101", "");
  item("OAM Base Size", (unsigned)regs.oam_basesize);
  item("OAM Name Select", (unsigned)regs.oam_nameselect);
  item("OAM Name Base Address", string("0x", hex<4>(regs.oam_tdaddr)));

  //$2102-$2103
  item("$2102-$2103", "");
  item("OAM Base Address", string("0x", hex<4>(regs.oam_baseaddr)));
  item("OAM Priority", regs.oam_priority);

  //$2105
  item("$2105", "");
  item("BG1 Tile Size", regs.bg_tilesize[BG1] ? "16x16" : "8x8");
  item("BG2 Tile Size", regs.bg_tilesize[BG2] ? "16x16" : "8x8");
  item("BG3 Tile Size", regs.bg_tilesize[BG3] ? "16x16" : "8x8");
  item("BG4 Tile Size", regs.bg_tilesize[BG4] ? "16x16" : "8x8");
  item("BG3 Priority", regs.bg3_priority);
  item("BG Mode", (unsigned)regs.bg_mode);

  //$2106
  item("$2106", "");
  item("Mosaic Size", (unsigned)regs.mosaic_size);
  item("BG1 Mosaic Enable", regs.mosaic_enabled[BG1]);
  item("BG2 Mosaic Enable", regs.mosaic_enabled[BG2]);
  item("BG3 Mosaic Enable", regs.mosaic_enabled[BG3]);
  item("BG4 Mosaic Enable", regs.mosaic_enabled[BG4]);

  static char screen_size[4][8] = { "32x32", "32x64", "64x32", "64x64" };

  //$2107
  item("$2107", "");
  item("BG1 Screen Address", string("0x", hex<4>(regs.bg_scaddr[BG1])));
  item("BG1 Screen Size", screen_size[regs.bg_scsize[BG1]]);

  //$2108
  item("$2108", "");
  item("BG2 Screen Address", string("0x", hex<4>(regs.bg_scaddr[BG2])));
  item("BG2 Screen Size", screen_size[regs.bg_scsize[BG2]]);

  //$2109
  item("$2109", "");
  item("BG3 Screen Address", string("0x", hex<4>(regs.bg_scaddr[BG3])));
  item("BG3 Screen Size", screen_size[regs.bg_scsize[BG3]]);

  //$210a
  item("$210a", "");
  item("BG4 Screen Address", string("0x", hex<4>(regs.bg_scaddr[BG4])));
  item("BG4 Screen Size", screen_size[regs.bg_scsize[BG4]]);

  //$210b
  item("$210b", "");
  item("BG1 Name Base Address", string("0x", hex<4>(regs.bg_tdaddr[BG1])));
  item("BG2 Name Base Address", string("0x", hex<4>(regs.bg_tdaddr[BG2])));

  //$210c
  item("$210c", "");
  item("BG3 Name Base Address", string("0x", hex<4>(regs.bg_tdaddr[BG3])));
  item("BG4 Name Base Address", string("0x", hex<4>(regs.bg_tdaddr[BG4])));

  //$210d
  item("$210d", "");
  item("Mode 7 Scroll H-offset", (unsigned)(regs.m7_hofs & 0x1fff));
  item("BG1 Scroll H-offset", (unsigned)(regs.bg_hofs[BG1] & 0x03ff));

  //$210e
  item("$210e", "");
  item("Mode 7 Scroll V-offset", (unsigned)(regs.m7_vofs & 0x1fff));
  item("BG1 Scroll V-offset", (unsigned)(regs.bg_vofs[BG1] & 0x03ff));

  //$210f
  item("$210f", "");
  item("BG2 Scroll H-offset", (unsigned)(regs.bg_hofs[BG2] & 0x03ff));

  //$2110
  item("$2110", "");
  item("BG2 Scroll V-offset", (unsigned)(regs.bg_vofs[BG2] & 0x03ff));

  //$2111
  item("$2111", "");
  item("BG3 Scroll H-offset", (unsigned)(regs.bg_hofs[BG3] & 0x03ff));

  //$2112
  item("$2112", "");
  item("BG3 Scroll V-offset", (unsigned)(regs.bg_vofs[BG3] & 0x03ff));

  //$2113
  item("$2113", "");
  item("BG4 Scroll H-offset", (unsigned)(regs.bg_hofs[BG4] & 0x03ff));

  //$2114
  item("$2114", "");
  item("BG4 Scroll V-offset", (unsigned)(regs.bg_vofs[BG4] & 0x03ff));

  //$2115
  item("$2115", "");
  item("VRAM Increment Mode", (unsigned)regs.vram_incmode);
  item("VRAM Increment Formation", (unsigned)regs.vram_mapping);
  item("VRAM Increment Size", (unsigned)regs.vram_incsize);

  //$2116-$2117
  item("$2116-$2117", "");
  item("VRAM Address", string("0x", hex<4>(regs.vram_addr)));

  //$211a
  item("$211a", "");
  item("Mode 7 Repeat", (unsigned)regs.mode7_repeat);
  item("Mode 7 V-flip", regs.mode7_vflip);
  item("Mode 7 H-flip", regs.mode7_hflip);

  //$211b
  item("$211b", "");
  item("Mode 7 A", (unsigned)regs.m7a);

  //$211c
  item("$211c", "");
  item("Mode 7 B", (unsigned)regs.m7b);

  //$211d
  item("$211d", "");
  item("Mode 7 C", (unsigned)regs.m7c);

  //$211e
  item("$211e", "");
  item("Mode 7 D", (unsigned)regs.m7d);

  //$211f
  item("$211f", "");
  item("Mode 7 X", (unsigned)regs.m7x);

  //$2120
  item("$2120", "");
  item("Mode 7 Y", (unsigned)regs.m7y);

  //$2121
  item("$2121", "");
  item("CGRAM Address", string("0x", hex<4>(regs.cgram_addr)));

  //$2123
  item("$2123", "");
  item("BG1 Window 1 Enable", regs.window1_enabled[BG1]);
  item("BG1 Window 1 Invert", regs.window1_invert [BG1]);
  item("BG1 Window 2 Enable", regs.window2_enabled[BG1]);
  item("BG1 Window 2 Invert", regs.window2_invert [BG1]);
  item("BG2 Window 1 Enable", regs.window1_enabled[BG2]);
  item("BG2 Window 1 Invert", regs.window1_invert [BG2]);
  item("BG2 Window 2 Enable", regs.window2_enabled[BG2]);
  item("BG2 Window 2 Invert", regs.window2_invert [BG2]);

  //$2124
  item("$2124", "");
  item("BG3 Window 1 Enable", regs.window1_enabled[BG3]);
  item("BG3 Window 1 Invert", regs.window1_invert [BG3]);
  item("BG3 Window 2 Enable", regs.window2_enabled[BG3]);
  item("BG3 Window 2 Invert", regs.window2_invert [BG3]);
  item("BG4 Window 1 Enable", regs.window1_enabled[BG4]);
  item("BG4 Window 1 Invert", regs.window1_invert [BG4]);
  item("BG4 Window 2 Enable", regs.window2_enabled[BG4]);
  item("BG4 Window 2 Invert", regs.window2_invert [BG4]);

  //$2125
  item("$2125", "");
  item("OAM Window 1 Enable", regs.window1_enabled[OAM]);
  item("OAM Window 1 Invert", regs.window1_invert [OAM]);
  item("OAM Window 2 Enable", regs.window2_enabled[OAM]);
  item("OAM Window 2 Invert", regs.window2_invert [OAM]);
  item("Color Window 1 Enable", regs.window1_enabled[COL]);
  item("Color Window 1 Invert", regs.window1_invert [COL]);
  item("Color Window 2 Enable", regs.window2_enabled[COL]);
  item("Color Window 2 Invert", regs.window2_enabled[COL]);

  //$2126
  item("$2126", "");
  item("Window 1 Left", (unsigned)regs.window1_left);

  //$2127
  item("$2127", "");
  item("Window 1 Right", (unsigned)regs.window1_right);

  //$2128
  item("$2128", "");
  item("Window 2 Left", (unsigned)regs.window2_left);

  //$2129
  item("$2129", "");
  item("Window 2 Right", (unsigned)regs.window2_right);

  static char window_mask_mode[4][8] = { "OR", "AND", "XOR", "XNOR" };

  //$212a
  item("$212a", "");
  item("BG1 Window Mask", window_mask_mode[regs.window_mask[BG1]]);
  item("BG2 Window Mask", window_mask_mode[regs.window_mask[BG2]]);
  item("BG3 Window Mask", window_mask_mode[regs.window_mask[BG3]]);
  item("BG4 Window Mask", window_mask_mode[regs.window_mask[BG4]]);

  //$212b
  item("$212b", "");
  item("OAM Window Mask", window_mask_mode[regs.window_mask[OAM]]);
  item("Color Window Mask", window_mask_mode[regs.window_mask[COL]]);

  //$212c
  item("$212c", "");
  item("BG1 Mainscreen Enable", regs.bg_enabled[BG1]);
  item("BG2 Mainscreen Enable", regs.bg_enabled[BG2]);
  item("BG3 Mainscreen Enable", regs.bg_enabled[BG3]);
  item("BG4 Mainscreen Enable", regs.bg_enabled[BG4]);
  item("OAM Mainscreen Enable", regs.bg_enabled[OAM]);

  //$212d
  item("$212d", "");
  item("BG1 Subscreen Enable", regs.bgsub_enabled[BG1]);
  item("BG2 Subscreen Enable", regs.bgsub_enabled[BG2]);
  item("BG3 Subscreen Enable", regs.bgsub_enabled[BG3]);
  item("BG4 Subscreen Enable", regs.bgsub_enabled[BG4]);
  item("OAM Subscreen Enable", regs.bgsub_enabled[OAM]);

  //$212e
  item("$212e", "");
  item("BG1 Mainscreen Window Enable", regs.window_enabled[BG1]);
  item("BG2 Mainscreen Window Enable", regs.window_enabled[BG2]);
  item("BG3 Mainscreen Window Enable", regs.window_enabled[BG3]);
  item("BG4 Mainscreen Window Enable", regs.window_enabled[BG4]);
  item("OAM Mainscreen Window Enable", regs.window_enabled[OAM]);

  //$212f
  item("$212f", "");
  item("BG1 Subscreen Window Enable", regs.sub_window_enabled[BG1]);
  item("BG2 Subscreen Window Enable", regs.sub_window_enabled[BG2]);
  item("BG3 Subscreen Window Enable", regs.sub_window_enabled[BG3]);
  item("BG4 Subscreen Window Enable", regs.sub_window_enabled[BG4]);
  item("OAM Subscreen Window Enable", regs.sub_window_enabled[OAM]);

  static char color_window_mask_mode[4][32] = { "Always", "Never", "Inside Window Only", "Outside Window Only" };

  //$2130
  item("$2130", "");
  item("Color Mainscreen Window Mask", color_window_mask_mode[regs.color_mask]);
  item("Color Subscreen Window Mask", color_window_mask_mode[regs.colorsub_mask]);
  item("Color Add/Subtract Mode", !regs.addsub_mode ? "Fixed Color" : "Subscreen");
  item("Direct Color", regs.direct_color);

  //$2131
  item("$2131", "");
  item("Color Mode", !regs.color_mode ? "Add" : "Subtract");
  item("Color Halve", regs.color_halve);
  item("BG1 Color Enable", regs.color_enabled[BG1]);
  item("BG2 Color Enable", regs.color_enabled[BG2]);
  item("BG3 Color Enable", regs.color_enabled[BG3]);
  item("BG4 Color Enable", regs.color_enabled[BG4]);
  item("OAM Color Enable", regs.color_enabled[OAM]);
  item("Back Color Enable", regs.color_enabled[BACK]);

  //$2132
  item("$2132", "");
  item("Color Constant - Blue", (unsigned)regs.color_b);
  item("Color Constant - Green", (unsigned)regs.color_g);
  item("Color Constant - Red", (unsigned)regs.color_r);

  //$2133
  item("$2133", "");
  item("Mode 7 EXTBG", regs.mode7_extbg);
  item("Pseudo Hires", regs.pseudo_hires);
  item("Overscan", regs.overscan);
  item("OAM Interlace", regs.oam_interlace);
  item("Interlace", regs.interlace);

  //$213c
  item("$213c", "");
  item("H-counter", (unsigned)hcounter());

  //$213d
  item("$213d", "");
  item("V-counter", (unsigned)vcounter());

  //$213e
  item("$213e", "");
  item("Range Over", regs.range_over);
  item("Time Over", regs.time_over);
  item("S-PPU1 Version", (unsigned)ppu1_version);

  //$213f
  item("$213f", "");
  item("Field", cpu.field());
  item("Region", !region ? "NTSC" : "PAL");
  item("S-PPU2 Version", (unsigned)ppu2_version);

  #undef item
  return false;
}

#endif
