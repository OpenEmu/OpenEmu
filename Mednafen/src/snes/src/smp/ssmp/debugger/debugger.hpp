class sSMPDebugger : public sSMP {
public:
  function<void ()> step_event;

  enum Usage {
    UsageRead  = 0x80,
    UsageWrite = 0x40,
    UsageExec  = 0x20,
  };
  uint8 *usage;
  uint16 opcode_pc;

  void op_step();
  uint8 op_read(uint16 addr);
  void op_write(uint16 addr, uint8 data);

  sSMPDebugger();
  ~sSMPDebugger();
};
