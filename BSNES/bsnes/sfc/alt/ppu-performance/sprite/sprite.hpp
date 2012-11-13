class Sprite {
  bool priority0_enable;
  bool priority1_enable;
  bool priority2_enable;
  bool priority3_enable;

  struct Regs {
    unsigned priority0;
    unsigned priority1;
    unsigned priority2;
    unsigned priority3;

    unsigned base_size;
    unsigned nameselect;
    unsigned tiledata_addr;
    unsigned first_sprite;

    bool main_enable;
    bool sub_enable;

    bool interlace;

    bool time_over;
    bool range_over;
  } regs;

  struct List {
    unsigned width;
    unsigned height;
    unsigned x;
    unsigned y;
    unsigned character;
    bool use_nameselect;
    bool vflip;
    bool hflip;
    unsigned palette;
    unsigned priority;
    bool size;
  } list[128];
  bool list_valid;

  uint8 itemlist[32];
  struct TileList {
    unsigned x;
    unsigned y;
    unsigned priority;
    unsigned palette;
    unsigned tile;
    bool hflip;
  } tilelist[34];

  struct Output {
    uint8 palette[256];
    uint8 priority[256];
  } output;

  LayerWindow window;

  void frame();
  void update_list(unsigned addr, uint8 data);
  void address_reset();
  void set_first();
  alwaysinline bool on_scanline(unsigned sprite);
  void render();

  void serialize(serializer&);
  Sprite(PPU &self);

  PPU &self;
  friend class PPU;
};
