#ifdef SCPU_CPP

void sCPUDebugger::op_step() {
  bool break_event = false;

  usage[regs.pc] &= ~(UsageFlagM | UsageFlagX);
  usage[regs.pc] |= UsageExec | (regs.p.m << 1) | (regs.p.x << 0);
  opcode_pc = regs.pc;

  if(debugger.step_cpu) {
    debugger.break_event = Debugger::CPUStep;
    scheduler.exit(Scheduler::DebuggerEvent);
  } else {
    debugger.breakpoint_test(Debugger::Breakpoint::CPUBus, Debugger::Breakpoint::Exec, regs.pc, 0x00);
  }

  if(step_event) step_event();
  sCPU::op_step();
  scheduler.sync_cpusmp();
}

uint8 sCPUDebugger::op_read(uint32 addr) {
  uint8 data = sCPU::op_read(addr);
  usage[addr] |= UsageRead;
  debugger.breakpoint_test(Debugger::Breakpoint::CPUBus, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void sCPUDebugger::op_write(uint32 addr, uint8 data) {
  sCPU::op_write(addr, data);
  usage[addr] |= UsageWrite;
  usage[addr] &= ~UsageExec;
  debugger.breakpoint_test(Debugger::Breakpoint::CPUBus, Debugger::Breakpoint::Write, addr, data);
}

sCPUDebugger::sCPUDebugger() {
  usage = new uint8[1 << 24]();
  opcode_pc = 0x8000;
}

sCPUDebugger::~sCPUDebugger() {
  delete[] usage;
}

//===========
//CPUDebugger
//===========

//internal
unsigned sCPUDebugger::mdr() { return regs.mdr; }

//$2181-$2183
unsigned sCPUDebugger::wram_address() { return status.wram_addr; }

//$4016
bool sCPUDebugger::joypad_strobe_latch() { return status.joypad_strobe_latch; }

//$4200
bool sCPUDebugger::nmi_enable() { return status.nmi_enabled; }
bool sCPUDebugger::hirq_enable() { return status.hirq_enabled; }
bool sCPUDebugger::virq_enable() { return status.virq_enabled; }
bool sCPUDebugger::auto_joypad_poll() { return status.auto_joypad_poll; }

//$4201
unsigned sCPUDebugger::pio_bits() { return status.pio; }

//$4202
unsigned sCPUDebugger::multiplicand() { return status.mul_a; }

//$4203
unsigned sCPUDebugger::multiplier() { return status.mul_b; }

//$4204-$4205
unsigned sCPUDebugger::dividend() { return status.div_a; }

//$4206
unsigned sCPUDebugger::divisor() { return status.div_b; }

//$4207-$4208
unsigned sCPUDebugger::htime() { return status.hirq_pos; }

//$4209-$420a
unsigned sCPUDebugger::vtime() { return status.virq_pos; }

//$420d
bool sCPUDebugger::fastrom_enable() { return status.rom_speed; }

#endif
