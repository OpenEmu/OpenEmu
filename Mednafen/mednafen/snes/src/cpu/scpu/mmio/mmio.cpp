#ifdef SCPU_CPP

uint8 sCPU::pio() { return status.pio; }
bool sCPU::joylatch() { return status.joypad_strobe_latch; }

//WMDATA
uint8 sCPU::mmio_r2180() {
  uint8 r = bus.read(0x7e0000 | status.wram_addr);
  status.wram_addr = (status.wram_addr + 1) & 0x01ffff;
  return r;
}

//WMDATA
void sCPU::mmio_w2180(uint8 data) {
  bus.write(0x7e0000 | status.wram_addr, data);
  status.wram_addr = (status.wram_addr + 1) & 0x01ffff;
}

//WMADDL
void sCPU::mmio_w2181(uint8 data) {
  status.wram_addr  = (status.wram_addr & 0xffff00) | (data);
  status.wram_addr &= 0x01ffff;
}

//WMADDM
void sCPU::mmio_w2182(uint8 data) {
  status.wram_addr  = (status.wram_addr & 0xff00ff) | (data << 8);
  status.wram_addr &= 0x01ffff;
}

//WMADDH
void sCPU::mmio_w2183(uint8 data) {
  status.wram_addr  = (status.wram_addr & 0x00ffff) | (data << 16);
  status.wram_addr &= 0x01ffff;
}

//JOYSER0
//bit 0 is shared between JOYSER0 and JOYSER1, therefore
//strobing $4016.d0 affects both controller port latches.
//$4017 bit 0 writes are ignored.
void sCPU::mmio_w4016(uint8 data) {
  status.joypad_strobe_latch = !!(data & 1);

  if(status.joypad_strobe_latch == 1) {
    input.poll();
  }
}

//JOYSER0
//7-2 = MDR
//1-0 = Joypad serial data
//
//TODO: test whether strobe latch of zero returns
//realtime or buffered status of joypadN.b
uint8 sCPU::mmio_r4016() {
  uint8 r = regs.mdr & 0xfc;
  r |= input.port_read(0) & 3;
  return r;
}

//JOYSER1
//7-5 = MDR
//4-2 = Always 1 (pins are connected to GND)
//1-0 = Joypad serial data
uint8 sCPU::mmio_r4017() {
  uint8 r = (regs.mdr & 0xe0) | 0x1c;
  r |= input.port_read(1) & 3;
  return r;
}

//NMITIMEN
void sCPU::mmio_w4200(uint8 data) {
  status.auto_joypad_poll = !!(data & 0x01);
  nmitimen_update(data);
}

//WRIO
void sCPU::mmio_w4201(uint8 data) {
  if((status.pio & 0x80) && !(data & 0x80)) {
    ppu.latch_counters();
  }
  status.pio = data;
}

//WRMPYA
void sCPU::mmio_w4202(uint8 data) {
  status.mul_a = data;
}

//WRMPYB
void sCPU::mmio_w4203(uint8 data) {
  status.mul_b = data;
  status.r4216 = status.mul_a * status.mul_b;

  status.alu_lock = true;
  event.enqueue(config.cpu.alu_mul_delay, EventAluLockRelease);
}

//WRDIVL
void sCPU::mmio_w4204(uint8 data) {
  status.div_a = (status.div_a & 0xff00) | (data);
}

//WRDIVH
void sCPU::mmio_w4205(uint8 data) {
  status.div_a = (status.div_a & 0x00ff) | (data << 8);
}

//WRDIVB
void sCPU::mmio_w4206(uint8 data) {
  status.div_b = data;
  status.r4214 = (status.div_b) ? status.div_a / status.div_b : 0xffff;
  status.r4216 = (status.div_b) ? status.div_a % status.div_b : status.div_a;

  status.alu_lock = true;
  event.enqueue(config.cpu.alu_div_delay, EventAluLockRelease);
}

//HTIMEL
void sCPU::mmio_w4207(uint8 data) {
  status.hirq_pos  = (status.hirq_pos & ~0xff) | (data);
  status.hirq_pos &= 0x01ff;
}

//HTIMEH
void sCPU::mmio_w4208(uint8 data) {
  status.hirq_pos  = (status.hirq_pos &  0xff) | (data << 8);
  status.hirq_pos &= 0x01ff;
}

//VTIMEL
void sCPU::mmio_w4209(uint8 data) {
  status.virq_pos  = (status.virq_pos & ~0xff) | (data);
  status.virq_pos &= 0x01ff;
}

