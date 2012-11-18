#include <sfc/sfc.hpp>

#define ARMDSP_CPP
namespace SuperFamicom {

#include "memory.cpp"
#include "serialization.cpp"
ArmDSP armdsp;

void ArmDSP::Enter() { armdsp.enter(); }

void ArmDSP::enter() {
  //reset hold delay
  while(bridge.reset) {
    step(1);
    continue;
  }

  //reset sequence delay
  if(bridge.ready == false) {
    step(65536);
    bridge.ready = true;
  }

  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(crash) {
      print(disassemble_arm_instruction(pipeline.execute.address), "\n");
      print(disassemble_registers(), "\n");
      print("Executed: ", instructions, "\n");
      while(true) step(frequency);
    }

    arm_step();
  }
}

void ArmDSP::step(unsigned clocks) {
  if(bridge.timer && --bridge.timer == 0);
  Coprocessor::step(clocks);
  synchronize_cpu();
}

//MMIO: $00-3f|80-bf:3800-38ff
//3800-3807 mirrored throughout
//a0 ignored

uint8 ArmDSP::mmio_read(unsigned addr) {
  cpu.synchronize_coprocessors();

  uint8 data = 0x00;
  addr &= 0xff06;

  if(addr == 0x3800) {
    if(bridge.armtocpu.ready) {
      bridge.armtocpu.ready = false;
      data = bridge.armtocpu.data;
    }
  }

  if(addr == 0x3802) {
    bridge.signal = false;
  }

  if(addr == 0x3804) {
    data = bridge.status();
  }

  return data;
}

void ArmDSP::mmio_write(unsigned addr, uint8 data) {
  cpu.synchronize_coprocessors();

  addr &= 0xff06;

  if(addr == 0x3802) {
    bridge.cputoarm.ready = true;
    bridge.cputoarm.data = data;
  }

  if(addr == 0x3804) {
    data &= 1;
    if(!bridge.reset && data) arm_reset();
    bridge.reset = data;
  }
}

void ArmDSP::init() {
}

void ArmDSP::load() {
}

void ArmDSP::unload() {
}

void ArmDSP::power() {
  for(unsigned n = 0; n < 16 * 1024; n++) programRAM[n] = random(0x00);
}

void ArmDSP::reset() {
  bridge.reset = false;
  arm_reset();
}

void ArmDSP::arm_reset() {
  create(ArmDSP::Enter, 21477272);
  ARM::power();

  bridge.ready = false;
  bridge.signal = false;
  bridge.timer = 0;
  bridge.timerlatch = 0;
  bridge.cputoarm.ready = false;
  bridge.armtocpu.ready = false;
}

ArmDSP::ArmDSP() {
  firmware = new uint8[160 * 1024]();
  programRAM = new uint8[16 * 1024]();

  programROM = &firmware[0];
  dataROM = &firmware[128 * 1024];
}

ArmDSP::~ArmDSP() {
  delete[] firmware;
  delete[] programRAM;
}

}
