struct Background {
  struct ID { enum { BG1, BG2, BG3, BG4 }; };
  unsigned id;

  struct Mode { enum  { BPP2, BPP4, BPP8, Mode7, Inactive }; };
  struct ScreenSize { enum { Size32x32, Size32x64, Size64x32, Size64x64 }; };
  struct TileSize { enum { Size8x8, Size16x16 }; };
  struct Screen { enum { Main, Sub }; };

  struct Regs {
    uint16 tiledata_addr;
    uint16 screen_addr;
    uint2 screen_size;
    uint4 mosaic;
    bool tile_size;

    unsigned mode;
    unsigned priority0;
    unsigned priority1;

    bool main_enable;
    bool sub_enable;

    uint16 hoffset;
    uint16 voffset;
  } regs;

  struct Cache {
    uint16 hoffset;
    uint16 voffset;
  } cache;

  unsigned voffset() const;
  unsigned hoffset() const;

  struct Output {
    struct Pixel {
      unsigned priority;  //0 = none (transparent)
      uint8 palette;
      uint16 tile;
    } main, sub;
  } output;

  struct Mosaic : Output::Pixel {
    unsigned vcounter;
    unsigned voffset;
    unsigned hcounter;
    unsigned hoffset;
  } mosaic;

  struct {
    signed x;
    signed y;

    unsigned tile_counter;
    unsigned tile;
    unsigned priority;
    unsigned palette_number;
    unsigned palette_index;
    uint8 data[8];
  };

  void frame();
  void scanline();
  void begin();
  void run(bool screen);
  void reset();

  void get_tile();
  unsigned get_tile_color();
  unsigned get_tile(unsigned x, unsigned y);
  signed clip(signed n);
  void begin_mode7();
  void run_mode7();

  void serialize(serializer&);
  Background(PPU &self, unsigned id);

  PPU &self;
  friend class PPU;
};