//VTIMEH
void sCPU::mmio_w420a(uint8 data) {
  status.virq_pos  = (status.virq_pos &  0xff) | (data << 8);
  status.virq_pos &= 0x01ff;
}

//DMAEN
void sCPU::mmio_w420b(uint8 data) {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].dma_enabled = data & (1 << i);
  }
  if(data) status.dma_pending = true;
}

//HDMAEN
void sCPU::mmio_w420c(uint8 data) {
  for(unsigned i = 0; i < 8; i++) {
    channel[i].hdma_enabled = data & (1 << i);
  }
}

//MEMSEL
void sCPU::mmio_w420d(uint8 data) {
  status.rom_speed = (data & 1 ? 6 : 8);
}

//RDNMI
//7   = NMI acknowledge
//6-4 = MDR
//3-0 = CPU (5a22) version
uint8 sCPU::mmio_r4210() {
  uint8 r = (regs.mdr & 0x70);
  r |= (uint8)(rdnmi()) << 7;
  r |= (cpu_version & 0x0f);
  return r;
}

//TIMEUP
//7   = IRQ acknowledge
//6-0 = MDR
uint8 sCPU::mmio_r4211() {
  uint8 r = (regs.mdr & 0x7f);
  r |= (uint8)(timeup()) << 7;
  return r;
}

//HVBJOY
//7   = VBLANK acknowledge
//6   = HBLANK acknowledge
//5-1 = MDR
//0   = JOYPAD acknowledge
uint8 sCPU::mmio_r4212() {
  uint8 r = (regs.mdr & 0x3e);
  uint16 vs = ppu.overscan() == false ? 225 : 240;

  //auto joypad polling
  if(vcounter() >= vs && vcounter() <= (vs + 2))r |= 0x01;

  //hblank
  if(hcounter() <= 2 || hcounter() >= 1096)r |= 0x40;

  //vblank
  if(vcounter() >= vs)r |= 0x80;

  return r;
}

//RDIO
uint8 sCPU::mmio_r4213() {
  return status.pio;
}

//RDDIVL
uint8 sCPU::mmio_r4214() {
  if(status.alu_lock) return 0;
  return status.r4214;
}

//RDDIVH
uint8 sCPU::mmio_r4215() {
  if(status.alu_lock) return 0;
  return status.r4214 >> 8;
}

//RDMPYL
uint8 sCPU::mmio_r4216() {
  if(status.alu_lock) return 0;
  return status.r4216;
}

//RDMPYH
uint8 sCPU::mmio_r4217() {
  if(status.alu_lock) return 0;
  return status.r4216 >> 8;
}

//TODO: handle reads during joypad polling (v=225-227)
uint8 sCPU::mmio_r4218() { return status.joy1l; } //JOY1L
uint8 sCPU::mmio_r4219() { return status.joy1h; } //JOY1H
uint8 sCPU::mmio_r421a() { return status.joy2l; } //JOY2L
uint8 sCPU::mmio_r421b() { return status.joy2h; } //JOY2H
uint8 sCPU::mmio_r421c() { return status.joy3l; } //JOY3L
uint8 sCPU::mmio_r421d() { return status.joy3h; } //JOY3H
uint8 sCPU::mmio_r421e() { return status.joy4l; } //JOY4L
uint8 sCPU::mmio_r421f() { return status.joy4h; } //JOY4H

//DMAPx
uint8 sCPU::mmio_r43x0(uint8 i) {
  return channel[i].dmap;
}

//BBADx
uint8 sCPU::mmio_r43x1(uint8 i) {
  return channel[i].destaddr;
}

//A1TxL
uint8 sCPU::mmio_r43x2(uint8 i) {
  return channel[i].srcaddr;
}

//A1TxH
uint8 sCPU::mmio_r43x3(uint8 i) {
  return channel[i].srcaddr >> 8;
}

//A1Bx
uint8 sCPU::mmio_r43x4(uint8 i) {
  return channel[i].srcbank;
}

//DASxL
//union { uint16 xfersize; uint16 hdma_iaddr; };
uint8 sCPU::mmio_r43x5(uint8 i) {
  return channel[i].xfersize;
}

//DASxH
//union { uint16 xfersize; uint16 hdma_iaddr; };
uint8 sCPU::mmio_r43x6(uint8 i) {
  return channel[i].xfersize >> 8;
}

