class SA1 : public Coprocessor, public CPUcore {
public:
  #include "bus/bus.hpp"
  #include "dma/dma.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"

  struct Status {
    uint8 tick_counter;

    bool interrupt_pending;

    uint16 scanlines;
    uint16 vcounter;
    uint16 hcounter;
  } status;

  static void Enter();
  void enter();
  void tick();
  void op_irq();

  alwaysinline void trigger_irq();
  alwaysinline void last_cycle();
  alwaysinline bool interrupt_pending();

  void init();
  void load();
  void unload();
  void power();
  void reset();

  void serialize(serializer&);
  SA1();
};

extern SA1 sa1;
