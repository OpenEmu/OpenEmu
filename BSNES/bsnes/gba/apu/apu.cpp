#include <gba/gba.hpp>

namespace GameBoyAdvance {

#include "registers.cpp"
#include "mmio.cpp"
#include "square.cpp"
#include "square1.cpp"
#include "square2.cpp"
#include "wave.cpp"
#include "noise.cpp"
#include "sequencer.cpp"
#include "fifo.cpp"
#include "serialization.cpp"
APU apu;

void APU::Enter() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    apu.main();
  }
}

void APU::main() {
  for(unsigned n = 0; n < 128; n++) {
    runsequencer();
  }

  signed lsample = regs.bias.level - 0x0200;
  signed rsample = regs.bias.level - 0x0200;

  //(4-bit x 4 -> 6-bit) + 3-bit volume = 9-bit output
  if(sequencer.masterenable) {
    signed lsequence = 0;
    if(sequencer.lenable[0]) lsequence += square1.output;
    if(sequencer.lenable[1]) lsequence += square2.output;
    if(sequencer.lenable[2]) lsequence +=    wave.output;
    if(sequencer.lenable[3]) lsequence +=   noise.output;

    signed rsequence = 0;
    if(sequencer.renable[0]) rsequence += square1.output;
    if(sequencer.renable[1]) rsequence += square2.output;
    if(sequencer.renable[2]) rsequence +=    wave.output;
    if(sequencer.renable[3]) rsequence +=   noise.output;

    if(sequencer.volume < 3) {
      lsample += lsequence * (sequencer.lvolume + 1) >> (2 - sequencer.volume);
      rsample += rsequence * (sequencer.rvolume + 1) >> (2 - sequencer.volume);
    }
  }

  //(8-bit x 2 -> 7-bit) + 1-bit volume = 10-bit output
  signed fifo0 = fifo[0].output + (1 << fifo[0].volume);
  signed fifo1 = fifo[1].output + (1 << fifo[1].volume);

  if(fifo[0].lenable) lsample += fifo0;
  if(fifo[1].lenable) lsample += fifo1;

  if(fifo[0].renable) rsample += fifo0;
  if(fifo[1].renable) rsample += fifo1;

  lsample = sclamp<10>(lsample);
  rsample = sclamp<10>(rsample);

  if(regs.bias.amplitude == 1) lsample &=  ~3, rsample &=  ~3;
  if(regs.bias.amplitude == 2) lsample &=  ~7, rsample &=  ~7;
  if(regs.bias.amplitude == 3) lsample &= ~15, rsample &= ~15;

  if(cpu.regs.mode == CPU::Registers::Mode::Stop) lsample = 0, rsample = 0;
  interface->audioSample(sclamp<16>(lsample << 7), sclamp<16>(rsample << 7));  //should be <<5, use <<7 for added volume
  step(512);
}

void APU::step(unsigned clocks) {
  clock += clocks;
  if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
}

void APU::power() {
  create(APU::Enter, 16777216);

  square1.power();
  square2.power();
  wave.power();
  noise.power();
  sequencer.power();
  fifo[0].power();
  fifo[1].power();

  regs.bias = 0x0200;

  for(unsigned n = 0x060; n <= 0x0a7; n++) bus.mmio[n] = this;
}

}
