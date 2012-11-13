#ifdef CONTROLLER_CPP

uint2 Mouse::data() {
  if(counter >= 32) return 1;

  int position_x = interface->inputPoll(port, (unsigned)Input::Device::Mouse, (unsigned)Input::MouseID::X);  //-n = left, 0 = center, +n = right
  int position_y = interface->inputPoll(port, (unsigned)Input::Device::Mouse, (unsigned)Input::MouseID::Y);  //-n = up,   0 = center, +n = down

  bool direction_x = position_x < 0;  //0 = right, 1 = left
  bool direction_y = position_y < 0;  //0 = down,  1 = up

  if(position_x < 0) position_x = -position_x;  //abs(position_x)
  if(position_y < 0) position_y = -position_y;  //abs(position_y)

  position_x = min(127, position_x);  //range = 0 - 127
  position_y = min(127, position_y);

  switch(counter++) { default:
  case  0: return 0;
  case  1: return 0;
  case  2: return 0;
  case  3: return 0;
  case  4: return 0;
  case  5: return 0;
  case  6: return 0;
  case  7: return 0;

  case  8: return interface->inputPoll(port, (unsigned)Input::Device::Mouse, (unsigned)Input::MouseID::Right);
  case  9: return interface->inputPoll(port, (unsigned)Input::Device::Mouse, (unsigned)Input::MouseID::Left);
  case 10: return 0;  //speed (0 = slow, 1 = normal, 2 = fast, 3 = unused)
  case 11: return 0;  // ||

  case 12: return 0;  //signature
  case 13: return 0;  // ||
  case 14: return 0;  // ||
  case 15: return 1;  // ||

  case 16: return (direction_y);
  case 17: return (position_y >> 6) & 1;
  case 18: return (position_y >> 5) & 1;
  case 19: return (position_y >> 4) & 1;
  case 20: return (position_y >> 3) & 1;
  case 21: return (position_y >> 2) & 1;
  case 22: return (position_y >> 1) & 1;
  case 23: return (position_y >> 0) & 1;

  case 24: return (direction_x);
  case 25: return (position_x >> 6) & 1;
  case 26: return (position_x >> 5) & 1;
  case 27: return (position_x >> 4) & 1;
  case 28: return (position_x >> 3) & 1;
  case 29: return (position_x >> 2) & 1;
  case 30: return (position_x >> 1) & 1;
  case 31: return (position_x >> 0) & 1;
  }
}

void Mouse::latch(bool data) {
  if(latched == data) return;
  latched = data;
  counter = 0;
}

Mouse::Mouse(bool port) : Controller(port) {
  latched = 0;
  counter = 0;
}

#endif
