class sSMP : public SMP, public SMPcore {
public:
  void enter();
  debugvirtual void op_step();

  #include "memory/memory.hpp"
  #include "timing/timing.hpp"

  struct {

    uint8 opcode;
    bool in_opcode;

    //timing
    uint32 clock_counter;
    uint32 dsp_counter;

    //$00f0
    uint8 clock_speed;
    bool mmio_disabled;
    bool ram_writable;

    //$00f1
    bool iplrom_enabled;

    //$00f2
    uint8 dsp_addr;

    //$00f8,$00f9
    uint8 smp_f8, smp_f9;
  } status;

  //ssmp.cpp
  void power();
  void reset();

  void serialize(serializer&);
  sSMP();
  ~sSMP();

  friend class sSMPDebug;
};

#if defined(DEBUGGER)
  #include "debugger/debugger.hpp"
  extern sSMPDebug smp;
#else
  extern sSMP smp;
#endif
