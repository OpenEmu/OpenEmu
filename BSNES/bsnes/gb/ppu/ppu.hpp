struct PPU : Thread, MMIO {
  struct Status {
    unsigned lx;
    unsigned wyc;

    //$ff40  LCDC
    bool display_enable;
    bool window_tilemap_select;
    bool window_display_enable;
    bool bg_tiledata_select;
    bool bg_tilemap_select;
    bool ob_size;
    bool ob_enable;
    bool bg_enable;

    //$ff41  STAT
    bool interrupt_lyc;
    bool interrupt_oam;
    bool interrupt_vblank;
    bool interrupt_hblank;

    //$ff42  SCY
    uint8 scy;

    //$ff43  SCX
    uint8 scx;

    //$ff44  LY
    uint8 ly;

    //$ff45  LYC
    uint8 lyc;

    //$ff4a  WY
    uint8 wy;

    //$ff4b  WX
    uint8 wx;

    //$ff4f  VBK
    bool vram_bank;

    //$ff68  BGPI
    bool bgpi_increment;
    uint6 bgpi;

    //$ff6a  OBPI
    bool obpi_increment;
    uint8 obpi;
  } status;

  uint32 screen[160 * 144];
  uint16 line[160];
  struct Origin { enum : unsigned { None, BG, BGP, OB }; };
  uint8 origin[160];

  uint8 vram[16384];  //GB = 8192, GBC = 16384
  uint8 oam[160];
  uint8 bgp[4];
  uint8 obp[2][4];
  uint8 bgpd[64];
  uint8 obpd[64];

  static void Main();
  void main();
  void add_clocks(unsigned clocks);
  void scanline();
  void frame();

  unsigned hflip(unsigned data) const;

  //mmio.cpp
  unsigned vram_addr(uint16 addr) const;
  uint8 mmio_read(uint16 addr);
  void mmio_write(uint16 addr, uint8 data);

  //dmg.cpp
  void dmg_render();
  uint16 dmg_read_tile(bool select, unsigned x, unsigned y);
  void dmg_render_bg();
  void dmg_render_window();
  void dmg_render_ob();

  //cgb.cpp
  void cgb_render();
  void cgb_read_tile(bool select, unsigned x, unsigned y, unsigned &tile, unsigned &attr, unsigned &data);
  void cgb_render_bg();
  void cgb_render_window();
  void cgb_render_ob();

  void power();

  void serialize(serializer&);
  PPU();
};

extern PPU ppu;
