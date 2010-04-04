#include "device.cpp"
#include "userinterface.cpp"

//=========
//InputCode
//=========

InputCode::type_t InputCode::type(uint16_t code) {
  for(unsigned i = 0; i < keyboard<>::count; i++) {
    unsigned index = keyboard<>::index(i, keyboard<>::none);
    if(code >= index && code < index + keyboard<>::length) return KeyboardButton;
  }

  for(unsigned i = 0; i < mouse<>::count; i++) {
    unsigned index = mouse<>::index(i, mouse<>::none);
    if(code == index + mouse<>::x) return MouseAxis;
    if(code == index + mouse<>::y) return MouseAxis;
    if(code == index + mouse<>::z) return MouseAxis;

    index = mouse<>::index(i, mouse<>::button);
    if(code >= index && code < index + mouse<>::buttons) return MouseButton;
  }

  for(unsigned i = 0; i < joypad<>::count; i++) {
    unsigned index;

    index = joypad<>::index(i, joypad<>::hat);
    if(code >= index && code < index + joypad<>::hats) return JoypadHat;

    index = joypad<>::index(i, joypad<>::axis);
    if(code >= index && code < index + joypad<>::axes) return JoypadAxis;

    index = joypad<>::index(i, joypad<>::button);
    if(code >= index && code < index + joypad<>::buttons) return JoypadButton;
  }

  return Unknown;
}

InputCode::axistype_t InputCode::axisType(uint16_t code) {
  for(unsigned i = 0; i < joypad<>::count; i++) {
    unsigned index = joypad<>::index(i, joypad<>::axis);
    if(code >= index && code < index + joypad<>::axes) {
      return (InputCode::axistype_t)inputManager.axisType(i, code - index);
    }
  }
  return InvalidAxis;
}

//finds what mouse# is associated with code, returns -1 if not a mouse code
int InputCode::mouseNumber(uint16_t code) {
  for(unsigned i = 0; i < mouse<>::count; i++) {
    unsigned index = mouse<>::index(i, mouse<>::none);
    if(code >= index && code < index + mouse<>::length) return i;
  }
  return -1;
}

//finds what joypad# is associated with code, returns -1 if not a joypad code
int InputCode::joypadNumber(uint16_t code) {
  for(unsigned i = 0; i < joypad<>::count; i++) {
    unsigned index = joypad<>::index(i, joypad<>::none);
    if(code >= index && code < index + joypad<>::length) return i;
  }
  return -1;
}

//============
//InputManager
//============

void InputManager::bind() {
  inputPool.bind();
  inputUiPool.bind();
}

void InputManager::poll() {
  if(config.input.focusPolicy == Configuration::Input::FocusPolicyIgnoreInput
  && mainWindow->isActive() == false) {
    inputPool.clear();
  } else {
    inputPool.poll(stateTable[activeState]);
  }
}

void InputManager::clear() {
  inputPool.clear();
}

void InputManager::flush() {
  for(unsigned i = 0; i < nall::input_limit; i++) {
    stateTable[0][i] = 0;
    stateTable[1][i] = 0;
  }
}

int16_t InputManager::state(uint16_t code) const {
  return stateTable[ activeState][code];
}

int16_t InputManager::lastState(uint16_t code) const {
  return stateTable[!activeState][code];
}

int16_t InputManager::getStatus(unsigned deviceid, unsigned id) const {
  InputDevice *device = inputPool.find((SNES::Input::DeviceID)deviceid);
  if(device) return device->state(id);
  return 0;
}

void InputManager::refresh() {
  bool last = activeState;
  activeState = !activeState;
  bool next = activeState;

  input.poll(stateTable[next]);
  for(unsigned i = 0; i < nall::input_limit; i++) {
    //alert via callback whenever input state changes for any ID ...
    if(onInput && stateTable[last][i] != stateTable[next][i]) onInput(i);
  }
}

void InputManager::calibrate(unsigned joy) {
  unsigned index = joypad<>::index(joy, joypad<>::none);

  for(unsigned axis = 0; axis < joypad<>::axes; axis++) {
    int16_t value = state(index + joypad<>::axis + axis);
    pad[joy].axis[axis] = (value >= -16384 && value <= +16384) ? InputCode::Stick : InputCode::Trigger;
  }

  pad[joy].calibrated = true;
}

bool InputManager::calibrated(unsigned joy) const {
  return pad[joy].calibrated;
}

