struct CPU : Processor::R65816, Thread, public PPUcounter {
  uint8 wram[128 * 1024];

  enum : bool { Threaded = true };
  vector<Thread*> coprocessors;
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_smp();
  void synchronize_ppu();
  void synchronize_coprocessors();
  void synchronize_controllers();

  uint8 port_read(uint2 port) const;
  void port_write(uint2 port, uint8 data);

  uint8 pio();
  bool joylatch();
  alwaysinline bool interrupt_pending() { return status.interrupt_pending; }

  void enter();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);
  CPU();
  ~CPU();

privileged:
  #include "dma/dma.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"
  #include "timing/timing.hpp"

  uint8 cpu_version;

  struct Status {
    bool interrupt_pending;

    unsigned clock_count;
    unsigned line_clocks;

    //timing
    bool irq_lock;

    unsigned dram_refresh_position;
    bool dram_refreshed;

    unsigned hdma_init_position;
    bool hdma_init_triggered;

    unsigned hdma_position;
    bool hdma_triggered;

    bool nmi_valid;
    bool nmi_line;
    bool nmi_transition;
    bool nmi_pending;
    bool nmi_hold;

    bool irq_valid;
    bool irq_line;
    bool irq_transition;
    bool irq_pending;
    bool irq_hold;

    bool reset_pending;

    //DMA
    bool dma_active;
    unsigned dma_counter;
    unsigned dma_clocks;
    bool dma_pending;
    bool hdma_pending;
    bool hdma_mode;  //0 = init, 1 = run

    //auto joypad polling
    bool auto_joypad_active;
    bool auto_joypad_latch;
    unsigned auto_joypad_counter;
    unsigned auto_joypad_clock;

    //MMIO
    //$2140-217f
    uint8 port[4];

    //$2181-$2183
    uint17 wram_addr;

    //$4016-$4017
    bool joypad_strobe_latch;
    uint32 joypad1_bits;
    uint32 joypad2_bits;

    //$4200
    bool nmi_enabled;
    bool hirq_enabled, virq_enabled;
    bool auto_joypad_poll;

    //$4201
    uint8 pio;

    //$4202-$4203
    uint8 wrmpya;
    uint8 wrmpyb;

    //$4204-$4206
    uint16 wrdiva;
    uint8 wrdivb;

    //$4207-$420a
    uint9 hirq_pos;
    uint9 virq_pos;

    //$420d
    unsigned rom_speed;

    //$4214-$4217
    uint16 rddiv;
    uint16 rdmpy;

    //$4218-$421f
    uint16 joy1;
    uint16 joy2;
    uint16 joy3;
    uint16 joy4;
  } status;

  struct ALU {
    unsigned mpyctr;
    unsigned divctr;
    unsigned shift;
  } alu;

  static void Enter();
  void op_step();

  struct Debugger {
    hook<void (uint24)> op_exec;
    hook<void (uint24)> op_read;
    hook<void (uint24, uint8)> op_write;
    hook<void ()> op_nmi;
    hook<void ()> op_irq;
  } debugger;
};

extern CPU cpu;
