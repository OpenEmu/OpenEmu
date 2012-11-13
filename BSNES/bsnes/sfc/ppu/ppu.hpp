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

  void serialize(serializer&);
  PPU();
  ~PPU();

privileged:
  uint32 *surface;
  uint32 *output;

  uint8 ppu1_version;
  uint8 ppu2_version;

  struct {
    bool interlace;
    bool overscan;
  } display;

  #include "background/background.hpp"
  #include "mmio/mmio.hpp"
  #include "screen/screen.hpp"
  #include "sprite/sprite.hpp"
  #include "window/window.hpp"

  Background bg1;
  Background bg2;
  Background bg3;
  Background bg4;
  Sprite sprite;
  Window window;
  Screen screen;

  static void Enter();
  void add_clocks(unsigned);

  void scanline();
  void frame();

  friend class PPU::Background;
  friend class PPU::Sprite;
  friend class PPU::Window;
  friend class PPU::Screen;
  friend class Video;

  struct Debugger {
    hook<void (uint16)> vram_read;
    hook<void (uint16)> oam_read;
    hook<void (uint16)> cgram_read;
    hook<void (uint16, uint8)> vram_write;
    hook<void (uint16, uint8)> oam_write;
    hook<void (uint16, uint8)> cgram_write;
  } debugger;
};

extern PPU ppu;