InputCode::axistype_t InputManager::axisType(unsigned joy, unsigned axis) const {
  return pad[joy].axis[axis];
}

InputManager::InputManager() {
  activeState = 0;
  flush();

  for(unsigned i = 0; i < joypad<>::count; i++) {
    pad[i].calibrated = false;
    for(unsigned n = 0; n < joypad<>::axes; n++) pad[i].axis[n] = InputCode::InvalidAxis;
  }
}

//===========
//InputObject
//===========

void InputObject::bind() {
  lstring part;
  part.split("::", id);

  code = nall::input_find((const char*)part[0]);
  codetype = InputCode::type(code);

  modifier = None;
  if(part.size() > 1) {
    if(part[1] == "up"     ) modifier = Up;
    if(part[1] == "down"   ) modifier = Down;
    if(part[1] == "left"   ) modifier = Left;
    if(part[1] == "right"  ) modifier = Right;
    if(part[1] == "lo"     ) modifier = Lo;
    if(part[1] == "hi"     ) modifier = Hi;
    if(part[1] == "trigger") modifier = Trigger;
  }

  if(code == nall::input_none) id = "none";
}

void InputObject::bind(uint16_t newCode) {
  code = newCode;
  codetype = InputCode::type(code);

  id = nall::input_find(newCode);
  modifier = None;

  if(type == Button && codetype == InputCode::JoypadHat) {
    switch(inputManager.state(code)) {
      case joypad<>::hat_up:    id << "::up";    modifier = Up;    break;
      case joypad<>::hat_down:  id << "::down";  modifier = Down;  break;
      case joypad<>::hat_left:  id << "::left";  modifier = Left;  break;
      case joypad<>::hat_right: id << "::right"; modifier = Right; break;
    }
  } else if(type == Button && codetype == InputCode::JoypadAxis) {
    InputCode::axistype_t type = InputCode::axisType(code);
    int16_t state = inputManager.state(code);

    if(type == InputCode::Stick) {
      if(state < 0) {
        id << "::lo";
        modifier = Lo;
      } else {
        id << "::hi";
        modifier = Hi;
      }
    } else if(type == InputCode::Trigger) {
      id << "::trigger";
      modifier = Trigger;
    }
  }
}

void InputObject::poll(int16_t newState) {
  if(type == Button && codetype == InputCode::JoypadHat) {
    //map 4-way hat input to button state
    state = (modifier == Up    && (newState & joypad<>::hat_up   ))
         || (modifier == Down  && (newState & joypad<>::hat_down ))
         || (modifier == Left  && (newState & joypad<>::hat_left ))
         || (modifier == Right && (newState & joypad<>::hat_right));
  } else if(type == Button && codetype == InputCode::JoypadAxis) {
    //convert analog input to button state
    state = (modifier == Lo && newState < -16384)
         || (modifier == Hi && newState > +16384)
         || (modifier == Trigger && newState < 0);
  } else if(codetype == InputCode::MouseAxis && !input.acquired()) {
    //mouse must be acquired (locked to window) to move axes
    state = 0;
  } else if(0 && codetype == InputCode::MouseButton && !input.acquired()) {
    //same for buttons
    //note: disabled for now ... requiring exclusive access makes it much more difficult
    //to utilize mouse buttons for eg SNES joypad input.
    state = 0;
  } else if(codetype == InputCode::JoypadAxis) {
    //joypad axis range = -32768 to +32767, scale to -8 to +7 to roughly match mouse delta
    //todo: scale mouse up instead of joypad down? (would require mouse DPI / resolution)
    state = newState / 4096;
  } else {
    state = newState;
  }
}

InputObject::InputObject(InputObject::type_t t, const char *n, string &s) : parent(0), type(t), name(n), id(s) {
}

//==========
//InputGroup
//==========

void InputGroup::attach(InputObject &object) {
  list.add(&object);
  object.parent = this;
}

void InputGroup::bind() {
  for(unsigned i = 0; i < list.size(); i++) list[i]->bind();
}

void InputGroup::clear() {
  for(unsigned i = 0; i < list.size(); i++) list[i]->state = 0;
}

void InputGroup::poll(const int16_t *table) {
  for(unsigned i = 0; i < list.size(); i++) list[i]->poll(table[list[i]->code]);
}

int16_t InputGroup::state(unsigned index) const {
  if(index < list.size()) return list[index]->state;
  return 0;
}

InputGroup::InputGroup(const char *n) : list(*this), name(n) {
}
