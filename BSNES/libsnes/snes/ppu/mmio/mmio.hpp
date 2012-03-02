public:
  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

private:

struct {
  uint8 ppu1_mdr;
  uint8 ppu2_mdr;

  uint16 vram_readbuffer;
  uint8 oam_latchdata;
  uint8 cgram_latchdata;
  uint8 bgofs_latchdata;
  uint8 mode7_latchdata;
  bool counters_latched;
  bool latch_hcounter;
  bool latch_vcounter;

  uint10 oam_iaddr;
  uint9 cgram_iaddr;

  //$2100  INIDISP
  bool display_disable;
  uint4 display_brightness;

  //$2102  OAMADDL
  //$2103  OAMADDH
  uint10 oam_baseaddr;
  uint10 oam_addr;
  bool oam_priority;

  //$2105  BGMODE
  bool bg3_priority;
  uint8 bgmode;

  //$210d  BG1HOFS
  uint16 mode7_hoffset;

  //$210e  BG1VOFS
  uint16 mode7_voffset;

  //$2115  VMAIN
  bool vram_incmode;
  uint2 vram_mapping;
  uint8 vram_incsize;

  //$2116  VMADDL
  //$2117  VMADDH
  uint16 vram_addr;

  //$211a  M7SEL
  uint2 mode7_repeat;
  bool mode7_vflip;
  bool mode7_hflip;

  //$211b  M7A
  uint16 m7a;

  //$211c  M7B
  uint16 m7b;

  //$211d  M7C
  uint16 m7c;

  //$211e  M7D
  uint16 m7d;

  //$211f  M7X
  uint16 m7x;

  //$2120  M7Y
  uint16 m7y;

  //$2121  CGADD
  uint9 cgram_addr;

  //$2133  SETINI
  bool mode7_extbg;
  bool pseudo_hires;
  bool overscan;
  bool interlace;

  //$213c  OPHCT
  uint16 hcounter;

  //$213d  OPVCT
  uint16 vcounter;
} regs;

uint16 get_vram_address();
uint8 vram_read(unsigned addr);
void vram_write(unsigned addr, uint8 data);

void mmio_update_video_mode();

void mmio_w2100(uint8);  //INIDISP
void mmio_w2101(uint8);  //OBSEL
void mmio_w2102(uint8);  //OAMADDL
void mmio_w2103(uint8);  //OAMADDH
void mmio_w2104(uint8);  //OAMDATA
void mmio_w2105(uint8);  //BGMODE
void mmio_w2106(uint8);  //MOSAIC
void mmio_w2107(uint8);  //BG1SC
void mmio_w2108(uint8);  //BG2SC
void mmio_w2109(uint8);  //BG3SC
void mmio_w210a(uint8);  //BG4SC
void mmio_w210b(uint8);  //BG12NBA
void mmio_w210c(uint8);  //BG34NBA
void mmio_w210d(uint8);  //BG1HOFS
void mmio_w210e(uint8);  //BG1VOFS
void mmio_w210f(uint8);  //BG2HOFS
void mmio_w2110(uint8);  //BG2VOFS
void mmio_w2111(uint8);  //BG3HOFS
void mmio_w2112(uint8);  //BG3VOFS
void mmio_w2113(uint8);  //BG4HOFS
void mmio_w2114(uint8);  //BG4VOFS
void mmio_w2115(uint8);  //VMAIN
void mmio_w2116(uint8);  //VMADDL
void mmio_w2117(uint8);  //VMADDH
void mmio_w2118(uint8);  //VMDATAL
void mmio_w2119(uint8);  //VMDATAH
void mmio_w211a(uint8);  //M7SEL
void mmio_w211b(uint8);  //M7A
void mmio_w211c(uint8);  //M7B
void mmio_w211d(uint8);  //M7C
void mmio_w211e(uint8);  //M7D
void mmio_w211f(uint8);  //M7X
void mmio_w2120(uint8);  //M7Y
void mmio_w2121(uint8);  //CGADD
void mmio_w2122(uint8);  //CGDATA
void mmio_w2123(uint8);  //W12SEL
void mmio_w2124(uint8);  //W34SEL
void mmio_w2125(uint8);  //WOBJSEL
void mmio_w2126(uint8);  //WH0
void mmio_w2127(uint8);  //WH1
void mmio_w2128(uint8);  //WH2
void mmio_w2129(uint8);  //WH3
void mmio_w212a(uint8);  //WBGLOG
void mmio_w212b(uint8);  //WOBJLOG
void mmio_w212c(uint8);  //TM
void mmio_w212d(uint8);  //TS
void mmio_w212e(uint8);  //TMW
void mmio_w212f(uint8);  //TSW
void mmio_w2130(uint8);  //CGWSEL
void mmio_w2131(uint8);  //CGADDSUB
void mmio_w2132(uint8);  //COLDATA
void mmio_w2133(uint8);  //SETINI
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

void mmio_reset();