//DASBx
uint8 sCPU::mmio_r43x7(uint8 i) {
  return channel[i].hdma_ibank;
}

//A2AxL
uint8 sCPU::mmio_r43x8(uint8 i) {
  return channel[i].hdma_addr;
}

//A2AxH
uint8 sCPU::mmio_r43x9(uint8 i) {
  return channel[i].hdma_addr >> 8;
}

//NTRLx
uint8 sCPU::mmio_r43xa(uint8 i) {
  return channel[i].hdma_line_counter;
}

//???
uint8 sCPU::mmio_r43xb(uint8 i) {
  return channel[i].unknown;
}

//DMAPx
void sCPU::mmio_w43x0(uint8 i, uint8 data) {
  channel[i].dmap          = data;
  channel[i].direction     = !!(data & 0x80);
  channel[i].hdma_indirect = !!(data & 0x40);
  channel[i].reversexfer   = !!(data & 0x10);
  channel[i].fixedxfer     = !!(data & 0x08);
  channel[i].xfermode      = data & 7;
}

//DDBADx
void sCPU::mmio_w43x1(uint8 i, uint8 data) {
  channel[i].destaddr = data;
}

//A1TxL
void sCPU::mmio_w43x2(uint8 i, uint8 data) {
  channel[i].srcaddr = (channel[i].srcaddr & 0xff00) | (data);
}

//A1TxH
void sCPU::mmio_w43x3(uint8 i, uint8 data) {
  channel[i].srcaddr = (channel[i].srcaddr & 0x00ff) | (data << 8);
}

//A1Bx
void sCPU::mmio_w43x4(uint8 i, uint8 data) {
  channel[i].srcbank = data;
}

//DASxL
//union { uint16 xfersize; uint16 hdma_iaddr; };
void sCPU::mmio_w43x5(uint8 i, uint8 data) {
  channel[i].xfersize = (channel[i].xfersize & 0xff00) | (data);
}

//DASxH
//union { uint16 xfersize; uint16 hdma_iaddr; };
void sCPU::mmio_w43x6(uint8 i, uint8 data) {
  channel[i].xfersize = (channel[i].xfersize & 0x00ff) | (data << 8);
}

//DASBx
void sCPU::mmio_w43x7(uint8 i, uint8 data) {
  channel[i].hdma_ibank = data;
}

//A2AxL
void sCPU::mmio_w43x8(uint8 i, uint8 data) {
  channel[i].hdma_addr = (channel[i].hdma_addr & 0xff00) | (data);
}

//A2AxH
void sCPU::mmio_w43x9(uint8 i, uint8 data) {
  channel[i].hdma_addr = (channel[i].hdma_addr & 0x00ff) | (data << 8);
}

//NTRLx
void sCPU::mmio_w43xa(uint8 i, uint8 data) {
  channel[i].hdma_line_counter = data;
}

//???
void sCPU::mmio_w43xb(uint8 i, uint8 data) {
  channel[i].unknown = data;
}

void sCPU::mmio_power() {
}

void sCPU::mmio_reset() {
  //$2181-$2183
  status.wram_addr = 0x000000;

  //$4016-$4017
  status.joypad_strobe_latch = 0;
  status.joypad1_bits = ~0;
  status.joypad2_bits = ~0;

  //$4200
  status.nmi_enabled      = false;
  status.hirq_enabled     = false;
  status.virq_enabled     = false;
  status.auto_joypad_poll = false;

  //$4201
  status.pio = 0xff;

  //$4202-$4203
  status.mul_a = 0xff;
  status.mul_b = 0xff;

  //$4204-$4206
  status.div_a = 0xffff;
  status.div_b = 0xff;

  //$4207-$420a
  status.hirq_pos = 0x01ff;
  status.virq_pos = 0x01ff;

  //$420d
  status.rom_speed = 8;

  //$4214-$4217
  status.r4214 = 0x0000;
  status.r4216 = 0x0000;

  //$4218-$421f
  status.joy1l = 0x00;
  status.joy1h = 0x00;
  status.joy2l = 0x00;
  status.joy2h = 0x00;
  status.joy3l = 0x00;
  status.joy3h = 0x00;
  status.joy4l = 0x00;
  status.joy4h = 0x00;
}

