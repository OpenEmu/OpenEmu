struct Window {
  struct {
    bool bg1_one_enable;
    bool bg1_one_invert;
    bool bg1_two_enable;
    bool bg1_two_invert;

    bool bg2_one_enable;
    bool bg2_one_invert;
    bool bg2_two_enable;
    bool bg2_two_invert;

    bool bg3_one_enable;
    bool bg3_one_invert;
    bool bg3_two_enable;
    bool bg3_two_invert;

    bool bg4_one_enable;
    bool bg4_one_invert;
    bool bg4_two_enable;
    bool bg4_two_invert;

    bool oam_one_enable;
    bool oam_one_invert;
    bool oam_two_enable;
    bool oam_two_invert;

    bool col_one_enable;
    bool col_one_invert;
    bool col_two_enable;
    bool col_two_invert;

    uint8 one_left;
    uint8 one_right;
    uint8 two_left;
    uint8 two_right;

    uint2 bg1_mask;
    uint2 bg2_mask;
    uint2 bg3_mask;
    uint2 bg4_mask;
    uint2 oam_mask;
    uint2 col_mask;

    bool bg1_main_enable;
    bool bg1_sub_enable;
    bool bg2_main_enable;
    bool bg2_sub_enable;
    bool bg3_main_enable;
    bool bg3_sub_enable;
    bool bg4_main_enable;
    bool bg4_sub_enable;
    bool oam_main_enable;
    bool oam_sub_enable;

    uint2 col_main_mask;
    uint2 col_sub_mask;
  } regs;

  struct Output {
    struct Pixel {
      bool color_enable;
    } main, sub;
  } output;

  struct {
    unsigned x;
    bool one;
    bool two;
  };

  void scanline();
  void run();
  void reset();

  void test(
    bool &main, bool &sub,
    bool one_enable, bool one_invert,
    bool two_enable, bool two_invert,
    uint8 mask, bool main_enable, bool sub_enable
  );

  void serialize(serializer&);
  Window(PPU &self);

  PPU &self;
  friend class PPU;
};
