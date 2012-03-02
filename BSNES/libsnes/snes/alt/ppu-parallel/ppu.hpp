struct PPU : public Processor, public PPUcounter {
  uint8 vram[64 * 1024];
  uint8 oam[544];
  uint8 cgram[512];
  uint8 r[64];

  enum : bool { Threaded = false };

  void latch_counters();
  bool interlace() const;
  bool overscan() const;
  bool hires() const;

  void enter();
  void enable();
  void power();
  void reset();

  void scanline();
  void frame();

  void serialize(serializer&);
  PPU();
  ~PPU();

private:
  uint32 *surface;
  uint32 *output;

  //mmio.cpp
  alwaysinline bool display_disable() const;
  alwaysinline unsigned display_brightness() const;

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  friend class Video;
};

extern PPU ppu;
