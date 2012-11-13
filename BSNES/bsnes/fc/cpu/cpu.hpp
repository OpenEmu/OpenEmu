struct CPU : Processor::R6502, Thread {
  uint8 ram[0x0800];

  struct Status {
    bool interrupt_pending;
    bool nmi_pending;
    bool nmi_line;
    bool irq_line;
    bool irq_apu_line;

    bool rdy_line;
    optional<uint16> rdy_addr;

    bool oam_dma_pending;
    uint8 oam_dma_page;

    bool controller_latch;
    unsigned controller_port0;
    unsigned controller_port1;
  } status;

  static void Enter();
  void main();
  void add_clocks(unsigned clocks);

  void power();
  void reset();

  uint8 debugger_read(uint16 addr);

  uint8 ram_read(uint16 addr);
  void ram_write(uint16 addr, uint8 data);

  uint8 read(uint16 addr);
  void write(uint16 addr, uint8 data);

  void serialize(serializer&);

  //timing.cpp
  uint8 op_read(uint16 addr);
  void op_write(uint16 addr, uint8 data);
  void last_cycle();
  void nmi(uint16 &vector);

  void oam_dma();

  void set_nmi_line(bool);
  void set_irq_line(bool);
  void set_irq_apu_line(bool);

  void set_rdy_line(bool);
  void set_rdy_addr(optional<uint16>);
};

extern CPU cpu;
