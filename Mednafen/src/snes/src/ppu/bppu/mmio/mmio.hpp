struct {
  //open bus support
  uint8 ppu1_mdr, ppu2_mdr;

  //bg line counters
  uint16 bg_y[4];

  //$2100
  bool   display_disabled;
  uint8  display_brightness;

  //$2101
  uint8  oam_basesize;
  uint8  oam_nameselect;
  uint16 oam_tdaddr;

  //$2102-$2103
  uint16 oam_baseaddr;
  uint16 oam_addr;
  bool   oam_priority;
  uint8  oam_firstsprite;

  //$2104
  uint8  oam_latchdata;

  //$2105
  bool   bg_tilesize[4];
  bool   bg3_priority;
  uint8  bg_mode;

  //$2106
  uint8  mosaic_size;
  bool   mosaic_enabled[4];
  uint16 mosaic_countdown;

  //$2107-$210a
  uint16 bg_scaddr[4];
  uint8  bg_scsize[4];

  //$210b-$210c
  uint16 bg_tdaddr[4];

  //$210d-$2114
  uint8  bg_ofslatch;
  uint16 m7_hofs, m7_vofs;
  uint16 bg_hofs[4];
  uint16 bg_vofs[4];

  //$2115
  bool   vram_incmode;
  uint8  vram_mapping;
  uint8  vram_incsize;

  //$2116-$2117
  uint16 vram_addr;

  //$211a
  uint8  mode7_repeat;
  bool   mode7_vflip;
  bool   mode7_hflip;

  //$211b-$2120
  uint8  m7_latch;
  uint16 m7a, m7b, m7c, m7d, m7x, m7y;

  //$2121
  uint16 cgram_addr;

  //$2122
  uint8  cgram_latchdata;

  //$2123-$2125
  bool   window1_enabled[6];
  bool   window1_invert [6];
  bool   window2_enabled[6];
  bool   window2_invert [6];

  //$2126-$2129
  uint8  window1_left, window1_right;
  uint8  window2_left, window2_right;

  //$212a-$212b
  uint8  window_mask[6];

  //$212c-$212d
  bool   bg_enabled[5], bgsub_enabled[5];

  //$212e-$212f
  bool   window_enabled[5], sub_window_enabled[5];

  //$2130
  uint8  color_mask, colorsub_mask;
  bool   addsub_mode;
  bool   direct_color;

  //$2131
  bool   color_mode, color_halve;
  bool   color_enabled[6];

  //$2132
  uint8  color_r, color_g, color_b;
  uint16 color_rgb;

  //$2133
  //overscan and interlace are checked once per frame to
  //determine if entire frame should be interlaced/non-interlace
  //and overscan adjusted. therefore, the variables act sort of
  //like a buffer, but they do still affect internal rendering
  bool   mode7_extbg;
  bool   pseudo_hires;
  bool   overscan;
  uint16 scanlines;
  bool   oam_interlace;
  bool   interlace;

  //$2137
  uint16 hcounter, vcounter;
  bool   latch_hcounter, latch_vcounter;
  bool   counters_latched;

  //$2139-$213a
  uint16 vram_readbuffer;

  //$213e
  bool   time_over, range_over;
  uint16 oam_itemcount, oam_tilecount;
} regs;

void mmio_w2100(uint8 value);  //INIDISP
void mmio_w2101(uint8 value);  //OBSEL
void mmio_w2102(uint8 value);  //OAMADDL
void mmio_w2103(uint8 value);  //OAMADDH
void mmio_w2104(uint8 value);  //OAMDATA
void mmio_w2105(uint8 value);  //BGMODE
void mmio_w2106(uint8 value);  //MOSAIC
void mmio_w2107(uint8 value);  //BG1SC
void mmio_w2108(uint8 value);  //BG2SC
void mmio_w2109(uint8 value);  //BG3SC
void mmio_w210a(uint8 value);  //BG4SC
void mmio_w210b(uint8 value);  //BG12NBA
void mmio_w210c(uint8 value);  //BG34NBA
void mmio_w210d(uint8 value);  //BG1HOFS
void mmio_w210e(uint8 value);  //BG1VOFS
void mmio_w210f(uint8 value);  //BG2HOFS
void mmio_w2110(uint8 value);  //BG2VOFS
void mmio_w2111(uint8 value);  //BG3HOFS
void mmio_w2112(uint8 value);  //BG3VOFS
void mmio_w2113(uint8 value);  //BG4HOFS
void mmio_w2114(uint8 value);  //BG4VOFS
void mmio_w2115(uint8 value);  //VMAIN
void mmio_w2116(uint8 value);  //VMADDL
void mmio_w2117(uint8 value);  //VMADDH
void mmio_w2118(uint8 value);  //VMDATAL
void mmio_w2119(uint8 value);  //VMDATAH
void mmio_w211a(uint8 value);  //M7SEL
void mmio_w211b(uint8 value);  //M7A
void mmio_w211c(uint8 value);  //M7B
void mmio_w211d(uint8 value);  //M7C
void mmio_w211e(uint8 value);  //M7D
void mmio_w211f(uint8 value);  //M7X
void mmio_w2120(uint8 value);  //M7Y
void mmio_w2121(uint8 value);  //CGADD
void mmio_w2122(uint8 value);  //CGDATA
void mmio_w2123(uint8 value);  //W12SEL
void mmio_w2124(uint8 value);  //W34SEL
void mmio_w2125(uint8 value);  //WOBJSEL
void mmio_w2126(uint8 value);  //WH0
void mmio_w2127(uint8 value);  //WH1
void mmio_w2128(uint8 value);  //WH2
void mmio_w2129(uint8 value);  //WH3
void mmio_w212a(uint8 value);  //WBGLOG
void mmio_w212b(uint8 value);  //WOBJLOG
void mmio_w212c(uint8 value);  //TM
void mmio_w212d(uint8 value);  //TS
void mmio_w212e(uint8 value);  //TMW
void mmio_w212f(uint8 value);  //TSW
void mmio_w2130(uint8 value);  //CGWSEL
void mmio_w2131(uint8 value);  //CGADDSUB
void mmio_w2132(uint8 value);  //COLDATA
void mmio_w2133(uint8 value);  //SETINI

uint8 mmio_r2134();  //MPYL
uint8 mmio_r2135();  //MPYM
uint8 mmio_r2136();  //MPYH
uint8 mmio_r2137();  //SLHV
uint8 mmio_r2138();  //OAMDATAREAD
uint8 mmio_r2139();  //VMDATALREAD
uint8 mmio_r213a();  //VMDATAHREAD
uint8 mmio_r213b();  //CGDATAREAD
uint8 mmio_r213c();  //OPHCT
uint8 mmio_r213d();  //OPVCT
uint8 mmio_r213e();  //STAT77
uint8 mmio_r213f();  //STAT78

uint8 mmio_read(unsigned addr);
void mmio_write(unsigned addr, uint8 data);

void latch_counters();
