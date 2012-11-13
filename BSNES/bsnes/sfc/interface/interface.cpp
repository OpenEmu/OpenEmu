#include <sfc/sfc.hpp>

namespace SuperFamicom {

Interface *interface = nullptr;

double Interface::videoFrequency() {
  switch(system.region()) { default:
  case System::Region::NTSC: return system.cpu_frequency() / (262.0 * 1364.0 - 4.0);
  case System::Region::PAL:  return system.cpu_frequency() / (312.0 * 1364.0);
  }
}

double Interface::audioFrequency() {
  return system.apu_frequency() / 768.0;
}

bool Interface::loaded() {
  return cartridge.loaded();
}

string Interface::sha256() {
  return cartridge.sha256();
}

unsigned Interface::group(unsigned id) {
  switch(id) {
  case ID::Nec7725DSP:
  case ID::Nec96050DSP:
  case ID::HitachiDSP:
  case ID::ArmDSP:
  case ID::ROM:
  case ID::RAM:
  case ID::NecDSPRAM:
  case ID::RTC:
  case ID::SPC7110RTC:
  case ID::BsxRAM:
  case ID::BsxPSRAM:
    return 0;
  case ID::SuperGameBoyROM:
  case ID::SuperGameBoyRAM:
  case ID::SuperGameBoyRTC:
    return 1;
  case ID::BsxFlashROM:
    return 2;
  case ID::SufamiTurboSlotAROM:
  case ID::SufamiTurboSlotARAM:
    return 3;
  case ID::SufamiTurboSlotBROM:
  case ID::SufamiTurboSlotBRAM:
    return 4;
  }
  return 0;
}

void Interface::load(unsigned id, const stream &stream, const string &markup) {
  if(id == ID::IPLROM) {
    stream.read(smp.iplrom, min(64u, stream.size()));
  }

  if(id == ID::Nec7725DSP) {
    for(unsigned n = 0; n <  2048; n++) necdsp.programROM[n] = stream.readl(3);
    for(unsigned n = 0; n <  1024; n++) necdsp.dataROM[n]    = stream.readl(2);
  }

  if(id == ID::Nec96050DSP) {
    for(unsigned n = 0; n < 16384; n++) necdsp.programROM[n] = stream.readl(3);
    for(unsigned n = 0; n <  2048; n++) necdsp.dataROM[n]    = stream.readl(2);
  }

  if(id == ID::HitachiDSP) {
    for(unsigned n = 0; n < 1024; n++) hitachidsp.dataROM[n] = stream.readl(3);
  }

  if(id == ID::ArmDSP) {
    stream.read(armdsp.firmware, stream.size());
  }

  if(id == ID::ROM) {
    cartridge.load(markup, stream);
    system.power();
  }

  if(id == ID::SuperGameBoyROM) {
    GameBoy::cartridge.load(GameBoy::System::Revision::SuperGameBoy, markup, stream);
  }

  if(id == ID::BsxFlashROM) {
    bsxflash.memory.copy(stream);
  }

  if(id == ID::SufamiTurboSlotAROM) {
    sufamiturbo.slotA.rom.copy(stream);
  }

  if(id == ID::SufamiTurboSlotBROM) {
    sufamiturbo.slotB.rom.copy(stream);
  }

  if(id == ID::RAM) {
    stream.read(cartridge.ram.data(), min(cartridge.ram.size(), stream.size()));
  }

  if(id == ID::NecDSPRAM) {
    for(unsigned n = 0; n < 2048; n++) necdsp.dataRAM[n] = stream.readl(2);
  }

  if(id == ID::RTC) {
    stream.read(srtc.rtc, min(stream.size(), sizeof srtc.rtc));
  }

  if(id == ID::SPC7110RTC) {
    stream.read(spc7110.rtc, min(stream.size(), sizeof srtc.rtc));
  }

  if(id == ID::BsxRAM) {
    stream.read(bsxcartridge.sram.data(), min(stream.size(), bsxcartridge.sram.size()));
  }

  if(id == ID::BsxPSRAM) {
    stream.read(bsxcartridge.psram.data(), min(stream.size(), bsxcartridge.psram.size()));
  }

  if(id == ID::SuperGameBoyRAM) {
    stream.read(GameBoy::cartridge.ramdata, GameBoy::cartridge.ramsize);
  }

  if(id == ID::SufamiTurboSlotARAM) {
    sufamiturbo.slotA.ram.copy(stream);
  }

  if(id == ID::SufamiTurboSlotBRAM) {
    sufamiturbo.slotB.ram.copy(stream);
  }
}

void Interface::save(unsigned id, const stream &stream) {
  if(id == ID::RAM) {
    stream.write(cartridge.ram.data(), cartridge.ram.size());
  }

  if(id == ID::NecDSPRAM) {
    for(unsigned n = 0; n < 2048; n++) stream.writel(necdsp.dataRAM[n], 2);
  }

  if(id == ID::RTC) {
    stream.write(srtc.rtc, sizeof srtc.rtc);
  }

  if(id == ID::SPC7110RTC) {
    stream.write(spc7110.rtc, sizeof srtc.rtc);
  }

  if(id == ID::BsxRAM) {
    stream.write(bsxcartridge.sram.data(), bsxcartridge.sram.size());
  }

  if(id == ID::BsxPSRAM) {
    stream.write(bsxcartridge.psram.data(), bsxcartridge.psram.size());
  }

  if(id == ID::SuperGameBoyRAM) {
    stream.write(GameBoy::cartridge.ramdata, GameBoy::cartridge.ramsize);
  }

  if(id == ID::SufamiTurboSlotARAM) {
    stream.write(sufamiturbo.slotA.ram.data(), sufamiturbo.slotA.ram.size());
  }

  if(id == ID::SufamiTurboSlotBRAM) {
    stream.write(sufamiturbo.slotB.ram.data(), sufamiturbo.slotB.ram.size());
  }
}

void Interface::unload() {
  cartridge.unload();
}

void Interface::connect(unsigned port, unsigned device) {
  input.connect(port, (Input::Device)device);
}

void Interface::power() {
  system.power();
}

void Interface::reset() {
  system.reset();
}

void Interface::run() {
  system.run();
}

serializer Interface::serialize() {
  system.runtosave();
  return system.serialize();
}

bool Interface::unserialize(serializer &s) {
  return system.unserialize(s);
}

void Interface::cheatSet(const lstring &list) {
  //Super Game Boy
  if(cartridge.has_gb_slot()) {
    GameBoy::cheat.reset();
    for(auto &code : list) {
      lstring codelist = code.split("+");
      for(auto &part : codelist) {
        unsigned addr, data, comp;
        part.trim();
        if(GameBoy::Cheat::decode(part, addr, data, comp)) GameBoy::cheat.append({addr, data, comp});
      }
    }
    GameBoy::cheat.synchronize();
    return;
  }

  //Super Famicom, Broadcast Satellaview, Sufami Turbo
  cheat.reset();
  for(auto &code : list) {
    lstring codelist = code.split("+");
    for(auto &part : codelist) {
      unsigned addr, data;
      part.trim();
      if(Cheat::decode(part, addr, data)) cheat.append({addr, data});
    }
  }
  cheat.synchronize();
}

void Interface::updatePalette() {
  video.generate_palette();
}

Interface::Interface() {
  interface = this;
  system.init();

  information.name        = "Super Famicom";
  information.width       = 256;
  information.height      = 240;
  information.overscan    = true;
  information.aspectRatio = 8.0 / 7.0;
  information.resettable  = true;

  firmware.append({ID::IPLROM, "Super Famicom", "sys", "spc700.rom"});

  media.append({ID::ROM, "Super Famicom",    "sys", "program.rom", "sfc"});
  media.append({ID::ROM, "Super Game Boy",   "sfc", "program.rom", "gb" });
  media.append({ID::ROM, "BS-X Satellaview", "sfc", "program.rom", "bs" });
  media.append({ID::ROM, "Sufami Turbo",     "sfc", "program.rom", "st" });

  {
    Device device{0, ID::Port1 | ID::Port2, "Controller"};
    device.input.append({ 0, 0, "B"     });
    device.input.append({ 1, 0, "Y"     });
    device.input.append({ 2, 0, "Select"});
    device.input.append({ 3, 0, "Start" });
    device.input.append({ 4, 0, "Up"    });
    device.input.append({ 5, 0, "Down"  });
    device.input.append({ 6, 0, "Left"  });
    device.input.append({ 7, 0, "Right" });
    device.input.append({ 8, 0, "A"     });
    device.input.append({ 9, 0, "X"     });
    device.input.append({10, 0, "L"     });
    device.input.append({11, 0, "R"     });
    device.order = {4, 5, 6, 7, 0, 8, 1, 9, 10, 11, 2, 3};
    this->device.append(device);
  }

  {
    Device device{1, ID::Port1 | ID::Port2, "Multitap"};
    for(unsigned p = 1, n = 0; p <= 4; p++, n += 12) {
      device.input.append({n +  0, 0, {"Port ", p, " - ", "B"     }});
      device.input.append({n +  1, 0, {"Port ", p, " - ", "Y"     }});
      device.input.append({n +  2, 0, {"Port ", p, " - ", "Select"}});
      device.input.append({n +  3, 0, {"Port ", p, " - ", "Start" }});
      device.input.append({n +  4, 0, {"Port ", p, " - ", "Up"    }});
      device.input.append({n +  5, 0, {"Port ", p, " - ", "Down"  }});
      device.input.append({n +  6, 0, {"Port ", p, " - ", "Left"  }});
      device.input.append({n +  7, 0, {"Port ", p, " - ", "Right" }});
      device.input.append({n +  8, 0, {"Port ", p, " - ", "A"     }});
      device.input.append({n +  9, 0, {"Port ", p, " - ", "X"     }});
      device.input.append({n + 10, 0, {"Port ", p, " - ", "L"     }});
      device.input.append({n + 11, 0, {"Port ", p, " - ", "R"     }});
      device.order.append(n + 4, n + 5, n +  6, n +  7, n + 0, n + 8);
      device.order.append(n + 1, n + 9, n + 10, n + 11, n + 2, n + 3);
    }
    this->device.append(device);
  }

  {
    Device device{2, ID::Port1 | ID::Port2, "Mouse"};
    device.input.append({0, 1, "X-axis"});
    device.input.append({1, 1, "Y-axis"});
    device.input.append({2, 0, "Left"  });
    device.input.append({3, 0, "Right" });
    device.order = {0, 1, 2, 3};
    this->device.append(device);
  }

  {
    Device device{3, ID::Port2, "Super Scope"};
    device.input.append({0, 1, "X-axis" });
    device.input.append({1, 1, "Y-axis" });
    device.input.append({2, 0, "Trigger"});
    device.input.append({3, 0, "Cursor" });
    device.input.append({4, 0, "Turbo"  });
    device.input.append({5, 0, "Pause"  });
    device.order = {0, 1, 2, 3, 4, 5};
    this->device.append(device);
  }

  {
    Device device{4, ID::Port2, "Justifier"};
    device.input.append({0, 1, "X-axis" });
    device.input.append({1, 1, "Y-axis" });
    device.input.append({2, 0, "Trigger"});
    device.input.append({3, 0, "Start"  });
    device.order = {0, 1, 2, 3};
    this->device.append(device);
  }

  {
    Device device{5, ID::Port2, "Justifiers"};
    device.input.append({0, 1, "Port 1 - X-axis" });
    device.input.append({1, 1, "Port 1 - Y-axis" });
    device.input.append({2, 0, "Port 1 - Trigger"});
    device.input.append({3, 0, "Port 1 - Start"  });
    device.order.append(0, 1, 2, 3);
    device.input.append({4, 1, "Port 2 - X-axis" });
    device.input.append({5, 1, "Port 2 - Y-axis" });
    device.input.append({6, 0, "Port 2 - Trigger"});
    device.input.append({7, 0, "Port 2 - Start"  });
    device.order.append(4, 5, 6, 7);
    this->device.append(device);
  }

  {
    Device device{6, ID::Port1, "Serial USART"};
    this->device.append(device);
  }

  {
    Device device{7, ID::Port1 | ID::Port2, "None"};
    this->device.append(device);
  }

  port.append({0, "Port 1"});
  port.append({1, "Port 2"});

  for(auto &device : this->device) {
    for(auto &port : this->port) {
      if(device.portmask & (1 << port.id)) {
        port.device.append(device);
      }
    }
  }
}

}
