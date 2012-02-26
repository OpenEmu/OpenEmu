class SMPDebugger : public SMP, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);

  function<bool ()> step_event;

  enum Usage {
    UsageRead  = 0x80,
    UsageWrite = 0x40,
    UsageExec  = 0x20,
  };
  uint8 *usage;
  uint16 opcode_pc;
  bool opcode_edge;

  void op_step();
  uint8 op_read(uint16 addr);
  void op_write(uint16 addr, uint8 data);

  SMPDebugger();
  ~SMPDebugger();
};
