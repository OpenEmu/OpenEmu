#ifdef SSMP_CPP

alwaysinline uint8 sSMP::ram_read(uint16 addr) {
  if(addr < 0xffc0) return memory::apuram[addr];
  if(status.iplrom_enabled == false) return memory::apuram[addr];
  return iplrom[addr & 0x3f];
}

alwaysinline void sSMP::ram_write(uint16 addr, uint8 data) {
  //writes to $ffc0-$ffff always go to spcram, even if the iplrom is enabled
  memory::apuram[addr] = data;
}

//

alwaysinline uint8 sSMP::port_read(uint8 port) {
  return memory::apuram[0xf4 + (port & 3)];
}

alwaysinline void sSMP::port_write(uint8 port, uint8 data) {
  memory::apuram[0xf4 + (port & 3)] = data;
}

//

alwaysinline uint8 sSMP::op_busread(uint16 addr) {
  uint8 r;
  if((addr & 0xfff0) == 0x00f0) {
    //addr >= 0x00f0 && addr <= 0x00ff
    switch(addr) {
      case 0xf0: { //TEST -- write-only register
        r = 0x00;
      } break;

      case 0xf1: { //CONTROL -- write-only register
        r = 0x00;
      } break;

      case 0xf2: { //DSPADDR
        r = status.dsp_addr;
      } break;

      case 0xf3: { //DSPDATA
        //0x80-0xff are read-only mirrors of 0x00-0x7f
        r = dsp.read(status.dsp_addr & 0x7f);
      } break;

      case 0xf4:   //CPUIO0
      case 0xf5:   //CPUIO1
      case 0xf6:   //CPUIO2
      case 0xf7: { //CPUIO3
        scheduler.sync_smpcpu();
        r = cpu.port_read(addr & 3);
      } break;

      case 0xf8: { //???
        r = status.smp_f8;
      } break;

      case 0xf9: { //???
        r = status.smp_f9;
      } break;

      case 0xfa:   //T0TARGET
      case 0xfb:   //T1TARGET
      case 0xfc: { //T2TARGET -- write-only registers
        r = 0x00;
      } break;

      case 0xfd: { //T0OUT -- 4-bit counter value
        r = t0.stage3_ticks & 15;
        t0.stage3_ticks = 0;
      } break;

      case 0xfe: { //T1OUT -- 4-bit counter value
        r = t1.stage3_ticks & 15;
        t1.stage3_ticks = 0;
      } break;

      case 0xff: { //T2OUT -- 4-bit counter value
        r = t2.stage3_ticks & 15;
        t2.stage3_ticks = 0;
      } break;
    }
  } else {
    r = ram_read(addr);
  }

  return r;
}

alwaysinline void sSMP::op_buswrite(uint16 addr, uint8 data) {
  if((addr & 0xfff0) == 0x00f0) {
    //addr >= 0x00f0 && addr >= 0x00ff
    if(status.mmio_disabled == true) return;

    switch(addr) {
      case 0xf0: { //TEST
        if(regs.p.p) break; //writes only valid when P flag is clear

        //multiplier table may not be 100% accurate, some settings crash
        //the processor due to S-SMP <> S-DSP bus access misalignment
        //static uint8 clock_speed_tbl[16] =
        //{ 3, 5, 9, 17, 4, 6, 10, 18, 6, 8, 12, 20, 10, 12, 16, 24 };

        //status.clock_speed   = 24 * clock_speed_tbl[data >> 4] / 3;
        status.mmio_disabled = !!(data & 0x04);
        status.ram_writable  = !!(data & 0x02);

        //if((data >> 4) != 0) {
          //dprintf("* S-SMP critical warning: $00f0 (TEST) clock speed control modified!");
          //dprintf("* S-SMP may crash on hardware as a result!");
        //}
      } break;

      case 0xf1: { //CONTROL
        status.iplrom_enabled = !!(data & 0x80);

        if(data & 0x30) {
          //one-time clearing of APU port read registers,
          //emulated by simulating CPU writes of 0x00
          scheduler.sync_smpcpu();
          if(data & 0x20) {
            cpu.port_write(2, 0x00);
            cpu.port_write(3, 0x00);
          }
          if(data & 0x10) {
            cpu.port_write(0, 0x00);
            cpu.port_write(1, 0x00);
          }
        }

        //0->1 transistion resets timers
        if(t2.enabled == false && (data & 0x04)) {
          t2.stage2_ticks = 0;
          t2.stage3_ticks = 0;
        }
        t2.enabled = !!(data & 0x04);

        if(t1.enabled == false && (data & 0x02)) {
          t1.stage2_ticks = 0;
          t1.stage3_ticks = 0;
        }
        t1.enabled = !!(data & 0x02);

        if(t0.enabled == false && (data & 0x01)) {
          t0.stage2_ticks = 0;
          t0.stage3_ticks = 0;
        }
        t0.enabled = !!(data & 0x01);
      } break;

      case 0xf2: { //DSPADDR
        status.dsp_addr = data;
      } break;

      case 0xf3: { //DSPDATA
        //0x80-0xff is a read-only mirror of 0x00-0x7f
        if(!(status.dsp_addr & 0x80)) {
          dsp.write(status.dsp_addr & 0x7f, data);
        }
      } break;

      case 0xf4:   //CPUIO0
      case 0xf5:   //CPUIO1
      case 0xf6:   //CPUIO2
      case 0xf7: { //CPUIO3
        scheduler.sync_smpcpu();
        port_write(addr & 3, data);
      } break;

      case 0xf8: { //???
        status.smp_f8 = data;
      } break;

      case 0xf9: { //???
        status.smp_f9 = data;
      } break;

      case 0xfa: { //T0TARGET
        t0.target = data;
      } break;

      case 0xfb: { //T1TARGET
        t1.target = data;
      } break;

      case 0xfc: { //T2TARGET
        t2.target = data;
      } break;

      case 0xfd:   //T0OUT
      case 0xfe:   //T1OUT
      case 0xff: { //T2OUT -- read-only registers
      } break;
    }
  }

  //all writes, even to MMIO registers, appear on bus
  if(status.ram_writable == true) {
    ram_write(addr, data);
  }
}

//

void sSMP::op_io() {
  add_clocks(24);
  tick_timers();
}

uint8 sSMP::op_read(uint16 addr) {
  add_clocks(12);
  uint8 r = op_busread(addr);
  add_clocks(12);
  tick_timers();
  return r;
}

void sSMP::op_write(uint16 addr, uint8 data) {
  add_clocks(24);
  op_buswrite(addr, data);
  tick_timers();
}

#endif
