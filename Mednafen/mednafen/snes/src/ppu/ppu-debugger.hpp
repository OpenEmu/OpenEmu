struct PPUDebugger : ChipDebugger {
  bool property(unsigned id, string &name, string &value);

  //internal
  virtual unsigned ppu1_mdr() { return 0; }
  virtual unsigned ppu2_mdr() { return 0; }

  //$2100
  virtual bool display_disable() { return 0; }
  virtual unsigned display_brightness() { return 0; }

  //$2101
  virtual unsigned oam_base_size() { return 0; }
  virtual unsigned oam_name_select() { return 0; }
  virtual unsigned oam_name_base_address() { return 0; }

  //$2102-$2103
  virtual unsigned oam_base_address() { return 0; }
  virtual bool oam_priority() { return 0; }

  //$2105
  virtual bool bg1_tile_size() { return 0; }
  virtual bool bg2_tile_size() { return 0; }
  virtual bool bg3_tile_size() { return 0; }
  virtual bool bg4_tile_size() { return 0; }
  virtual bool bg3_priority() { return 0; }
  virtual unsigned bg_mode() { return 0; }

  //$2106
  virtual unsigned mosaic_size() { return 0; }
  virtual bool bg1_mosaic_enable() { return 0; }
  virtual bool bg2_mosaic_enable() { return 0; }
  virtual bool bg3_mosaic_enable() { return 0; }
  virtual bool bg4_mosaic_enable() { return 0; }

  //$2107
  virtual unsigned bg1_screen_address() { return 0; }
  virtual unsigned bg1_screen_size() { return 0; }

  //$2108
  virtual unsigned bg2_screen_address() { return 0; }
  virtual unsigned bg2_screen_size() { return 0; }

  //$2109
  virtual unsigned bg3_screen_address() { return 0; }
  virtual unsigned bg3_screen_size() { return 0; }

  //$210a
  virtual unsigned bg4_screen_address() { return 0; }
  virtual unsigned bg4_screen_size() { return 0; }

  //$210b
  virtual unsigned bg1_name_base_address() { return 0; }
  virtual unsigned bg2_name_base_address() { return 0; }

  //$210c
  virtual unsigned bg3_name_base_address() { return 0; }
  virtual unsigned bg4_name_base_address() { return 0; }

  //$210d
  virtual unsigned mode7_hoffset() { return 0; }
  virtual unsigned bg1_hoffset() { return 0; }

  //$210e
  virtual unsigned mode7_voffset() { return 0; }
  virtual unsigned bg1_voffset() { return 0; }

  //$210f
  virtual unsigned bg2_hoffset() { return 0; }

  //$2110
  virtual unsigned bg2_voffset() { return 0; }

  //$2111
  virtual unsigned bg3_hoffset() { return 0; }

  //$2112
  virtual unsigned bg3_voffset() { return 0; }

  //$2113
  virtual unsigned bg4_hoffset() { return 0; }

  //$2114
  virtual unsigned bg4_voffset() { return 0; }

  //$2115
  virtual bool vram_increment_mode() { return 0; }
  virtual unsigned vram_increment_formation() { return 0; }
  virtual unsigned vram_increment_size() { return 0; }

  //$2116-$2117
  virtual unsigned vram_address() { return 0; }

  //$211a
  virtual unsigned mode7_repeat() { return 0; }
  virtual bool mode7_vflip() { return 0; }
  virtual bool mode7_hflip() { return 0; }

  //$211b
  virtual unsigned mode7_a() { return 0; }

  //$211c
  virtual unsigned mode7_b() { return 0; }

  //$211d
  virtual unsigned mode7_c() { return 0; }

  //$211e
  virtual unsigned mode7_d() { return 0; }

  //$211f
  virtual unsigned mode7_x() { return 0; }

  //$2120
  virtual unsigned mode7_y() { return 0; }

  //$2121
  virtual unsigned cgram_address() { return 0; }

  //$2123
  virtual bool bg1_window1_enable() { return 0; }
  virtual bool bg1_window1_invert() { return 0; }
  virtual bool bg1_window2_enable() { return 0; }
  virtual bool bg1_window2_invert() { return 0; }
  virtual bool bg2_window1_enable() { return 0; }
  virtual bool bg2_window1_invert() { return 0; }
  virtual bool bg2_window2_enable() { return 0; }
  virtual bool bg2_window2_invert() { return 0; }

  //$2124
  virtual bool bg3_window1_enable() { return 0; }
  virtual bool bg3_window1_invert() { return 0; }
  virtual bool bg3_window2_enable() { return 0; }
  virtual bool bg3_window2_invert() { return 0; }
  virtual bool bg4_window1_enable() { return 0; }
  virtual bool bg4_window1_invert() { return 0; }
  virtual bool bg4_window2_enable() { return 0; }
  virtual bool bg4_window2_invert() { return 0; }

  //$2125
  virtual bool oam_window1_enable() { return 0; }
  virtual bool oam_window1_invert() { return 0; }
  virtual bool oam_window2_enable() { return 0; }
  virtual bool oam_window2_invert() { return 0; }
  virtual bool color_window1_enable() { return 0; }
  virtual bool color_window1_invert() { return 0; }
  virtual bool color_window2_enable() { return 0; }
  virtual bool color_window2_invert() { return 0; }

  //$2126
  virtual unsigned window1_left() { return 0; }

  //$2127
  virtual unsigned window1_right() { return 0; }

  //$2128
  virtual unsigned window2_left() { return 0; }

  //$2129
  virtual unsigned window2_right() { return 0; }

  //$212a
  virtual unsigned bg1_window_mask() { return 0; }
  virtual unsigned bg2_window_mask() { return 0; }
  virtual unsigned bg3_window_mask() { return 0; }
  virtual unsigned bg4_window_mask() { return 0; }

  //$212b
  virtual unsigned oam_window_mask() { return 0; }
  virtual unsigned color_window_mask() { return 0; }

  //$212c
  virtual bool bg1_mainscreen_enable() { return 0; }
  virtual bool bg2_mainscreen_enable() { return 0; }
  virtual bool bg3_mainscreen_enable() { return 0; }
  virtual bool bg4_mainscreen_enable() { return 0; }
  virtual bool oam_mainscreen_enable() { return 0; }

  //$212d
  virtual bool bg1_subscreen_enable() { return 0; }
  virtual bool bg2_subscreen_enable() { return 0; }
  virtual bool bg3_subscreen_enable() { return 0; }
  virtual bool bg4_subscreen_enable() { return 0; }
  virtual bool oam_subscreen_enable() { return 0; }

  //$212e
  virtual bool bg1_mainscreen_window_enable() { return 0; }
  virtual bool bg2_mainscreen_window_enable() { return 0; }
  virtual bool bg3_mainscreen_window_enable() { return 0; }
  virtual bool bg4_mainscreen_window_enable() { return 0; }
  virtual bool oam_mainscreen_window_enable() { return 0; }

  //$212f
  virtual bool bg1_subscreen_window_enable() { return 0; }
  virtual bool bg2_subscreen_window_enable() { return 0; }
  virtual bool bg3_subscreen_window_enable() { return 0; }
  virtual bool bg4_subscreen_window_enable() { return 0; }
  virtual bool oam_subscreen_window_enable() { return 0; }

  //$2130
  virtual unsigned color_mainscreen_window_mask() { return 0; }
  virtual unsigned color_subscreen_window_mask() { return 0; }
  virtual bool color_add_subtract_mode() { return 0; }
  virtual bool direct_color() { return 0; }

  //$2131
  virtual bool color_mode() { return 0; }
  virtual bool color_halve() { return 0; }
  virtual bool bg1_color_enable() { return 0; }
  virtual bool bg2_color_enable() { return 0; }
  virtual bool bg3_color_enable() { return 0; }
  virtual bool bg4_color_enable() { return 0; }
  virtual bool oam_color_enable() { return 0; }
  virtual bool back_color_enable() { return 0; }

  //$2132
  virtual unsigned color_constant_blue() { return 0; }
  virtual unsigned color_constant_green() { return 0; }
  virtual unsigned color_constant_red() { return 0; }

  //$2133
  virtual bool mode7_extbg() { return 0; }
  virtual bool pseudo_hires() { return 0; }
  virtual bool overscan() { return 0; }
  virtual bool oam_interlace() { return 0; }
  virtual bool interlace() { return 0; }

  //$213c
  virtual unsigned hcounter() { return 0; }

  //$213d
  virtual unsigned vcounter() { return 0; }

  //$213e
  virtual bool range_over() { return 0; }
  virtual bool time_over() { return 0; }
  virtual unsigned ppu1_version() { return 0; }

  //$213f
  virtual bool field() { return 0; }
  virtual bool region() { return 0; }
  virtual unsigned ppu2_version() { return 0; }
};
