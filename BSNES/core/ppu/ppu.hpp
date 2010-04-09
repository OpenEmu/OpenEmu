//PPUcounter emulates the H/V latch counters of the S-PPU2.
//
//real hardware has the S-CPU maintain its own copy of these counters that are
//updated based on the state of the S-PPU Vblank and Hblank pins. emulating this
//would require full lock-step synchronization for every clock tick.
//to bypass this and allow the two to run out-of-order, both the CPU and PPU
//classes inherit PPUcounter and keep their own counters.
//the timers are kept in sync, as the only differences occur on V=240 and V=261,
//based on interlace. thus, we need only synchronize and fetch interlace at any
//point before this in the frame, which is handled internally by this class at
//V=128.

class PPUcounter {
public:
  alwaysinline void tick();
  alwaysinline void tick(unsigned clocks);

  alwaysinline bool   field   () const;
  alwaysinline uint16 vcounter() const;
  alwaysinline uint16 hcounter() const;
  inline uint16 hdot() const;
  inline uint16 lineclocks() const;

  alwaysinline bool   field   (unsigned offset) const;
  alwaysinline uint16 vcounter(unsigned offset) const;
  alwaysinline uint16 hcounter(unsigned offset) const;

  inline void reset();
  function<void ()> scanline;
  void serialize(serializer&);

private:
  inline void vcounter_tick();

  struct {
    bool interlace;
    bool field;
    uint16 vcounter;
    uint16 hcounter;
  } status;

  struct {
    bool field[2048];
    uint16 vcounter[2048];
    uint16 hcounter[2048];

    int32 index;
  } history;
};

class PPU : public PPUcounter, public MMIO {
public:
  virtual void enter() = 0;

  uint16 *output;

  struct {
    bool render_output;
    bool frame_executed;
    bool frames_updated;
    unsigned frames_rendered;
    unsigned frames_executed;
  } status;

  //PPU1 version number
  //* 1 is known
  //* reported by $213e
  uint8 ppu1_version;

  //PPU2 version number
  //* 1 and 3 are known
  //* reported by $213f
  uint8 ppu2_version;

  virtual bool interlace() const = 0;
  virtual bool overscan() const = 0;
  virtual bool hires() const = 0;

  virtual void latch_counters() = 0;

  virtual void frame();
  virtual void power();
  virtual void reset();
  virtual void enable_renderer(bool r);
  virtual bool renderer_enabled();

  virtual void serialize(serializer&);
  PPU();
  virtual ~PPU();
};
