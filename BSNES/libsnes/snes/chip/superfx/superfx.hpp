class SuperFX : public Coprocessor {
public:
  #include "bus/bus.hpp"
  #include "core/core.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"
  #include "timing/timing.hpp"
  #include "disasm/disasm.hpp"

  static void Enter();
  void enter();
  void init();
  void load();
  void unload();
  void power();
  void reset();
  void serialize(serializer&);

private:
  unsigned clockmode;
  unsigned instruction_counter;
};

extern SuperFX superfx;
