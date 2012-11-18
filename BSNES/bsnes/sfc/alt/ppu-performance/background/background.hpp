class Background {
  struct ID { enum { BG1, BG2, BG3, BG4 }; };
  struct Mode { enum { BPP2, BPP4, BPP8, Mode7, Inactive }; };
  struct ScreenSize { enum { Size32x32, Size32x64, Size64x32, Size64x64 }; };
  struct TileSize { enum { Size8x8, Size16x16 }; };

  bool priority0_enable;
  bool priority1_enable;

  struct Regs {
    unsigned mode;
    unsigned priority0;
    unsigned priority1;

    bool tile_size;
    unsigned mosaic;

    unsigned screen_addr;
    unsigned screen_size;
    unsigned tiledata_addr;

    unsigned hoffset;
    unsigned voffset;

    bool main_enable;
    bool sub_enable;
  } regs;

  uint16 **mosaic_table;

  const unsigned id;
  unsigned opt_valid_bit;

  bool hires;
  signed width;

  unsigned tile_width;
  unsigned tile_height;

  unsigned mask_x;
  unsigned mask_y;

  unsigned scx;
  unsigned scy;

  unsigned hscroll;
  unsigned vscroll;

  unsigned mosaic_vcounter;
  unsigned mosaic_voffset;

  LayerWindow window;

  alwaysinline unsigned get_tile(unsigned hoffset, unsigned voffset);
  void offset_per_tile(unsigned x, unsigned y, unsigned &hoffset, unsigned &voffset);
  void scanline();
  void render();
  void render_mode7();

  void serialize(serializer&);
  Background(PPU &self, unsigned id);
  ~Background();

  PPU &self;
  friend class PPU;
};
