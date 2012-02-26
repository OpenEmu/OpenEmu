class Screen {
  uint32 *output;

  struct Regs {
    bool addsub_mode;
    bool direct_color;

    bool color_mode;
    bool color_halve;
    bool bg1_color_enable;
    bool bg2_color_enable;
    bool bg3_color_enable;
    bool bg4_color_enable;
    bool oam_color_enable;
    bool back_color_enable;

    uint5 color_b;
    uint5 color_g;
    uint5 color_r;
  } regs;

  void scanline();
  void run();
  void reset();

  uint32 get_pixel(bool swap);
  uint16 addsub(unsigned x, unsigned y, bool halve);
  uint16 get_color(unsigned palette);
  uint16 get_direct_color(unsigned palette, unsigned tile);

  void serialize(serializer&);
  Screen(PPU &self);

  PPU &self;
  friend class PPU;
};
