class sCPUDebugger : public sCPU, public CPUDebugger {
public:
  function<void ()> step_event;

  enum Usage {
    UsageRead  = 0x80,
    UsageWrite = 0x40,
    UsageExec  = 0x20,
    UsageFlagM = 0x02,
    UsageFlagX = 0x01,
  };
  uint8 *usage;
  uint32 opcode_pc;  //points to the current opcode, used to backtrace on read/write breakpoints

  void op_step();
  uint8 op_read(uint32 addr);
  void op_write(uint32 addr, uint8 data);

  sCPUDebugger();
  ~sCPUDebugger();

  //===========
  //CPUDebugger
  //===========

  //internal
  unsigned mdr();

  //$2181-$2183
  unsigned wram_address();

  //$4016
  bool joypad_strobe_latch();

  //$4200
  bool nmi_enable();
  bool hirq_enable();
  bool virq_enable();
  bool auto_joypad_poll();

  //$4201
  unsigned pio_bits();

  //$4202
  unsigned multiplicand();

  //$4203
  unsigned multiplier();

  //$4204-$4205
  unsigned dividend();

  //$4206
  unsigned divisor();

  //$4207-$4208
  unsigned htime();

  //$4209-$420a
  unsigned vtime();

  //$420d
  bool fastrom_enable();
};
