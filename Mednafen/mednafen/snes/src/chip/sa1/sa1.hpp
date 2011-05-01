#include "bus/bus.hpp"

class SA1 : public CPUcore, public MMIO {
public:
  #include "dma/dma.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"

  struct Status {
    uint8 tick_counter;

    bool interrupt_pending;
    uint16 interrupt_vector;

    uint16 scanlines;
    uint16 vcounter;
    uint16 hcounter;
  } status;

  void enter();
  void interrupt(uint16 vector);
  void tick();

  alwaysinline void trigger_irq();
  alwaysinline void last_cycle();
  alwaysinline bool interrupt_pending();

  void init();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);
  SA1();
};

extern SA1 sa1;
extern SA1Bus sa1bus;
