#include <gb/gb.hpp>

#define CPU_CPP
namespace GameBoy {

#include "mmio.cpp"
#include "memory.cpp"
#include "timing.cpp"
#include "serialization.cpp"
CPU cpu;

void CPU::Main() {
  cpu.main();
}

void CPU::main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::CPU) {
      scheduler.sync = Scheduler::SynchronizeMode::All;
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    interrupt_test();
    exec();
  }
}

void CPU::interrupt_raise(CPU::Interrupt id) {
  if(id == Interrupt::Vblank) {
    status.interrupt_request_vblank = 1;
    if(status.interrupt_enable_vblank) r.halt = false;
  }

  if(id == Interrupt::Stat) {
    status.interrupt_request_stat = 1;
    if(status.interrupt_enable_stat) r.halt = false;
  }

  if(id == Interrupt::Timer) {
    status.interrupt_request_timer = 1;
    if(status.interrupt_enable_timer) r.halt = false;
  }

  if(id == Interrupt::Serial) {
    status.interrupt_request_serial = 1;
    if(status.interrupt_enable_serial) r.halt = false;
  }

  if(id == Interrupt::Joypad) {
    status.interrupt_request_joypad = 1;
    if(status.interrupt_enable_joypad) r.halt = r.stop = false;
  }
}

void CPU::interrupt_test() {
  if(r.ime) {
    if(status.interrupt_request_vblank && status.interrupt_enable_vblank) {
      status.interrupt_request_vblank = 0;
      return interrupt_exec(0x0040);
    }

    if(status.interrupt_request_stat && status.interrupt_enable_stat) {
      status.interrupt_request_stat = 0;
      return interrupt_exec(0x0048);
    }

    if(status.interrupt_request_timer && status.interrupt_enable_timer) {
      status.interrupt_request_timer = 0;
      return interrupt_exec(0x0050);
    }

    if(status.interrupt_request_serial && status.interrupt_enable_serial) {
      status.interrupt_request_serial = 0;
      return interrupt_exec(0x0058);
    }

    if(status.interrupt_request_joypad && status.interrupt_enable_joypad) {
      status.interrupt_request_joypad = 0;
      return interrupt_exec(0x0060);
    }
  }
}

void CPU::interrupt_exec(uint16 pc) {
  r.ime = 0;
  op_write(--r[SP], r[PC] >> 8);
  op_write(--r[SP], r[PC] >> 0);
  r[PC] = pc;
  op_io();
  op_io();
  op_io();
}

bool CPU::stop() {
  if(status.speed_switch) {
    status.speed_switch = 0;
    status.speed_double ^= 1;
    if(status.speed_double == 0) frequency = 4 * 1024 * 1024;
    if(status.speed_double == 1) frequency = 8 * 1024 * 1024;
    return true;
  }
  return false;
}

void CPU::power() {
  create(Main, 4 * 1024 * 1024);
  LR35902::power();

  for(unsigned n = 0xc000; n <= 0xdfff; n++) bus.mmio[n] = this;  //WRAM
  for(unsigned n = 0xe000; n <= 0xfdff; n++) bus.mmio[n] = this;  //WRAM (mirror)
  for(unsigned n = 0xff80; n <= 0xfffe; n++) bus.mmio[n] = this;  //HRAM

  bus.mmio[0xff00] = this;  //JOYP
  bus.mmio[0xff01] = this;  //SB
  bus.mmio[0xff02] = this;  //SC
  bus.mmio[0xff04] = this;  //DIV
  bus.mmio[0xff05] = this;  //TIMA
  bus.mmio[0xff06] = this;  //TMA
  bus.mmio[0xff07] = this;  //TAC
  bus.mmio[0xff0f] = this;  //IF
  bus.mmio[0xff46] = this;  //DMA
  bus.mmio[0xffff] = this;  //IE

  if(system.cgb()) {
  bus.mmio[0xff4d] = this;  //KEY1
  bus.mmio[0xff51] = this;  //HDMA1
  bus.mmio[0xff52] = this;  //HDMA2
  bus.mmio[0xff53] = this;  //HDMA3
  bus.mmio[0xff54] = this;  //HDMA4
  bus.mmio[0xff55] = this;  //HDMA5
  bus.mmio[0xff56] = this;  //RP
  bus.mmio[0xff6c] = this;  //???
  bus.mmio[0xff70] = this;  //SVBK
  bus.mmio[0xff72] = this;  //???
  bus.mmio[0xff73] = this;  //???
  bus.mmio[0xff74] = this;  //???
  bus.mmio[0xff75] = this;  //???
  bus.mmio[0xff76] = this;  //???
  bus.mmio[0xff77] = this;  //???
  }

  for(auto &n : wram) n = 0x00;
  for(auto &n : hram) n = 0x00;

  r[PC] = 0x0000;
  r[SP] = 0x0000;
  r[AF] = 0x0000;
  r[BC] = 0x0000;
  r[DE] = 0x0000;
  r[HL] = 0x0000;

  status.clock = 0;

  status.p15 = 0;
  status.p14 = 0;
  status.joyp = 0;
  status.mlt_req = 0;

  status.serial_data = 0;
  status.serial_bits = 0;

  status.serial_transfer = 0;
  status.serial_clock = 0;

  status.div = 0;

  status.tima = 0;

  status.tma = 0;

  status.timer_enable = 0;
  status.timer_clock = 0;

  status.interrupt_request_joypad = 0;
  status.interrupt_request_serial = 0;
  status.interrupt_request_timer = 0;
  status.interrupt_request_stat = 0;
  status.interrupt_request_vblank = 0;

  status.speed_double = 0;
  status.speed_switch = 0;

  status.dma_source = 0;
  status.dma_target = 0;

  status.dma_mode = 0;
  status.dma_length = 0;

  status.ff6c = 0;
  status.ff72 = 0;
  status.ff73 = 0;
  status.ff74 = 0;
  status.ff75 = 0;

  status.wram_bank = 1;

  status.interrupt_enable_joypad = 0;
  status.interrupt_enable_serial = 0;
  status.interrupt_enable_timer = 0;
  status.interrupt_enable_stat = 0;
  status.interrupt_enable_vblank = 0;
}

}
