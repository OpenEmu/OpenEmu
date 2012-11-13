#include <fc/fc.hpp>

namespace Famicom {

Interface *interface = nullptr;

double Interface::videoFrequency() {
  return 21477272.0 / (262.0 * 1364.0 - 4.0);
}

double Interface::audioFrequency() {
  return 21477272.0 / 12.0;
}

bool Interface::loaded() {
  return cartridge.loaded();
}

string Interface::sha256() {
  return cartridge.sha256();
}

void Interface::load(unsigned id, const stream &stream, const string &markup) {
  if(id == ID::ROM) {
    cartridge.load(markup, stream);
    system.power();
    input.connect(0, Input::Device::Joypad);
    input.connect(1, Input::Device::Joypad);
  }

  if(id == ID::RAM) {
    stream.read(cartridge.ram_data(), min(stream.size(), cartridge.ram_size()));
  }
}

void Interface::save(unsigned id, const stream &stream) {
  if(id == ID::RAM) {
    stream.write(cartridge.ram_data(), cartridge.ram_size());
  }
}

void Interface::unload() {
  cartridge.unload();
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

  information.name        = "Famicom";
  information.width       = 256;
  information.height      = 240;
  information.overscan    = true;
  information.aspectRatio = 8.0 / 7.0;
  information.resettable  = true;

  media.append({ID::ROM, "Famicom", "sys", "program.rom", "fc"});

  {
    Device device{0, ID::Port1 | ID::Port2, "Controller"};
    device.input.append({0, 0, "A"     });
    device.input.append({1, 0, "B"     });
    device.input.append({2, 0, "Select"});
    device.input.append({3, 0, "Start" });
    device.input.append({4, 0, "Up"    });
    device.input.append({5, 0, "Down"  });
    device.input.append({6, 0, "Left"  });
    device.input.append({7, 0, "Right" });
    device.order = {4, 5, 6, 7, 1, 0, 2, 3};
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
