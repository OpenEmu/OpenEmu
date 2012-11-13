class Screen {
  struct Regs {
    bool addsub_mode;
    bool direct_color;

    bool color_mode;
    bool color_halve;
    bool color_enable[7];

    unsigned color_b;
    unsigned color_g;
    unsigned color_r;
    unsigned color;
  } regs;

  struct Output {
    struct Pixel {
      unsigned color;
      unsigned priority;
      unsigned source;
    } main[256], sub[256];

    alwaysinline void plot_main(unsigned x, unsigned color, unsigned priority, unsigned source);
    alwaysinline void plot_sub(unsigned x, unsigned color, unsigned priority, unsigned source);
  } output;

  ColorWindow window;

  alwaysinline unsigned get_palette(unsigned color);
  unsigned get_direct_color(unsigned palette, unsigned tile);
  alwaysinline uint16 addsub(unsigned x, unsigned y, bool halve);
  void scanline();
  void render_black();
  alwaysinline uint16 get_pixel_main(unsigned x);
  alwaysinline uint16 get_pixel_sub(unsigned x);
  void render();

  void serialize(serializer&);
  Screen(PPU &self);
  ~Screen();

  PPU &self;
  friend class PPU;
};
