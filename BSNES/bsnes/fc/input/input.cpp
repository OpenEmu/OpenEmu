#include <fc/fc.hpp>

namespace Famicom {

#include "serialization.cpp"
Input input;

void Input::latch(bool data) {
  latchdata = data;

  if(latchdata == 1) {
    counter1 = 0;
    counter2 = 0;
  }
}

bool Input::data(bool port) {
  bool result = 0;

  if(port == 0) {
    if(port1 == Device::Joypad) {
      if(counter1 >= 8) return 1;
      result = interface->inputPoll(0, 0u, counter1);
      if(latchdata == 0) counter1++;
    }
  }

  if(port == 1) {
    if(port2 == Device::Joypad) {
      if(counter2 >= 8) return 1;
      result = interface->inputPoll(1, 0u, counter2);
      if(latchdata == 0) counter2++;
    }
  }

  return result;
}

void Input::connect(bool port, Device device) {
  if(port == 0) port1 = device, counter1 = 0;
  if(port == 1) port2 = device, counter2 = 0;
}

void Input::power() {
}

void Input::reset() {
  latchdata = 0;
  counter1 = 0;
  counter2 = 0;
}

}
