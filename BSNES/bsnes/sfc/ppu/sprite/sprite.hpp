struct Sprite {
  struct SpriteItem {
    uint16 x;
    uint16 y;
    uint8 character;
    bool nameselect;
    bool vflip;
    bool hflip;
    uint8 priority;
    uint8 palette;
    bool size;
    unsigned width() const;
    unsigned height() const;
  } list[128];

  struct TileItem {
    uint16 x;
    uint16 priority;
    uint16 palette;
    bool hflip;
    uint8 d0, d1, d2, d3;
  };

  struct State {
    unsigned x;
    unsigned y;

    unsigned item_count;
    unsigned tile_count;

    bool active;
    uint8 item[2][32];
    TileItem tile[2][34];
  } t;

  struct Regs {
    bool main_enable;
    bool sub_enable;
    bool interlace;

    uint3 base_size;
    uint2 nameselect;
    uint16 tiledata_addr;
    uint8 first_sprite;

    unsigned priority0;
    unsigned priority1;
    unsigned priority2;
    unsigned priority3;

    bool time_over;
    bool range_over;
  } regs;

  struct Output {
    struct Pixel {
      unsigned priority;  //0 = none (transparent)
      uint8 palette;
    } main, sub;
  } output;

  //list.cpp
  void update(unsigned addr, uint8 data);
  void synchronize();

  //sprite.cpp
  void address_reset();
  void set_first_sprite();
  void frame();
  void scanline();
  void run();
  void tilefetch();
  void reset();

  bool on_scanline(SpriteItem&);

  void serialize(serializer&);
  Sprite(PPU &self);

  PPU &self;
  friend class PPU;
};
