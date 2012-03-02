#ifdef SMP_CPP

void SMPDebugger::op_step() {
  bool break_event = false;

  usage[regs.pc] |= UsageExec;
  opcode_pc = regs.pc;

  opcode_edge = true;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::APURAM, Debugger::Breakpoint::Mode::Exec, regs.pc, 0x00);
  if(step_event && step_event() == true) {
    debugger.break_event = Debugger::BreakEvent::SMPStep;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  }
  opcode_edge = false;

  SMP::op_step();
  synchronize_cpu();
}

uint8 SMPDebugger::op_read(uint16 addr) {
  uint8 data = SMP::op_read(addr);
  usage[addr] |= UsageRead;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::APURAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void SMPDebugger::op_write(uint16 addr, uint8 data) {
  SMP::op_write(addr, data);
  usage[addr] |= UsageWrite;
  usage[addr] &= ~UsageExec;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::APURAM, Debugger::Breakpoint::Mode::Write, addr, data);
}

SMPDebugger::SMPDebugger() {
  usage = new uint8[1 << 16]();
  opcode_pc = 0xffc0;
  opcode_edge = false;
}

SMPDebugger::~SMPDebugger() {
  delete[] usage;
}

bool SMPDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  //$00f0
  item("$00f0", "");
  item("Clock Speed", (unsigned)status.clock_speed);
  item("Timers Enable", status.timers_enable);
  item("RAM Disable", status.ram_disable);
  item("RAM Writable", status.ram_writable);
  item("Timers Disable", status.timers_disable);

  //$00f1
  item("$00f1", "");
  item("IPLROM Enable", status.iplrom_enable);

  //$00f2
  item("$00f2", "");
  item("DSP Address", string("0x", hex<2>(status.dsp_addr)));

  #undef item
  return false;
}

#endif
