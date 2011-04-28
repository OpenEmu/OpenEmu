#ifdef BPPU_CPP

#include "render.cpp"

uint8 bPPUDebugger::vram_mmio_read(uint16 addr) {
  uint8 data = bPPU::vram_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::VRAM, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void bPPUDebugger::vram_mmio_write(uint16 addr, uint8 data) {
  bPPU::vram_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::VRAM, Debugger::Breakpoint::Write, addr, data);
}

uint8 bPPUDebugger::oam_mmio_read(uint16 addr) {
  uint8 data = bPPU::oam_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::OAM, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void bPPUDebugger::oam_mmio_write(uint16 addr, uint8 data) {
  bPPU::oam_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::OAM, Debugger::Breakpoint::Write, addr, data);
}

uint8 bPPUDebugger::cgram_mmio_read(uint16 addr) {
  uint8 data = bPPU::cgram_mmio_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::CGRAM, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void bPPUDebugger::cgram_mmio_write(uint16 addr, uint8 data) {
  bPPU::cgram_mmio_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::CGRAM, Debugger::Breakpoint::Write, addr, data);
}

bPPUDebugger::bPPUDebugger() {
  bg1_enabled[0] = bg1_enabled[1] = true;
  bg2_enabled[0] = bg2_enabled[1] = true;
  bg3_enabled[0] = bg3_enabled[1] = true;
  bg4_enabled[0] = bg4_enabled[1] = true;
  oam_enabled[0] = oam_enabled[1] = oam_enabled[2] = oam_enabled[3] = true;
}

//===========
//PPUDebugger
//===========

//internal
unsigned bPPUDebugger::ppu1_mdr() { return regs.ppu1_mdr; }
unsigned bPPUDebugger::ppu2_mdr() { return regs.ppu2_mdr; }

//$2100
bool bPPUDebugger::display_disable() { return regs.display_disabled; }
unsigned bPPUDebugger::display_brightness() { return regs.display_brightness; }

//$2101
unsigned bPPUDebugger::oam_base_size() { return regs.oam_basesize; }
unsigned bPPUDebugger::oam_name_select() { return regs.oam_nameselect; }
unsigned bPPUDebugger::oam_name_base_address() { return regs.oam_tdaddr; }

//$2102-$2103
unsigned bPPUDebugger::oam_base_address() { return regs.oam_baseaddr; }
bool bPPUDebugger::oam_priority() { return regs.oam_priority; }

//$2105
bool bPPUDebugger::bg1_tile_size() { return regs.bg_tilesize[BG1]; }
bool bPPUDebugger::bg2_tile_size() { return regs.bg_tilesize[BG2]; }
bool bPPUDebugger::bg3_tile_size() { return regs.bg_tilesize[BG3]; }
bool bPPUDebugger::bg4_tile_size() { return regs.bg_tilesize[BG4]; }
bool bPPUDebugger::bg3_priority() { return regs.bg3_priority; }
unsigned bPPUDebugger::bg_mode() { return regs.bg_mode; }

//$2106
unsigned bPPUDebugger::mosaic_size() { return regs.mosaic_size; }
bool bPPUDebugger::bg1_mosaic_enable() { return regs.mosaic_enabled[BG1]; }
bool bPPUDebugger::bg2_mosaic_enable() { return regs.mosaic_enabled[BG2]; }
bool bPPUDebugger::bg3_mosaic_enable() { return regs.mosaic_enabled[BG3]; }
bool bPPUDebugger::bg4_mosaic_enable() { return regs.mosaic_enabled[BG4]; }

//$2107
unsigned bPPUDebugger::bg1_screen_address() { return regs.bg_scaddr[BG1]; }
unsigned bPPUDebugger::bg1_screen_size() { return regs.bg_scsize[BG1]; }

//$2108
unsigned bPPUDebugger::bg2_screen_address() { return regs.bg_scaddr[BG2]; }
unsigned bPPUDebugger::bg2_screen_size() { return regs.bg_scsize[BG2]; }

//$2109
unsigned bPPUDebugger::bg3_screen_address() { return regs.bg_scaddr[BG3]; }
unsigned bPPUDebugger::bg3_screen_size() { return regs.bg_scsize[BG3]; }

//$210a
unsigned bPPUDebugger::bg4_screen_address() { return regs.bg_scaddr[BG4]; }
unsigned bPPUDebugger::bg4_screen_size() { return regs.bg_scsize[BG4]; }

//$210b
unsigned bPPUDebugger::bg1_name_base_address() { return regs.bg_tdaddr[BG1]; }
unsigned bPPUDebugger::bg2_name_base_address() { return regs.bg_tdaddr[BG2]; }

//$210c
unsigned bPPUDebugger::bg3_name_base_address() { return regs.bg_tdaddr[BG3]; }
unsigned bPPUDebugger::bg4_name_base_address() { return regs.bg_tdaddr[BG4]; }

//$210d
unsigned bPPUDebugger::mode7_hoffset() { return regs.m7_hofs & 0x1fff; }
unsigned bPPUDebugger::bg1_hoffset() { return regs.bg_hofs[BG1] & 0x03ff; }

