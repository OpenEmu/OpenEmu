#ifdef CONTROLLER_CPP

uint2 Gamepad::data() {
  if(counter >= 16) return 1;
  uint2 result = interface->inputPoll(port, Input::Device::Joypad, 0, counter);
  if(latched == 0) counter++;
  return result;
}

void Gamepad::latch(bool data) {
  if(latched == data) return;
  latched = data;
  counter = 0;
}

void Gamepad::serialize(serializer& s) {
  Processor::serialize(s);
  //Save block.
  uint8 block[Controller::SaveSize] = {0};
  block[0] = latched ? 1 : 0;
  block[1] = counter;
  s.array(block, Controller::SaveSize);
  if(s.mode() == nall::serializer::Load) {
    latched = (block[0] != 0);
    counter = block[1];
  }
}


Gamepad::Gamepad(bool port) : Controller(port) {
  latched = 0;
  counter = 0;
}

#endif
