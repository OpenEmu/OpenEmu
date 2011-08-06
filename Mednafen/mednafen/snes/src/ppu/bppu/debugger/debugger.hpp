class bPPUDebugger : public bPPU, public PPUDebugger {
public:
  bool bg1_enabled[2];
  bool bg2_enabled[2];
  bool bg3_enabled[2];
  bool bg4_enabled[2];
  bool oam_enabled[4];

  uint8 vram_mmio_read(uint16 addr);
  void vram_mmio_write(uint16 addr, uint8 data);

  uint8 oam_mmio_read(uint16 addr);
  void oam_mmio_write(uint16 addr, uint8 data);

  uint8 cgram_mmio_read(uint16 addr);
  void cgram_mmio_write(uint16 addr, uint8 data);

  void render_line_mode0();
  void render_line_mode1();
  void render_line_mode2();
  void render_line_mode3();
  void render_line_mode4();
  void render_line_mode5();
  void render_line_mode6();
  void render_line_mode7();

  bPPUDebugger();

  //===========
  //PPUDebugger
  //===========

  //internal
  unsigned ppu1_mdr();
  unsigned ppu2_mdr();

  //$2100
  bool display_disable();
  unsigned display_brightness();

  //$2101
  unsigned oam_base_size();
  unsigned oam_name_select();
  unsigned oam_name_base_address();

  //$2102-$2103
  unsigned oam_base_address();
  bool oam_priority();

  //$2105
  bool bg1_tile_size();
  bool bg2_tile_size();
  bool bg3_tile_size();
  bool bg4_tile_size();
  bool bg3_priority();
  unsigned bg_mode();

  //$2106
  unsigned mosaic_size();
  bool bg1_mosaic_enable();
  bool bg2_mosaic_enable();
  bool bg3_mosaic_enable();
  bool bg4_mosaic_enable();

  //$2107
  unsigned bg1_screen_address();
  unsigned bg1_screen_size();

  //$2108
  unsigned bg2_screen_address();
  unsigned bg2_screen_size();

  //$2109
  unsigned bg3_screen_address();
  unsigned bg3_screen_size();

  //$210a
  unsigned bg4_screen_address();
  unsigned bg4_screen_size();

  //$210b
  unsigned bg1_name_base_address();
  unsigned bg2_name_base_address();

  //$210c
  unsigned bg3_name_base_address();
  unsigned bg4_name_base_address();

  //$210d
  unsigned mode7_hoffset();
  unsigned bg1_hoffset();

  //$210e
  unsigned mode7_voffset();
  unsigned bg1_voffset();

  //$210f
  unsigned bg2_hoffset();

  //$2110
  unsigned bg2_voffset();

  //$2111
  unsigned bg3_hoffset();

  //$2112
  unsigned bg3_voffset();

  //$2113
  unsigned bg4_hoffset();

  //$2114
  unsigned bg4_voffset();

  //$2115
  bool vram_increment_mode();
  unsigned vram_increment_formation();
  unsigned vram_increment_size();

  //$2116-$2117
  unsigned vram_address();

  //$211a
  unsigned mode7_repeat();
  bool mode7_vflip();
  bool mode7_hflip();

  //$211b
  unsigned mode7_a();

  //$211c
  unsigned mode7_b();

  //$211d
  unsigned mode7_c();

  //$211e
  unsigned mode7_d();

  //$211f
  unsigned mode7_x();

  //$2120
  unsigned mode7_y();

  //$2121
  unsigned cgram_address();

  //$2123
  bool bg1_window1_enable();
  bool bg1_window1_invert();
  bool bg1_window2_enable();
  bool bg1_window2_invert();
  bool bg2_window1_enable();
  bool bg2_window1_invert();
  bool bg2_window2_enable();
  bool bg2_window2_invert();

  //$2124
  bool bg3_window1_enable();
  bool bg3_window1_invert();
  bool bg3_window2_enable();
  bool bg3_window2_invert();
  bool bg4_window1_enable();
  bool bg4_window1_invert();
  bool bg4_window2_enable();
  bool bg4_window2_invert();

  //$2125
  bool oam_window1_enable();
  bool oam_window1_invert();
  bool oam_window2_enable();
  bool oam_window2_invert();
  bool color_window1_enable();
  bool color_window1_invert();
  bool color_window2_enable();
  bool color_window2_invert();

  //$2126
  unsigned window1_left();

  //$2127
  unsigned window1_right();

  //$2128
  unsigned window2_left();

  //$2129
  unsigned window2_right();

  //$212a
  unsigned bg1_window_mask();
  unsigned bg2_window_mask();
  unsigned bg3_window_mask();
  unsigned bg4_window_mask();

  //$212b
  unsigned oam_window_mask();
  unsigned color_window_mask();

  //$212c
  bool bg1_mainscreen_enable();
  bool bg2_mainscreen_enable();
  bool bg3_mainscreen_enable();
  bool bg4_mainscreen_enable();
  bool oam_mainscreen_enable();

  //$212d
  bool bg1_subscreen_enable();
  bool bg2_subscreen_enable();
  bool bg3_subscreen_enable();
  bool bg4_subscreen_enable();
  bool oam_subscreen_enable();

  //$212e
  bool bg1_mainscreen_window_enable();
  bool bg2_mainscreen_window_enable();
  bool bg3_mainscreen_window_enable();
  bool bg4_mainscreen_window_enable();
  bool oam_mainscreen_window_enable();

  //$212f
  bool bg1_subscreen_window_enable();
  bool bg2_subscreen_window_enable();
  bool bg3_subscreen_window_enable();
  bool bg4_subscreen_window_enable();
  bool oam_subscreen_window_enable();

  //$2130
  unsigned color_mainscreen_window_mask();
  unsigned color_subscreen_window_mask();
  bool color_add_subtract_mode();
  bool direct_color();

  //$2131
  bool color_mode();
  bool color_halve();
  bool bg1_color_enable();
  bool bg2_color_enable();
  bool bg3_color_enable();
  bool bg4_color_enable();
  bool oam_color_enable();
  bool back_color_enable();

  //$2132
  unsigned color_constant_blue();
  unsigned color_constant_green();
  unsigned color_constant_red();

  //$2133
  bool mode7_extbg();
  bool pseudo_hires();
  bool overscan();
  bool oam_interlace();
  bool interlace();

  //$213c
  unsigned hcounter();

  //$213d
  unsigned vcounter();

  //$213e
  bool range_over();
  bool time_over();
  unsigned ppu1_version();

  //$213f
  bool field();
  bool region();
  unsigned ppu2_version();
};
