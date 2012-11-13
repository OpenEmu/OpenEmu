#include <gb/gb.hpp>

namespace GameBoy {

Interface *interface = nullptr;

void Interface::lcdScanline() {
  if(hook) hook->lcdScanline();
}

void Interface::joypWrite(bool p15, bool p14) {
  if(hook) hook->joypWrite(p15, p14);
}

double Interface::videoFrequency() {
  return 4194304.0 / (154.0 * 456.0);
}

double Interface::audioFrequency() {
  return 4194304.0;
}

bool Interface::loaded() {
  return cartridge.loaded();
}

string Interface::sha256() {
  return cartridge.sha256();
}

void Interface::load(unsigned id, const stream &stream, const string &markup) {
  if(id == ID::GameBoyBootROM) {
    stream.read(system.bootROM.dmg, min( 256u, stream.size()));
  }

  if(id == ID::SuperGameBoyBootROM) {
    stream.read(system.bootROM.sgb, min( 256u, stream.size()));
  }

  if(id == ID::GameBoyColorBootROM) {
    stream.read(system.bootROM.cgb, min(2048u, stream.size()));
  }

  if(id == ID::GameBoyROM) {
    cartridge.load(System::Revision::GameBoy, markup, stream);
    system.power();
  }

  if(id == ID::GameBoyColorROM) {
    cartridge.load(System::Revision::GameBoyColor, markup, stream);
    system.power();
  }

  if(id == ID::RAM) {
    stream.read(cartridge.ramdata, min(stream.size(), cartridge.ramsize));
  }
}

void Interface::save(unsigned id, const stream &stream) {
  if(id == ID::RAM) {
    stream.write(cartridge.ramdata, cartridge.ramsize);
  }
}

void Interface::unload() {
  cartridge.unload();
}

void Interface::power() {
  system.power();
}

void Interface::reset() {
  system.power();
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
  cheat.reset();
  for(auto &code : list) {
    lstring codelist = code.split("+");
    for(auto &part : codelist) {
      unsigned addr, data, comp;
      if(Cheat::decode(part, addr, data, comp)) cheat.append({addr, data, comp});
    }
  }
  cheat.synchronize();
}

void Interface::updatePalette() {
  video.generate_palette();
}

Interface::Interface() {
  interface = this;
  hook = nullptr;

  information.name        = "Game Boy";
  information.width       = 160;
  information.height      = 144;
  information.overscan    = false;
  information.aspectRatio = 1.0;
  information.resettable  = false;

  firmware.append({ID::GameBoyBootROM,      "Game Boy",       "sys", "boot.rom"});
  firmware.append({ID::SuperGameBoyBootROM, "Super Game Boy", "sfc", "boot.rom"});
  firmware.append({ID::GameBoyColorBootROM, "Game Boy Color", "sys", "boot.rom"});

  media.append({ID::GameBoyROM,      "Game Boy",       "sys", "program.rom", "gb" });
  media.append({ID::GameBoyColorROM, "Game Boy Color", "sys", "program.rom", "gbc"});

  {
    Device device{0, ID::Device, "Controller"};
    device.input.append({0, 0, "Up"    });
    device.input.append({1, 0, "Down"  });
    device.input.append({2, 0, "Left"  });
    device.input.append({3, 0, "Right" });
    device.input.append({4, 0, "B"     });
    device.input.append({5, 0, "A"     });
    device.input.append({6, 0, "Select"});
    device.input.append({7, 0, "Start" });
    device.order = {0, 1, 2, 3, 4, 5, 6, 7};
    this->device.append(device);
  }

  port.append({0, "Device", {device[0]}});
}

}
