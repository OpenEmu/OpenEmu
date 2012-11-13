#ifdef CPU_CPP

void CPU::queue_event(unsigned id) {
  switch(id) {
    case QueueEvent::DramRefresh: return add_clocks(40);
    case QueueEvent::HdmaRun: return hdma_run();
  }
}

void CPU::last_cycle() {
  if(status.irq_lock) {
    status.irq_lock = false;
    return;
  }

  if(status.nmi_transition) {
    regs.wai = false;
    status.nmi_transition = false;
    status.nmi_pending = true;
  }

  if(status.irq_transition || regs.irq) {
    regs.wai = false;
    status.irq_transition = false;
    status.irq_pending = !regs.p.i;
  }
}

void CPU::add_clocks(unsigned clocks) {
  if(status.hirq_enabled) {
    if(status.virq_enabled) {
      unsigned cpu_time = vcounter() * 1364 + hcounter();
      unsigned irq_time = status.vtime * 1364 + status.htime * 4;
      unsigned framelines = (system.region() == System::Region::NTSC ? 262 : 312) + field();
      if(cpu_time > irq_time) irq_time += framelines * 1364;
      bool irq_valid = status.irq_valid;
      status.irq_valid = cpu_time <= irq_time && cpu_time + clocks > irq_time;
      if(!irq_valid && status.irq_valid) status.irq_line = true;
    } else {
      unsigned irq_time = status.htime * 4;
      if(hcounter() > irq_time) irq_time += 1364;
      bool irq_valid = status.irq_valid;
      status.irq_valid = hcounter() <= irq_time && hcounter() + clocks > irq_time;
      if(!irq_valid && status.irq_valid) status.irq_line = true;
    }
    if(status.irq_line) status.irq_transition = true;
  } else if(status.virq_enabled) {
    bool irq_valid = status.irq_valid;
    status.irq_valid = vcounter() == status.vtime;
    if(!irq_valid && status.irq_valid) status.irq_line = true;
    if(status.irq_line) status.irq_transition = true;
  } else {
    status.irq_valid = false;
  }

  tick(clocks);
  queue.tick(clocks);
  step(clocks);
}

void CPU::scanline() {
  synchronize_smp();
  synchronize_ppu();
  synchronize_coprocessors();
  system.scanline();

  if(vcounter() == 0) hdma_init();

  queue.enqueue(534, QueueEvent::DramRefresh);

  if(vcounter() <= (ppu.overscan() == false ? 224 : 239)) {
    queue.enqueue(1104 + 8, QueueEvent::HdmaRun);
  }

  bool nmi_valid = status.nmi_valid;
  status.nmi_valid = vcounter() >= (ppu.overscan() == false ? 225 : 240);
  if(!nmi_valid && status.nmi_valid) {
    status.nmi_line = true;
    if(status.nmi_enabled) status.nmi_transition = true;
  } else if(nmi_valid && !status.nmi_valid) {
    status.nmi_line = false;
  }

  if(status.auto_joypad_poll_enabled && vcounter() == (ppu.overscan() == false ? 227 : 242)) {
    run_auto_joypad_poll();
  }
}

void CPU::run_auto_joypad_poll() {
  input.port1->latch(1);
  input.port2->latch(1);
  input.port1->latch(0);
  input.port2->latch(0);

  uint16 joy1 = 0, joy2 = 0, joy3 = 0, joy4 = 0;
  for(unsigned i = 0; i < 16; i++) {
    uint8 port0 = input.port1->data();
    uint8 port1 = input.port2->data();

    joy1 |= (port0 & 1) ? (0x8000 >> i) : 0;
    joy2 |= (port1 & 1) ? (0x8000 >> i) : 0;
    joy3 |= (port0 & 2) ? (0x8000 >> i) : 0;
    joy4 |= (port1 & 2) ? (0x8000 >> i) : 0;
  }

  status.joy1l = joy1;
  status.joy1h = joy1 >> 8;

  status.joy2l = joy2;
  status.joy2h = joy2 >> 8;

  status.joy3l = joy3;
  status.joy3h = joy3 >> 8;

  status.joy4l = joy4;
  status.joy4h = joy4 >> 8;
}

#endif
