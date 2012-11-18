struct CPU : Processor::LR35902, Thread, MMIO {
  enum class Interrupt : unsigned {
    Vblank,
    Stat,
    Timer,
    Serial,
    Joypad,
  };

  struct Status {
    unsigned clock;

    //$ff00  JOYP
    bool p15;
    bool p14;
    uint8 joyp;
    uint8 mlt_req;

    //$ff01  SB
    uint8 serial_data;
    unsigned serial_bits;

    //$ff02  SC
    bool serial_transfer;
    bool serial_clock;

    //$ff04  DIV
    uint8 div;

    //$ff05  TIMA
    uint8 tima;

    //$ff06  TMA
    uint8 tma;

    //$ff07  TAC
    bool timer_enable;
    unsigned timer_clock;

    //$ff0f  IF
    bool interrupt_request_joypad;
    bool interrupt_request_serial;
    bool interrupt_request_timer;
    bool interrupt_request_stat;
    bool interrupt_request_vblank;

    //$ff4d  KEY1
    bool speed_double;
    bool speed_switch;

    //$ff51,$ff52  HDMA1,HDMA2
    uint16 dma_source;

    //$ff53,$ff54  HDMA3,HDMA4
    uint16 dma_target;

    //$ff55  HDMA5
    bool dma_mode;
    uint16 dma_length;

    //$ff6c  ???
    uint8 ff6c;

    //$ff70  SVBK
    uint3 wram_bank;

    //$ff72-$ff75  ???
    uint8 ff72;
    uint8 ff73;
    uint8 ff74;
    uint8 ff75;

    //$ffff  IE
    bool interrupt_enable_joypad;
    bool interrupt_enable_serial;
    bool interrupt_enable_timer;
    bool interrupt_enable_stat;
    bool interrupt_enable_vblank;
  } status;

  uint8 wram[32768];  //GB=8192, GBC=32768
  uint8 hram[128];

  static void Main();
  void main();
  void interrupt_raise(Interrupt id);
  void interrupt_test();
  void interrupt_exec(uint16 pc);
  bool stop();
  void power();

  void serialize(serializer&);

  //mmio.cpp
  unsigned wram_addr(uint16 addr) const;
  void mmio_joyp_poll();
  uint8 mmio_read(uint16 addr);
  void mmio_write(uint16 addr, uint8 data);

  //memory.cpp
  void op_io();
  uint8 op_read(uint16 addr);
  void op_write(uint16 addr, uint8 data);
  void cycle_edge();
  uint8 debugger_read(uint16 addr);

  //timing.cpp
  void add_clocks(unsigned clocks);
  void timer_262144hz();
  void timer_65536hz();
  void timer_16384hz();
  void timer_8192hz();
  void timer_4096hz();
  void hblank();
};

extern CPU cpu;
