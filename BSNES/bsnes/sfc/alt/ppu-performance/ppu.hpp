struct PPU : Thread, public PPUcounter {
  uint8 vram[64 * 1024];
  uint8 oam[544];
  uint8 cgram[512];

  enum : bool { Threaded = true };
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_cpu();

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

  void layer_enable(unsigned layer, unsigned priority, bool enable);
  void set_frameskip(unsigned frameskip);

  void serialize(serializer&);
  PPU();
  ~PPU();

private:
  uint32 *surface;
  uint32 *output;

  #include "mmio/mmio.hpp"
  #include "window/window.hpp"
  #include "cache/cache.hpp"
  #include "background/background.hpp"
  #include "sprite/sprite.hpp"
  #include "screen/screen.hpp"

  Cache cache;
  Background bg1;
  Background bg2;
  Background bg3;
  Background bg4;
  Sprite sprite;
  Screen screen;

  struct Display {
    bool interlace;
    bool overscan;
    unsigned width;
    unsigned height;
    unsigned frameskip;
    unsigned framecounter;
  } display;

  static void Enter();
  void add_clocks(unsigned clocks);
  void render_scanline();

  friend class PPU::Cache;
  friend class PPU::Background;
  friend class PPU::Sprite;
  friend class PPU::Screen;
  friend class Video;
};

extern PPU ppu;
