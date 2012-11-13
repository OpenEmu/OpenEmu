void CPU::serialize(serializer &s) {
  ARM::serialize(s);
  Thread::serialize(s);

  s.array(iwram,  32 * 1024);
  s.array(ewram, 256 * 1024);

  for(auto &dma : regs.dma) {
    s.integer(dma.source);
    s.integer(dma.target);
    s.integer(dma.length);
    s.integer(dma.control.targetmode);
    s.integer(dma.control.sourcemode);
    s.integer(dma.control.repeat);
    s.integer(dma.control.size);
    s.integer(dma.control.drq);
    s.integer(dma.control.timingmode);
    s.integer(dma.control.irq);
    s.integer(dma.control.enable);
    s.integer(dma.run.target);
    s.integer(dma.run.source);
    s.integer(dma.run.length);
  }

  for(auto &timer : regs.timer) {
    s.integer(timer.period);
    s.integer(timer.reload);
    s.integer(timer.control.frequency);
    s.integer(timer.control.cascade);
    s.integer(timer.control.irq);
    s.integer(timer.control.enable);
  }

  for(auto &value : regs.serial.data) s.integer(value);
  s.integer(regs.serial.control.shiftclockselect);
  s.integer(regs.serial.control.shiftclockfrequency);
  s.integer(regs.serial.control.transferenablereceive);
  s.integer(regs.serial.control.transferenablesend);
  s.integer(regs.serial.control.startbit);
  s.integer(regs.serial.control.transferlength);
  s.integer(regs.serial.control.irqenable);
  s.integer(regs.serial.data8);

  for(auto &flag : regs.keypad.control.flag) s.integer(flag);
  s.integer(regs.keypad.control.enable);
  s.integer(regs.keypad.control.condition);

  s.integer(regs.joybus.settings.sc);
  s.integer(regs.joybus.settings.sd);
  s.integer(regs.joybus.settings.si);
  s.integer(regs.joybus.settings.so);
  s.integer(regs.joybus.settings.scmode);
  s.integer(regs.joybus.settings.sdmode);
  s.integer(regs.joybus.settings.simode);
  s.integer(regs.joybus.settings.somode);
  s.integer(regs.joybus.settings.irqenable);
  s.integer(regs.joybus.settings.mode);

  s.integer(regs.joybus.control.resetsignal);
  s.integer(regs.joybus.control.receivecomplete);
  s.integer(regs.joybus.control.sendcomplete);
  s.integer(regs.joybus.control.irqenable);

  s.integer(regs.joybus.receive);
  s.integer(regs.joybus.transmit);

  s.integer(regs.joybus.status.receiveflag);
  s.integer(regs.joybus.status.sendflag);
  s.integer(regs.joybus.status.generalflag);

  s.integer(regs.ime);

  s.integer(regs.irq.enable.vblank);
  s.integer(regs.irq.enable.hblank);
  s.integer(regs.irq.enable.vcoincidence);
  for(auto &flag : regs.irq.enable.timer) s.integer(flag);
  s.integer(regs.irq.enable.serial);
  for(auto &flag : regs.irq.enable.dma) s.integer(flag);
  s.integer(regs.irq.enable.keypad);
  s.integer(regs.irq.enable.cartridge);

  s.integer(regs.irq.flag.vblank);
  s.integer(regs.irq.flag.hblank);
  s.integer(regs.irq.flag.vcoincidence);
  for(auto &flag : regs.irq.flag.timer) s.integer(flag);
  s.integer(regs.irq.flag.serial);
  for(auto &flag : regs.irq.flag.dma) s.integer(flag);
  s.integer(regs.irq.flag.keypad);
  s.integer(regs.irq.flag.cartridge);

  for(auto &flag : regs.wait.control.nwait) s.integer(flag);
  for(auto &flag : regs.wait.control.swait) s.integer(flag);
  s.integer(regs.wait.control.phi);
  s.integer(regs.wait.control.prefetch);
  s.integer(regs.wait.control.gametype);

  s.integer(regs.memory.control.disable);
  s.integer(regs.memory.control.unknown1);
  s.integer(regs.memory.control.ewram);
  s.integer(regs.memory.control.ewramwait);
  s.integer(regs.memory.control.unknown2);

  s.integer(regs.postboot);
  s.integer((unsigned&)regs.mode);
  s.integer(regs.clock);

  s.integer(pending.dma.vblank);
  s.integer(pending.dma.hblank);
  s.integer(pending.dma.hdma);
}
