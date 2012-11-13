struct PPU : Thread {
  static void Main();
  void main();
  void tick();

  void scanline();
  void frame();

  void power();
  void reset();

  uint8 read(uint16 addr);
  void write(uint16 addr, uint8 data);

  uint8 ciram_read(uint16 addr);
  void ciram_write(uint16 addr, uint8 data);

  uint8 cgram_read(uint16 addr);
  void cgram_write(uint16 addr, uint8 data);

  bool raster_enable() const;
  unsigned nametable_addr() const;
  unsigned scrollx() const;
  unsigned scrolly() const;
  unsigned sprite_height() const;

  uint8 chr_load(uint16 addr);

  void scrollx_increment();
  void scrolly_increment();

  void raster_pixel();
  void raster_sprite();
  void raster_scanline();

  void serialize(serializer&);

  struct Status {
    uint8 mdr;

    bool field;
    unsigned lx;
    unsigned ly;

    uint8 bus_data;

    bool address_latch;

    uint15 vaddr;
    uint15 taddr;
    uint8 xaddr;

    bool nmi_hold;
    bool nmi_flag;

    //$2000
    bool nmi_enable;
    bool master_select;
    bool sprite_size;
    unsigned bg_addr;
    unsigned sprite_addr;
    unsigned vram_increment;

    //$2001
    uint3 emphasis;
    bool sprite_enable;
    bool bg_enable;
    bool sprite_edge_enable;
    bool bg_edge_enable;
    bool grayscale;

    //$2002
    bool sprite_zero_hit;
    bool sprite_overflow;

    //$2003
    uint8 oam_addr;
  } status;

  struct Raster {
    uint16 nametable;
    uint16 attribute;
    uint16 tiledatalo;
    uint16 tiledatahi;

    unsigned oam_iterator;
    unsigned oam_counter;

    struct OAM {
      uint8 id;
      uint8 y;
      uint8 tile;
      uint8 attr;
      uint8 x;

      uint8 tiledatalo;
      uint8 tiledatahi;
    } oam[8], soam[8];
  } raster;

  uint32 buffer[256 * 262];
  uint8 ciram[2048];
  uint8 cgram[32];
  uint8 oam[256];
};

extern PPU ppu;
