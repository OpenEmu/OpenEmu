#ifdef SSMP_CPP

void sSMPDebug::op_step() {
  bool break_event = false;

  if(debugger.step_smp) {
    debugger.break_event = Debugger::SMPStep;
    scheduler.exit();
  } else {
    debugger.breakpoint_test(Debugger::Breakpoint::APURAM, Debugger::Breakpoint::Exec, regs.pc, 0x00);
  }

  if(debugger.trace_smp) {
    char t[256];
    disassemble_opcode(t);
    debugger.tracefile.print(string() << t << "\n");
  }

  sSMP::op_step();
  scheduler.sync_smpcpu();
}

#endif