//$210e
unsigned bPPUDebugger::mode7_voffset() { return regs.m7_vofs & 0x1fff; }
unsigned bPPUDebugger::bg1_voffset() { return regs.bg_vofs[BG1] & 0x03ff; }

//$210f
unsigned bPPUDebugger::bg2_hoffset() { return regs.bg_hofs[BG2] & 0x03ff; }

//$2110
unsigned bPPUDebugger::bg2_voffset() { return regs.bg_vofs[BG2] & 0x03ff; }

//$2111
unsigned bPPUDebugger::bg3_hoffset() { return regs.bg_hofs[BG3] & 0x03ff; }

//$2112
unsigned bPPUDebugger::bg3_voffset() { return regs.bg_vofs[BG3] & 0x03ff; }

//$2113
unsigned bPPUDebugger::bg4_hoffset() { return regs.bg_hofs[BG4] & 0x03ff; }

//$2114
unsigned bPPUDebugger::bg4_voffset() { return regs.bg_vofs[BG4] & 0x03ff; }

//$2115
bool bPPUDebugger::vram_increment_mode() { return regs.vram_incmode; }
unsigned bPPUDebugger::vram_increment_formation() { return regs.vram_mapping; }
unsigned bPPUDebugger::vram_increment_size() { return regs.vram_incsize; }

//$2116-$2117
unsigned bPPUDebugger::vram_address() { return regs.vram_addr; }

//$211a
unsigned bPPUDebugger::mode7_repeat() { return regs.mode7_repeat; }
bool bPPUDebugger::mode7_vflip() { return regs.mode7_vflip; }
bool bPPUDebugger::mode7_hflip() { return regs.mode7_hflip; }

//$211b
unsigned bPPUDebugger::mode7_a() { return regs.m7a; }

//$211c
unsigned bPPUDebugger::mode7_b() { return regs.m7b; }

//$211d
unsigned bPPUDebugger::mode7_c() { return regs.m7c; }

//$211e
unsigned bPPUDebugger::mode7_d() { return regs.m7d; }

//$211f
unsigned bPPUDebugger::mode7_x() { return regs.m7x; }

//$2120
unsigned bPPUDebugger::mode7_y() { return regs.m7y; }

//$2121
unsigned bPPUDebugger::cgram_address() { return regs.cgram_addr; }

//$2123
bool bPPUDebugger::bg1_window1_enable() { return regs.window1_enabled[BG1]; }
bool bPPUDebugger::bg1_window1_invert() { return regs.window1_invert [BG1]; }
bool bPPUDebugger::bg1_window2_enable() { return regs.window2_enabled[BG1]; }
bool bPPUDebugger::bg1_window2_invert() { return regs.window2_invert [BG1]; }
bool bPPUDebugger::bg2_window1_enable() { return regs.window1_enabled[BG2]; }
bool bPPUDebugger::bg2_window1_invert() { return regs.window1_invert [BG2]; }
bool bPPUDebugger::bg2_window2_enable() { return regs.window2_enabled[BG2]; }
bool bPPUDebugger::bg2_window2_invert() { return regs.window2_invert [BG2]; }

//$2124
bool bPPUDebugger::bg3_window1_enable() { return regs.window1_enabled[BG3]; }
bool bPPUDebugger::bg3_window1_invert() { return regs.window1_invert [BG3]; }
bool bPPUDebugger::bg3_window2_enable() { return regs.window2_enabled[BG3]; }
bool bPPUDebugger::bg3_window2_invert() { return regs.window2_invert [BG3]; }
bool bPPUDebugger::bg4_window1_enable() { return regs.window1_enabled[BG4]; }
bool bPPUDebugger::bg4_window1_invert() { return regs.window1_invert [BG4]; }
bool bPPUDebugger::bg4_window2_enable() { return regs.window2_enabled[BG4]; }
bool bPPUDebugger::bg4_window2_invert() { return regs.window2_invert [BG4]; }

//$2125
bool bPPUDebugger::oam_window1_enable() { return regs.window1_enabled[OAM]; }
bool bPPUDebugger::oam_window1_invert() { return regs.window1_invert [OAM]; }
bool bPPUDebugger::oam_window2_enable() { return regs.window2_enabled[OAM]; }
bool bPPUDebugger::oam_window2_invert() { return regs.window2_invert [OAM]; }
bool bPPUDebugger::color_window1_enable() { return regs.window1_enabled[COL]; }
bool bPPUDebugger::color_window1_invert() { return regs.window1_invert [COL]; }
bool bPPUDebugger::color_window2_enable() { return regs.window2_enabled[COL]; }
bool bPPUDebugger::color_window2_invert() { return regs.window2_enabled[COL]; }

//$2126
unsigned bPPUDebugger::window1_left() { return regs.window1_left; }

//$2127
unsigned bPPUDebugger::window1_right() { return regs.window1_right; }

//$2128
unsigned bPPUDebugger::window2_left() { return regs.window2_left; }

//$2129
unsigned bPPUDebugger::window2_right() { return regs.window2_right; }

