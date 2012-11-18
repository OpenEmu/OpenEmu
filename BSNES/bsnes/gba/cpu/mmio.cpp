uint8 CPU::read(uint32 addr) {
  uint8 result = 0;

  switch(addr) {

  //DMA0CNT_H
  //DMA1CNT_H
  //DMA2CNT_H
  //DMA3CNT_H
  case 0x040000ba: case 0x040000bb:
  case 0x040000c6: case 0x040000c7:
  case 0x040000d2: case 0x040000d3:
  case 0x040000de: case 0x040000df: {
    auto &dma = regs.dma[(addr - 0x040000ba) / 12];
    unsigned shift = (addr & 1) * 8;
    return dma.control >> shift;
  }

  //TM0CNT_L
  //TM1CNT_L
  //TM2CNT_L
  //TM3CNT_L
  case 0x04000100: case 0x04000101:
  case 0x04000104: case 0x04000105:
  case 0x04000108: case 0x04000109:
  case 0x0400010c: case 0x0400010d: {
    auto &timer = regs.timer[(addr >> 2) & 3];
    unsigned shift = (addr & 1) * 8;
    return timer.period >> shift;
  }

  //TIM0CNT_H
  case 0x04000102: case 0x04000103:
  case 0x04000106: case 0x04000107:
  case 0x0400010a: case 0x0400010b:
  case 0x0400010e: case 0x0400010f: {
    auto &timer = regs.timer[(addr >> 2) & 3];
    unsigned shift = (addr & 1) * 8;
    return timer.control >> shift;
  }

  //SIOMULTI0 (SIODATA32_L)
  //SIOMULTI1 (SIODATA32_H)
  //SIOMULTI2
  //SIOMULTI3
  case 0x04000120: case 0x04000121:
  case 0x04000122: case 0x04000123:
  case 0x04000124: case 0x04000125:
  case 0x04000126: case 0x04000127: {
    auto &data = regs.serial.data[(addr >> 1) & 3];
    unsigned shift = (addr & 1) * 8;
    return data >> shift;
  }

  //SIOCNT
  case 0x04000128: return regs.serial.control >> 0;
  case 0x04000129: return regs.serial.control >> 8;

  //SIOMLT_SEND (SIODATA8)
  case 0x0400012a: return regs.serial.data8;
  case 0x0400012b: return 0u;

  //KEYINPUT
  case 0x04000130:
    for(unsigned n = 0; n < 8; n++) result |= interface->inputPoll(0, 0, n) << n;
    if((result & 0xc0) == 0xc0) result &= ~0xc0;  //up+down cannot be pressed simultaneously
    if((result & 0x30) == 0x30) result &= ~0x30;  //left+right cannot be pressed simultaneously
    return result ^ 0xff;
  case 0x04000131:
    result |= interface->inputPoll(0, 0, 8) << 0;
    result |= interface->inputPoll(0, 0, 9) << 1;
    return result ^ 0x03;

  //KEYCNT
  case 0x04000132: return regs.keypad.control >> 0;
  case 0x04000133: return regs.keypad.control >> 8;

  //RCNT
  case 0x04000134: return regs.joybus.settings >> 0;
  case 0x04000135: return regs.joybus.settings >> 8;

  //JOYCNT
  case 0x04000140: return regs.joybus.control >> 0;
  case 0x04000141: return regs.joybus.control >> 8;

  //JOY_RECV_L
  //JOY_RECV_H
  case 0x04000150: return regs.joybus.receive >>  0;
  case 0x04000151: return regs.joybus.receive >>  8;
  case 0x04000152: return regs.joybus.receive >> 16;
  case 0x04000153: return regs.joybus.receive >> 24;

  //JOY_TRANS_L
  //JOY_TRANS_H
  case 0x04000154: return regs.joybus.transmit >>  0;
  case 0x04000155: return regs.joybus.transmit >>  8;
  case 0x04000156: return regs.joybus.transmit >> 16;
  case 0x04000157: return regs.joybus.transmit >> 24;

  //JOYSTAT
  case 0x04000158: return regs.joybus.status >> 0;
  case 0x04000159: return regs.joybus.status >> 8;

  //IE
  case 0x04000200: return regs.irq.enable >> 0;
  case 0x04000201: return regs.irq.enable >> 8;

  //IF
  case 0x04000202: return regs.irq.flag >> 0;
  case 0x04000203: return regs.irq.flag >> 8;

  //WAITCNT
  case 0x04000204: return regs.wait.control >> 0;
  case 0x04000205: return regs.wait.control >> 8;

  //IME
  case 0x04000208: return regs.ime;
  case 0x04000209: return 0u;

  //POSTFLG + HALTCNT
  case 0x04000300: return regs.postboot;
  case 0x04000301: return 0u;

  //MEMCNT_L
  case 0x04000800: return regs.memory.control >>  0;
  case 0x04000801: return regs.memory.control >>  8;

  //MEMCNT_H
  case 0x04000802: return regs.memory.control >> 16;
  case 0x04000803: return regs.memory.control >> 24;

  }

  return 0u;
}

