#include <sfc/sfc.hpp>

#define ICD2_CPP
namespace SuperFamicom {

#include "interface/interface.cpp"
#include "mmio/mmio.cpp"
#include "serialization.cpp"
ICD2 icd2;

void ICD2::Enter() { icd2.enter(); }

void ICD2::enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      GameBoy::system.runtosave();
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if(r6003 & 0x80) {
      GameBoy::system.run();
      step(GameBoy::system.clocks_executed);
      GameBoy::system.clocks_executed = 0;
    } else {  //DMG halted
      audio.coprocessor_sample(0x0000, 0x0000);
      step(1);
    }
    synchronize_cpu();
  }
}

void ICD2::init() {
}

void ICD2::load() {
  bind = GameBoy::interface->bind;
  hook = GameBoy::interface->hook;
  GameBoy::interface->bind = this;
  GameBoy::interface->hook = this;

  interface->memory.append({ID::SuperGameBoyRAM, "save.ram"});
}

void ICD2::unload() {
  GameBoy::interface->bind = bind;
  GameBoy::interface->hook = hook;
}

void ICD2::power() {
  audio.coprocessor_enable(true);
  audio.coprocessor_frequency(4 * 1024 * 1024);
}

void ICD2::reset() {
  create(ICD2::Enter, cpu.frequency / 5);

  r6000_ly = 0x00;
  r6000_row = 0x00;
  r6003 = 0x00;
  r6004 = 0xff;
  r6005 = 0xff;
  r6006 = 0xff;
  r6007 = 0xff;
  for(auto &r : r7000) r = 0x00;
  r7800 = 0x0000;
  mlt_req = 0;

  for(auto &n : lcd.buffer) n = 0;
  for(auto &n : lcd.output) n = 0;
  lcd.row = 0;

  packetsize = 0;
  joyp_id = 3;
  joyp15lock = 0;
  joyp14lock = 0;
  pulselock = true;

  GameBoy::video.generate_palette();
  GameBoy::system.init();
  GameBoy::system.power();
}

}