//$212a
unsigned bPPUDebugger::bg1_window_mask() { return regs.window_mask[BG1]; }
unsigned bPPUDebugger::bg2_window_mask() { return regs.window_mask[BG2]; }
unsigned bPPUDebugger::bg3_window_mask() { return regs.window_mask[BG3]; }
unsigned bPPUDebugger::bg4_window_mask() { return regs.window_mask[BG4]; }

//$212b
unsigned bPPUDebugger::oam_window_mask() { return regs.window_mask[OAM]; }
unsigned bPPUDebugger::color_window_mask() { return regs.window_mask[COL]; }

//$212c
bool bPPUDebugger::bg1_mainscreen_enable() { return regs.bg_enabled[BG1]; }
bool bPPUDebugger::bg2_mainscreen_enable() { return regs.bg_enabled[BG2]; }
bool bPPUDebugger::bg3_mainscreen_enable() { return regs.bg_enabled[BG3]; }
bool bPPUDebugger::bg4_mainscreen_enable() { return regs.bg_enabled[BG4]; }
bool bPPUDebugger::oam_mainscreen_enable() { return regs.bg_enabled[OAM]; }

//$212d
bool bPPUDebugger::bg1_subscreen_enable() { return regs.bgsub_enabled[BG1]; }
bool bPPUDebugger::bg2_subscreen_enable() { return regs.bgsub_enabled[BG2]; }
bool bPPUDebugger::bg3_subscreen_enable() { return regs.bgsub_enabled[BG3]; }
bool bPPUDebugger::bg4_subscreen_enable() { return regs.bgsub_enabled[BG4]; }
bool bPPUDebugger::oam_subscreen_enable() { return regs.bgsub_enabled[OAM]; }

//$212e
bool bPPUDebugger::bg1_mainscreen_window_enable() { return regs.window_enabled[BG1]; }
bool bPPUDebugger::bg2_mainscreen_window_enable() { return regs.window_enabled[BG2]; }
bool bPPUDebugger::bg3_mainscreen_window_enable() { return regs.window_enabled[BG3]; }
bool bPPUDebugger::bg4_mainscreen_window_enable() { return regs.window_enabled[BG4]; }
bool bPPUDebugger::oam_mainscreen_window_enable() { return regs.window_enabled[OAM]; }

//$212f
bool bPPUDebugger::bg1_subscreen_window_enable() { return regs.sub_window_enabled[BG1]; }
bool bPPUDebugger::bg2_subscreen_window_enable() { return regs.sub_window_enabled[BG2]; }
bool bPPUDebugger::bg3_subscreen_window_enable() { return regs.sub_window_enabled[BG3]; }
bool bPPUDebugger::bg4_subscreen_window_enable() { return regs.sub_window_enabled[BG4]; }
bool bPPUDebugger::oam_subscreen_window_enable() { return regs.sub_window_enabled[OAM]; }

//$2130
unsigned bPPUDebugger::color_mainscreen_window_mask() { return regs.color_mask; }
unsigned bPPUDebugger::color_subscreen_window_mask() { return regs.colorsub_mask; }
bool bPPUDebugger::color_add_subtract_mode() { return regs.addsub_mode; }
bool bPPUDebugger::direct_color() { return regs.direct_color; }

//$2131
bool bPPUDebugger::color_mode() { return regs.color_mode; }
bool bPPUDebugger::color_halve() { return regs.color_halve; }
bool bPPUDebugger::bg1_color_enable() { return regs.color_enabled[BG1]; }
bool bPPUDebugger::bg2_color_enable() { return regs.color_enabled[BG2]; }
bool bPPUDebugger::bg3_color_enable() { return regs.color_enabled[BG3]; }
bool bPPUDebugger::bg4_color_enable() { return regs.color_enabled[BG4]; }
bool bPPUDebugger::oam_color_enable() { return regs.color_enabled[OAM]; }
bool bPPUDebugger::back_color_enable() { return regs.color_enabled[BACK]; }

//$2132
unsigned bPPUDebugger::color_constant_blue() { return regs.color_b; }
unsigned bPPUDebugger::color_constant_green() { return regs.color_g; }
unsigned bPPUDebugger::color_constant_red() { return regs.color_r; }

//$2133
bool bPPUDebugger::mode7_extbg() { return regs.mode7_extbg; }
bool bPPUDebugger::pseudo_hires() { return regs.pseudo_hires; }
bool bPPUDebugger::overscan() { return regs.overscan; }
bool bPPUDebugger::oam_interlace() { return regs.oam_interlace; }
bool bPPUDebugger::interlace() { return regs.interlace; }

//$213c
unsigned bPPUDebugger::hcounter() { return bPPU::hcounter(); }

//$213d
unsigned bPPUDebugger::vcounter() { return bPPU::vcounter(); }

//$213e
bool bPPUDebugger::range_over() { return regs.range_over; }
bool bPPUDebugger::time_over() { return regs.time_over; }
unsigned bPPUDebugger::ppu1_version() { return PPU::ppu1_version; }

//$213f
bool bPPUDebugger::field() { return cpu.field(); }
bool bPPUDebugger::region() { return bPPU::region; }
unsigned bPPUDebugger::ppu2_version() { return PPU::ppu2_version; }

#endif
