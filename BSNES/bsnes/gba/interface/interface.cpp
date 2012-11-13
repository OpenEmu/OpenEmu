#include <gba/gba.hpp>

namespace GameBoyAdvance {

Interface *interface = nullptr;

double Interface::videoFrequency() {
  return 16777216.0 / (228.0 * 1232.0);
}

double Interface::audioFrequency() {
  return 16777216.0 / 512.0;
}

bool Interface::loaded() {
  return cartridge.loaded();
}

void Interface::load(unsigned id, const stream &stream, const string &markup) {
  if(id == ID::BIOS) {
    stream.read(bios.data, min(bios.size, stream.size()));
  }

  if(id == ID::ROM) {
    memory.reset();
    cartridge.load(markup, stream);
    system.power();
  }

  if(id == ID::RAM) {
    stream.read(cartridge.ram.data, min(cartridge.ram.size, stream.size()));
  }

  if(id == ID::EEPROM) {
    stream.read(cartridge.eeprom.data, min(cartridge.eeprom.size, stream.size()));
  }

  if(id == ID::FlashROM) {
    stream.read(cartridge.flashrom.data, min(cartridge.flashrom.size, stream.size()));
  }
}

void Interface::save(unsigned id, const stream &stream) {
  if(id == ID::RAM) {
    stream.write(cartridge.ram.data, cartridge.ram.size);
  }

  if(id == ID::EEPROM) {
    stream.write(cartridge.eeprom.data, cartridge.eeprom.size);
  }

  if(id == ID::FlashROM) {
    stream.write(cartridge.flashrom.data, cartridge.flashrom.size);
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

void Interface::updatePalette() {
  video.generate_palette();
}

Interface::Interface() {
  interface = this;

  information.name        = "Game Boy Advance";
  information.width       = 240;
  information.height      = 160;
  information.overscan    = false;
  information.aspectRatio = 1.0;
  information.resettable  = false;

  firmware.append({ID::BIOS, "Game Boy Advance", "sys", "bios.rom"});

  media.append({ID::ROM, "Game Boy Advance", "sys", "program.rom", "gba"});

  {
    Device device{0, ID::Device, "Controller"};
    device.input.append({0, 0, "A"     });
    device.input.append({1, 0, "B"     });
    device.input.append({2, 0, "Select"});
    device.input.append({3, 0, "Start" });
    device.input.append({4, 0, "Right" });
    device.input.append({5, 0, "Left"  });
    device.input.append({6, 0, "Up"    });
    device.input.append({7, 0, "Down"  });
    device.input.append({8, 0, "R"     });
    device.input.append({9, 0, "L"     });
    device.order = {6, 7, 5, 4, 1, 0, 9, 8, 2, 3};
    this->device.append(device);
  }

  port.append({0, "Device", {device[0]}});
}

}