void CPU::write(uint32 addr, uint8 byte) {
  switch(addr) {

  //DMA0SAD
  //DMA1SAD
  //DMA2SAD
  //DMA3SAD
  case 0x040000b0: case 0x040000b1: case 0x040000b2: case 0x040000b3:
  case 0x040000bc: case 0x040000bd: case 0x040000be: case 0x040000bf:
  case 0x040000c8: case 0x040000c9: case 0x040000ca: case 0x040000cb:
  case 0x040000d4: case 0x040000d5: case 0x040000d6: case 0x040000d7: {
    auto &dma = regs.dma[(addr - 0x040000b0) / 12];
    unsigned shift = (addr & 3) * 8;
    dma.source = (dma.source & ~(255 << shift)) | (byte << shift);
    return;
  }

  //DMA0DAD
  //DMA1DAD
  //DMA2DAD
  //DMA3DAD
  case 0x040000b4: case 0x040000b5: case 0x040000b6: case 0x040000b7:
  case 0x040000c0: case 0x040000c1: case 0x040000c2: case 0x040000c3:
  case 0x040000cc: case 0x040000cd: case 0x040000ce: case 0x040000cf:
  case 0x040000d8: case 0x040000d9: case 0x040000da: case 0x040000db: {
    auto &dma = regs.dma[(addr - 0x040000b4) / 12];
    unsigned shift = (addr & 3) * 8;
    dma.target = (dma.target & ~(255 << shift)) | (byte << shift);
    return;
  }

  //DMA0CNT_L
  //DMA1CNT_L
  //DMA2CNT_L
  //DMA3CNT_L
  case 0x040000b8: case 0x040000b9:
  case 0x040000c4: case 0x040000c5:
  case 0x040000d0: case 0x040000d1:
  case 0x040000dc: case 0x040000dd: {
    auto &dma = regs.dma[(addr - 0x040000b8) / 12];
    unsigned shift = (addr & 1) * 8;
    dma.length = (dma.length & ~(255 << shift)) | (byte << shift);
    return;
  }

  //DMA0CNT_H
  //DMA1CNT_H
  //DMA2CNT_H
  //DMA3CNT_H
  case 0x040000ba: case 0x040000bb:
  case 0x040000c6: case 0x040000c7:
  case 0x040000d2: case 0x040000d3:
  case 0x040000de: case 0x040000df: {
    auto &dma = regs.dma[(addr - 0x040000ba) / 12];
    unsigned shift = (addr & 1) * 8;
    bool enable = dma.control.enable;
    dma.control = (dma.control & ~(255 << shift)) | (byte << shift);
    if(enable == 0 && dma.control.enable) {
      if(dma.control.timingmode == 0) dma.pending = true;  //immediate transfer mode
      dma.run.target = dma.target;
      dma.run.source = dma.source;
      dma.run.length = dma.length;
    } else if(dma.control.enable == 0) {
      dma.pending = false;
    }
    return;
  }

  //TM0CNT_L
  //TM1CNT_L
  //TM2CNT_L
  //TM3CNT_L
  case 0x04000100: case 0x04000101:
  case 0x04000104: case 0x04000105:
  case 0x04000108: case 0x04000109:
  case 0x0400010c: case 0x0400010d: {
    auto &timer = regs.timer[(addr >> 2) & 3];
    unsigned shift = (addr & 1) * 8;
    timer.reload = (timer.reload & ~(255 << shift)) | (byte << shift);
    return;
  }

  //TM0CNT_H
  //TM1CNT_H
  //TM2CNT_H
  //TM3CNT_H
  case 0x04000102:
  case 0x04000106:
  case 0x0400010a:
  case 0x0400010e: {
    auto &timer = regs.timer[(addr >> 2) & 3];
    bool enable = timer.control.enable;
    timer.control = byte;
    if(enable == 0 && timer.control.enable == 1) {
      timer.period = timer.reload;
    }
    return;
  }

  //SIOMULTI0 (SIODATA32_L)
  //SIOMULTI1 (SIODATA32_H)
  //SIOMULTI2
  //SIOMULTI3
  case 0x04000120: case 0x04000121:
  case 0x04000122: case 0x04000123:
  case 0x04000124: case 0x04000125:
  case 0x04000126: case 0x04000127: {
    auto &data = regs.serial.data[(addr >> 1) & 3];
    unsigned shift = (addr & 1) * 8;
    data = (data & ~(255 << shift)) | (byte << shift);
    return;
  }

  //SIOCNT
  case 0x04000128: regs.serial.control = (regs.serial.control & 0xff00) | (byte << 0); return;
  case 0x04000129: regs.serial.control = (regs.serial.control & 0x00ff) | (byte << 8); return;

  //SIOMLT_SEND (SIODATA8)
  case 0x0400012a: regs.serial.data8 = byte; return;
  case 0x0400012b: return;

  //KEYCNT
  case 0x04000132: regs.keypad.control = (regs.keypad.control & 0xff00) | (byte << 0); return;
  case 0x04000133: regs.keypad.control = (regs.keypad.control & 0x00ff) | (byte << 8); return;

  //RCNT
  case 0x04000134: regs.joybus.settings = (regs.joybus.settings & 0xff00) | (byte << 0); return;
  case 0x04000135: regs.joybus.settings = (regs.joybus.settings & 0x00ff) | (byte << 8); return;

  //JOYCNT
  case 0x04000140: regs.joybus.control = (regs.joybus.control & 0xff00) | (byte << 0); return;
  case 0x04000141: regs.joybus.control = (regs.joybus.control & 0x00ff) | (byte << 8); return;

  //JOY_RECV_L
  //JOY_RECV_H
  case 0x04000150: regs.joybus.receive = (regs.joybus.receive & 0xffffff00) | (byte <<  0); return;
  case 0x04000151: regs.joybus.receive = (regs.joybus.receive & 0xffff00ff) | (byte <<  8); return;
  case 0x04000152: regs.joybus.receive = (regs.joybus.receive & 0xff00ffff) | (byte << 16); return;
  case 0x04000153: regs.joybus.receive = (regs.joybus.receive & 0x00ffffff) | (byte << 24); return;

  //JOY_TRANS_L
  //JOY_TRANS_H
  case 0x04000154: regs.joybus.transmit = (regs.joybus.transmit & 0xffffff00) | (byte <<  0); return;
  case 0x04000155: regs.joybus.transmit = (regs.joybus.transmit & 0xffff00ff) | (byte <<  8); return;
  case 0x04000156: regs.joybus.transmit = (regs.joybus.transmit & 0xff00ffff) | (byte << 16); return;
  case 0x04000157: regs.joybus.transmit = (regs.joybus.transmit & 0x00ffffff) | (byte << 24); return;

  //JOYSTAT
  case 0x04000158: regs.joybus.status = (regs.joybus.status & 0xff00) | (byte << 0); return;
  case 0x04000159: regs.joybus.status = (regs.joybus.status & 0x00ff) | (byte << 8); return;

  //IE
  case 0x04000200: regs.irq.enable = (regs.irq.enable & 0xff00) | (byte << 0); return;
  case 0x04000201: regs.irq.enable = (regs.irq.enable & 0x00ff) | (byte << 8); return;

  //IF
  case 0x04000202: regs.irq.flag = regs.irq.flag & ~(byte << 0); return;
  case 0x04000203: regs.irq.flag = regs.irq.flag & ~(byte << 8); return;

  //WAITCNT
  case 0x04000204: regs.wait.control = (regs.wait.control & 0xff00) | ((byte & 0xff) << 0); return;
  case 0x04000205: regs.wait.control = (regs.wait.control & 0x00ff) | ((byte & 0x7f) << 8); return;

  //IME
  case 0x04000208: regs.ime = byte >> 0; return;
  case 0x04000209: return;

  //POSTFLG, HALTCNT
  case 0x04000300: regs.postboot |= byte >> 0; return;
  case 0x04000301: regs.mode = byte & 0x80 ? Registers::Mode::Stop : Registers::Mode::Halt; return;

  //MEMCNT_L
  //MEMCNT_H
  case 0x04000800: regs.memory.control = (regs.memory.control & 0xffffff00) | (byte <<  0); return;
  case 0x04000801: regs.memory.control = (regs.memory.control & 0xffff00ff) | (byte <<  8); return;
  case 0x04000802: regs.memory.control = (regs.memory.control & 0xff00ffff) | (byte << 16); return;
  case 0x04000803: regs.memory.control = (regs.memory.control & 0x00ffffff) | (byte << 24); return;

  }
}
