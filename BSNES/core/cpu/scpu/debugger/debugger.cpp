#ifdef SCPU_CPP

void sCPUDebug::op_step() {
  bool break_event = false;

  if(debugger.step_cpu) {
    debugger.break_event = Debugger::CPUStep;
    scheduler.exit();
  } else {
    debugger.breakpoint_test(Debugger::Breakpoint::CPUBus, Debugger::Breakpoint::Exec, regs.pc, 0x00);
  }

  if(debugger.trace_cpu) {
    char t[256];
    disassemble_opcode(t);
    debugger.tracefile.print(string() << t << "\n");
  }

  sCPU::op_step();
  scheduler.sync_cpusmp();
}

uint8 sCPUDebug::op_read(uint32 addr) {
  uint8 data = sCPU::op_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::CPUBus, Debugger::Breakpoint::Read, addr, data);
  return data;
}

void sCPUDebug::op_write(uint32 addr, uint8 data) {
  sCPU::op_write(addr, data);
  debugger.breakpoint_test(Debugger::Breakpoint::CPUBus, Debugger::Breakpoint::Write, addr, data);
}

#endif
