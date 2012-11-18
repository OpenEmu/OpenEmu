struct PPU : Thread, MMIO {
  uint8 vram[96 * 1024];
  uint16 pram[512];
  #include "registers.hpp"
  #include "state.hpp"
  uint32 *output;
  uint16 *blur;

  static void Enter();
  void main();
  void step(unsigned clocks);

  void power();
  void scanline();
  void frame();

  uint8 read(uint32 addr);
  void write(uint32 addr, uint8 byte);

  uint32 vram_read(uint32 addr, uint32 size);
  void vram_write(uint32 addr, uint32 size, uint32 word);

  uint32 pram_read(uint32 addr, uint32 size);
  void pram_write(uint32 addr, uint32 size, uint32 word);

  uint32 oam_read(uint32 addr, uint32 size);
  void oam_write(uint32 addr, uint32 size, uint32 word);

  void render_backgrounds();
  void render_background_linear(Registers::Background&);
  void render_background_affine(Registers::Background&);
  void render_background_bitmap(Registers::Background&);

  void render_objects();
  void render_object(Object&);
  uint8 object_vram_read(unsigned addr) const;

  void render_mosaic_background(unsigned id);
  void render_mosaic_object();

  void render_forceblank();
  void render_screen();
  void render_window(unsigned window);
  unsigned blend(unsigned above, unsigned eva, unsigned below, unsigned evb);

  void serialize(serializer&);
  PPU();
  ~PPU();
};

extern PPU ppu;
