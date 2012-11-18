struct CPU : Processor::R65816, Thread, public PPUcounter {
  uint8 wram[128 * 1024];

  enum : bool { Threaded = true };
  vector<Thread*> coprocessors;
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_smp();
  void synchronize_ppu();
  void synchronize_coprocessors();
  void synchronize_controllers();

  uint8 pio();
  bool joylatch();
  bool interrupt_pending();
  uint8 port_read(uint8 port);
  void port_write(uint8 port, uint8 data);
  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  void op_io();
  uint8 op_read(unsigned addr);
  void op_write(unsigned addr, uint8 data);

  void enter();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);
  CPU();
  ~CPU();

private:
  //cpu
  static void Enter();
  void op_step();

  //timing
  struct QueueEvent {
    enum : unsigned {
      DramRefresh,
      HdmaRun,
    };
  };
  nall::priority_queue<unsigned> queue;
  void queue_event(unsigned id);
  void last_cycle();
  void add_clocks(unsigned clocks);
  void scanline();
  void run_auto_joypad_poll();

  //memory
  unsigned speed(unsigned addr) const;

  //dma
  bool dma_transfer_valid(uint8 bbus, unsigned abus);
  bool dma_addr_valid(unsigned abus);
  uint8 dma_read(unsigned abus);
  void dma_write(bool valid, unsigned addr, uint8 data);
  void dma_transfer(bool direction, uint8 bbus, unsigned abus);
  uint8 dma_bbus(unsigned i, unsigned index);
  unsigned dma_addr(unsigned i);
  unsigned hdma_addr(unsigned i);
  unsigned hdma_iaddr(unsigned i);
  void dma_run();
  bool hdma_active_after(unsigned i);
  void hdma_update(unsigned i);
  void hdma_run();
  void hdma_init();
  void dma_reset();

  //registers
  uint8 port_data[4];

  struct Channel {
    bool dma_enabled;
    bool hdma_enabled;

    bool direction;
    bool indirect;
    bool unused;
    bool reverse_transfer;
    bool fixed_transfer;
    uint8 transfer_mode;

    uint8 dest_addr;
    uint16 source_addr;
    uint8 source_bank;

    union {
      uint16 transfer_size;
      uint16 indirect_addr;
    };

    uint8 indirect_bank;
    uint16 hdma_addr;
    uint8 line_counter;
    uint8 unknown;

    bool hdma_completed;
    bool hdma_do_transfer;
  } channel[8];

  struct Status {
    bool nmi_valid;
    bool nmi_line;
    bool nmi_transition;
    bool nmi_pending;

    bool irq_valid;
    bool irq_line;
    bool irq_transition;
    bool irq_pending;

    bool irq_lock;
    bool hdma_pending;

    unsigned wram_addr;

    bool joypad_strobe_latch;

    bool nmi_enabled;
    bool virq_enabled;
    bool hirq_enabled;
    bool auto_joypad_poll_enabled;

    uint8 pio;

    uint8 wrmpya;
    uint8 wrmpyb;
    uint16 wrdiva;
    uint8 wrdivb;

    uint16 htime;
    uint16 vtime;

    unsigned rom_speed;

    uint16 rddiv;
    uint16 rdmpy;

    uint8 joy1l, joy1h;
    uint8 joy2l, joy2h;
    uint8 joy3l, joy3h;
    uint8 joy4l, joy4h;
  } status;
};

extern CPU cpu;