uint8 sCPU::mmio_read(unsigned addr) {
  addr &= 0xffff;

  //APU
  if((addr & 0xffc0) == 0x2140) { //$2140-$217f
    scheduler.sync_cpusmp();
    return smp.port_read(addr & 3);
  }

  //DMA
  if((addr & 0xff80) == 0x4300) { //$4300-$437f
    unsigned i = (addr >> 4) & 7;
    switch(addr & 0xf) {
      case 0x0: return mmio_r43x0(i);
      case 0x1: return mmio_r43x1(i);
      case 0x2: return mmio_r43x2(i);
      case 0x3: return mmio_r43x3(i);
      case 0x4: return mmio_r43x4(i);
      case 0x5: return mmio_r43x5(i);
      case 0x6: return mmio_r43x6(i);
      case 0x7: return mmio_r43x7(i);
      case 0x8: return mmio_r43x8(i);
      case 0x9: return mmio_r43x9(i);
      case 0xa: return mmio_r43xa(i);
      case 0xb: return mmio_r43xb(i);
      case 0xc: return regs.mdr; //unmapped
      case 0xd: return regs.mdr; //unmapped
      case 0xe: return regs.mdr; //unmapped
      case 0xf: return mmio_r43xb(i); //mirror of $43xb
    }
  }

  switch(addr) {
    case 0x2180: return mmio_r2180();
    case 0x4016: return mmio_r4016();
    case 0x4017: return mmio_r4017();
    case 0x4210: return mmio_r4210();
    case 0x4211: return mmio_r4211();
    case 0x4212: return mmio_r4212();
    case 0x4213: return mmio_r4213();
    case 0x4214: return mmio_r4214();
    case 0x4215: return mmio_r4215();
    case 0x4216: return mmio_r4216();
    case 0x4217: return mmio_r4217();
    case 0x4218: return mmio_r4218();
    case 0x4219: return mmio_r4219();
    case 0x421a: return mmio_r421a();
    case 0x421b: return mmio_r421b();
    case 0x421c: return mmio_r421c();
    case 0x421d: return mmio_r421d();
    case 0x421e: return mmio_r421e();
    case 0x421f: return mmio_r421f();
  }

  return regs.mdr;
}

void sCPU::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;

  //APU
  if((addr & 0xffc0) == 0x2140) { //$2140-$217f
    scheduler.sync_cpusmp();
    port_write(addr & 3, data);
    return;
  }

  //DMA
  if((addr & 0xff80) == 0x4300) { //$4300-$437f
    unsigned i = (addr >> 4) & 7;
    switch(addr & 0xf) {
      case 0x0: mmio_w43x0(i, data); return;
      case 0x1: mmio_w43x1(i, data); return;
      case 0x2: mmio_w43x2(i, data); return;
      case 0x3: mmio_w43x3(i, data); return;
      case 0x4: mmio_w43x4(i, data); return;
      case 0x5: mmio_w43x5(i, data); return;
      case 0x6: mmio_w43x6(i, data); return;
      case 0x7: mmio_w43x7(i, data); return;
      case 0x8: mmio_w43x8(i, data); return;
      case 0x9: mmio_w43x9(i, data); return;
      case 0xa: mmio_w43xa(i, data); return;
      case 0xb: mmio_w43xb(i, data); return;
      case 0xc: return; //unmapped
      case 0xd: return; //unmapped
      case 0xe: return; //unmapped
      case 0xf: mmio_w43xb(i, data); return; //mirror of $43xb
    }
  }

  switch(addr) {
    case 0x2180: mmio_w2180(data); return;
    case 0x2181: mmio_w2181(data); return;
    case 0x2182: mmio_w2182(data); return;
    case 0x2183: mmio_w2183(data); return;
    case 0x4016: mmio_w4016(data); return;
    case 0x4017: return; //unmapped
    case 0x4200: mmio_w4200(data); return;
    case 0x4201: mmio_w4201(data); return;
    case 0x4202: mmio_w4202(data); return;
    case 0x4203: mmio_w4203(data); return;
    case 0x4204: mmio_w4204(data); return;
    case 0x4205: mmio_w4205(data); return;
    case 0x4206: mmio_w4206(data); return;
    case 0x4207: mmio_w4207(data); return;
    case 0x4208: mmio_w4208(data); return;
    case 0x4209: mmio_w4209(data); return;
    case 0x420a: mmio_w420a(data); return;
    case 0x420b: mmio_w420b(data); return;
    case 0x420c: mmio_w420c(data); return;
    case 0x420d: mmio_w420d(data); return;
  }
}

#endif
