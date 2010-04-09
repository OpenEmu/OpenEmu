#include <../base.hpp>

#define SYSTEM_CPP
namespace SNES {

System system;

#include "config/config.cpp"
#include "debugger/debugger.cpp"
#include "serialization.cpp"
#include "scheduler/scheduler.cpp"
#include "statemanager/statemanager.cpp"

#include "video/video.cpp"
#include "audio/audio.cpp"
#include "input/input.cpp"

void System::coprocessor_enter() {
  if(cartridge.mode() == Cartridge::ModeSuperGameBoy) supergameboy.enter();
  if(cartridge.has_superfx()) superfx.enter();
  if(cartridge.has_sa1()) sa1.enter();

  while(true) {
    scheduler.addclocks_cop(64 * 1024 * 1024);
    scheduler.sync_copcpu();
  }
}

void System::run() {
  scheduler.sync = Scheduler::SyncNone;

  scheduler.enter();
  input.update();
  video.update();
}

void System::runtosave() {
  scheduler.sync = Scheduler::SyncCpu;

  while(true) {
    scheduler.enter();
    if(scheduler.sync == Scheduler::SyncAll) break;
    input.update();
    video.update();
  }

  scheduler.thread_active = scheduler.thread_smp;
  scheduler.enter();

  scheduler.thread_active = scheduler.thread_ppu;
  scheduler.enter();

  #if !defined(DSP_STATE_MACHINE)
  scheduler.thread_active = scheduler.thread_dsp;
  scheduler.enter();
  #endif
}

void System::init(Interface *interface_) {
  interface = interface_;
  assert(interface != 0);

  supergameboy.init();
  sa1.init();
  superfx.init();
  bsxbase.init();
  bsxcart.init();
  bsxflash.init();
  srtc.init();
  sdd1.init();
  spc7110.init();
  cx4.init();
  dsp1.init();
  dsp2.init();
  dsp3.init();
  dsp4.init();
  obc1.init();
  st010.init();
  st011.init();
  st018.init();

  video.init();
  audio.init();
  input.init();
}

void System::term() {
}

void System::power() {
  snes_region = max(0, min(2, config.region));
  snes_expansion = max(0, min(1, config.expansion_port));

  if(snes_region == Autodetect) {
    snes_region = (cartridge.region() == Cartridge::NTSC ? NTSC : PAL);
  }

  audio.coprocessor_enable(false);

  scheduler.init();
  bus.power();

  for(unsigned i = 0x2100; i <= 0x213f; i++) memory::mmio.map(i, ppu);
  for(unsigned i = 0x2140; i <= 0x217f; i++) memory::mmio.map(i, cpu);
  for(unsigned i = 0x2180; i <= 0x2183; i++) memory::mmio.map(i, cpu);
  for(unsigned i = 0x4016; i <= 0x4017; i++) memory::mmio.map(i, cpu);
  for(unsigned i = 0x4200; i <= 0x421f; i++) memory::mmio.map(i, cpu);
  for(unsigned i = 0x4300; i <= 0x437f; i++) memory::mmio.map(i, cpu);

  if(expansion() == ExpansionBSX) bsxbase.enable();
  if(memory::bsxflash.data()) bsxflash.enable();
  if(cartridge.mode() == Cartridge::ModeBsx) bsxcart.enable();
  if(cartridge.mode() == Cartridge::ModeSuperGameBoy) supergameboy.enable();

  if(cartridge.has_superfx()) superfx.enable();
  if(cartridge.has_sa1())     sa1.enable();
  if(cartridge.has_srtc())    srtc.enable();
  if(cartridge.has_sdd1())    sdd1.enable();
  if(cartridge.has_spc7110()) spc7110.enable();
  if(cartridge.has_cx4())     cx4.enable();
  if(cartridge.has_dsp1())    dsp1.enable();
  if(cartridge.has_dsp2())    dsp2.enable();
  if(cartridge.has_dsp3())    dsp3.enable();
  if(cartridge.has_dsp4())    dsp4.enable();
  if(cartridge.has_obc1())    obc1.enable();
  if(cartridge.has_st010())   st010.enable();
  if(cartridge.has_st011())   st011.enable();
  if(cartridge.has_st018())   st018.enable();

  if(expansion() == ExpansionBSX) bsxbase.power();
  if(memory::bsxflash.data()) bsxflash.power();
  if(cartridge.mode() == Cartridge::ModeBsx) bsxcart.power();
  if(cartridge.mode() == Cartridge::ModeSuperGameBoy) supergameboy.power();

  if(cartridge.has_superfx()) superfx.power();
  if(cartridge.has_sa1())     sa1.power();
  if(cartridge.has_srtc())    srtc.power();
  if(cartridge.has_sdd1())    sdd1.power();
  if(cartridge.has_spc7110()) spc7110.power();
  if(cartridge.has_cx4())     cx4.power();
  if(cartridge.has_dsp1())    dsp1.power();
  if(cartridge.has_dsp2())    dsp2.power();
  if(cartridge.has_dsp3())    dsp3.power();
  if(cartridge.has_dsp4())    dsp4.power();
  if(cartridge.has_obc1())    obc1.power();
  if(cartridge.has_st010())   st010.power();
  if(cartridge.has_st011())   st011.power();
  if(cartridge.has_st018())   st018.power();

  //ppu.PPUcounter::reset();
  cpu.power();
  smp.power();
  dsp.power();
  ppu.power();

  input.port_set_device(0, config.controller_port1);
  input.port_set_device(1, config.controller_port2);
  input.update();
  video.update();
}

void System::reset() {
  scheduler.init();

  //ppu.PPUcounter::reset();
  cpu.reset();
  smp.reset();
  dsp.reset();
  ppu.reset();
  bus.reset();

  if(expansion() == ExpansionBSX) bsxbase.reset();
  if(memory::bsxflash.data()) bsxflash.reset();
  if(cartridge.mode() == Cartridge::ModeBsx) bsxcart.reset();
  if(cartridge.mode() == Cartridge::ModeSuperGameBoy) supergameboy.reset();

  if(cartridge.has_superfx()) superfx.reset();
  if(cartridge.has_sa1())     sa1.reset();
  if(cartridge.has_srtc())    srtc.reset();
  if(cartridge.has_sdd1())    sdd1.reset();
  if(cartridge.has_spc7110()) spc7110.reset();
  if(cartridge.has_cx4())     cx4.reset();
  if(cartridge.has_dsp1())    dsp1.reset();
  if(cartridge.has_dsp2())    dsp2.reset();
  if(cartridge.has_dsp3())    dsp3.reset();
  if(cartridge.has_dsp4())    dsp4.reset();
  if(cartridge.has_obc1())    obc1.reset();
  if(cartridge.has_st010())   st010.reset();
  if(cartridge.has_st011())   st011.reset();
  if(cartridge.has_st018())   st018.reset();

  input.port_set_device(0, config.controller_port1);
  input.port_set_device(1, config.controller_port2);
  input.update();
  video.update();
}

void System::unload() {
  if(cartridge.mode() == Cartridge::ModeSuperGameBoy) supergameboy.unload();
}

void System::scanline() {
  video.scanline();
  if(cpu.vcounter() == 241) scheduler.exit();
}

void System::frame() {
}

System::Region System::region() const {
  return (System::Region)snes_region;
}

System::ExpansionPortDevice System::expansion() const {
  return (System::ExpansionPortDevice)snes_expansion;
}

System::System() : interface(0), snes_region(NTSC), snes_expansion(ExpansionNone) {
}

};
