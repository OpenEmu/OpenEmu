#ifdef CPU_CPP

void CPUDebugger::op_step() {
  bool break_event = false;

  usage[regs.pc] &= ~(UsageFlagM | UsageFlagX);
  usage[regs.pc] |= UsageExec | (regs.p.m << 1) | (regs.p.x << 0);
  opcode_pc = regs.pc;

  opcode_edge = true;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Exec, regs.pc, 0x00);
  if(step_event && step_event()) {
    debugger.break_event = Debugger::BreakEvent::CPUStep;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  }
  opcode_edge = false;

  CPU::op_step();
  synchronize_smp();
}

uint8 CPUDebugger::op_read(uint32 addr) {
  uint8 data = CPU::op_read(addr);
  usage[addr] |= UsageRead;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void CPUDebugger::op_write(uint32 addr, uint8 data) {
  CPU::op_write(addr, data);
  usage[addr] |= UsageWrite;
  usage[addr] &= ~UsageExec;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Write, addr, data);
}

CPUDebugger::CPUDebugger() {
  usage = new uint8[1 << 24]();
  opcode_pc = 0x8000;
  opcode_edge = false;
}

CPUDebugger::~CPUDebugger() {
  delete[] usage;
}

bool CPUDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  //internal
  item("S-CPU MDR", string("0x", hex<2>(regs.mdr)));

  //$2181-2183
  item("$2181-$2183", "");
  item("WRAM Address", string("0x", hex<6>(status.wram_addr)));

  //$4016
  item("$4016", "");
  item("Joypad Strobe Latch", status.joypad_strobe_latch);

  //$4200
  item("$4200", "");
  item("NMI Enable", status.nmi_enabled);
  item("H-IRQ Enable", status.hirq_enabled);
  item("V-IRQ Enable", status.virq_enabled);
  item("Auto Joypad Poll", status.auto_joypad_poll_enabled);

  //$4201
  item("$4201", "");
  item("PIO", string("0x", hex<2>(status.pio)));

  //$4202
  item("$4202", "");
  item("Multiplicand", string("0x", hex<2>(status.wrmpya)));

  //$4203
  item("$4203", "");
  item("Multiplier", string("0x", hex<2>(status.wrmpyb)));

  //$4204-$4205
  item("$4204-$4205", "");
  item("Dividend", string("0x", hex<4>(status.wrdiva)));

  //$4206
  item("$4206", "");
  item("Divisor", string("0x", hex<2>(status.wrdivb)));

  //$4207-$4208
  item("$4207-$4208", "");
  item("H-Time", string("0x", hex<4>(status.htime)));

  //$4209-$420a
  item("$4209-$420a", "");
  item("V-Time", string("0x", hex<4>(status.vtime)));

  //$420b
  unsigned dma_enable = 0;
  for(unsigned n = 0; n < 8; n++) dma_enable |= channel[n].dma_enabled << n;

  item("$420b", "");
  item("DMA Enable", string("0x", hex<2>(dma_enable)));

  //$420c
  unsigned hdma_enable = 0;
  for(unsigned n = 0; n < 8; n++) hdma_enable |= channel[n].hdma_enabled << n;

  item("$420c", "");
  item("HDMA Enable", string("0x", hex<2>(hdma_enable)));

  //$420d
  item("$420d", "");
  item("FastROM Enable", status.rom_speed == 6);

  for(unsigned i = 0; i < 8; i++) {
    item(string("DMA Channel ", i), "");

    //$43x0
    item("Direction", channel[i].direction);
    item("Indirect", channel[i].indirect);
    item("Reverse Transfer", channel[i].reverse_transfer);
    item("Fixed Transfer", channel[i].fixed_transfer);
    item("Transfer Mode", (unsigned)channel[i].transfer_mode);

    //$43x1
    item("B-Bus Address", string("0x", hex<4>(channel[i].dest_addr)));

    //$43x2-$43x3
    item("A-Bus Address", string("0x", hex<4>(channel[i].source_addr)));

    //$43x4
    item("A-Bus Bank", string("0x", hex<2>(channel[i].source_bank)));

    //$43x5-$43x6
    item("Transfer Size / Indirect Address", string("0x", hex<4>(channel[i].transfer_size)));

    //$43x7
    item("Indirect Bank", string("0x", hex<2>(channel[i].indirect_bank)));

    //$43x8-$43x9
    item("Table Address", string("0x", hex<4>(channel[i].hdma_addr)));

    //$43xa
    item("Line Counter", string("0x", hex<2>(channel[i].line_counter)));
  }

  #undef item
  return false;
}

#endif
