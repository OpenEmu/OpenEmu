#include "../ui-base.hpp"

#include "controller.cpp"
#include "userinterface-general.cpp"
#include "userinterface-system.cpp"
#include "userinterface-emulationspeed.cpp"
#include "userinterface-states.cpp"
#include "userinterface-videosettings.cpp"

void MappedInput::bind() {
  lstring part;
  part.split("+", name);

  modifier = InputModifier::None;
  for(unsigned i = 0; i < part.size(); i++) {
    if(part[i] == "Shift") modifier |= InputModifier::Shift;
    if(part[i] == "Control") modifier |= InputModifier::Control;
    if(part[i] == "Alt") modifier |= InputModifier::Alt;
    if(part[i] == "Super") modifier |= InputModifier::Super;
  }

  string temp = part[part.size() - 1];
  part.split(".", temp);
  scancode = Scancode::decode(part[0]);
  specifier = InputSpecifier::None;
  if(part[1] == "Up") specifier = InputSpecifier::Up;
  if(part[1] == "Down") specifier = InputSpecifier::Down;
  if(part[1] == "Left") specifier = InputSpecifier::Left;
  if(part[1] == "Right") specifier = InputSpecifier::Right;
  if(part[1] == "Lo") specifier = InputSpecifier::Lo;
  if(part[1] == "Hi") specifier = InputSpecifier::Hi;
  if(part[1] == "Trigger") specifier = InputSpecifier::Trigger;

  //bypass modifier matching if scancode is itself a modifier
  modifierOverride = Keyboard::isAnyModifier(scancode);

  //re-encode name, in case previous name was invalid
  name = "";
  if(modifier & InputModifier::Shift) name << "Shift+";
  if(modifier & InputModifier::Control) name << "Control+";
  if(modifier & InputModifier::Alt) name << "Alt+";
  if(modifier & InputModifier::Super) name << "Super+";
  name << Scancode::encode(scancode);
  if(specifier == InputSpecifier::Up) name << ".Up";
  if(specifier == InputSpecifier::Down) name << ".Down";
  if(specifier == InputSpecifier::Left) name << ".Left";
  if(specifier == InputSpecifier::Right) name << ".Right";
  if(specifier == InputSpecifier::Lo) name << ".Lo";
  if(specifier == InputSpecifier::Hi) name << ".Hi";
  if(specifier == InputSpecifier::Trigger) name << ".Trigger";
}

void MappedInput::cache() {
  cachedState = state;
}

MappedInput::MappedInput(const char *label_, const char *configName) : parent(0), label(label_) {
  specifier = InputSpecifier::None;
  modifierOverride = false;
  state = 0;
  previousState = 0;
  cachedState = 0;
  config().attach(name = "None", configName);
}

//

void DigitalInput::poll() {
  previousState = state;
  if(modifier == mapper().modifier || modifierOverride) {
    if(specifier == InputSpecifier::None) {
      state = mapper().state(scancode);
    } else if(specifier == InputSpecifier::Up) {
      state = (bool)(mapper().state(scancode) & Joypad::HatUp);
    } else if(specifier == InputSpecifier::Down) {
      state = (bool)(mapper().state(scancode) & Joypad::HatDown);
    } else if(specifier == InputSpecifier::Left) {
      state = (bool)(mapper().state(scancode) & Joypad::HatLeft);
    } else if(specifier == InputSpecifier::Right) {
      state = (bool)(mapper().state(scancode) & Joypad::HatRight);
    } else if(specifier == InputSpecifier::Lo) {
      state = mapper().state(scancode) < -16384;
    } else if(specifier == InputSpecifier::Hi) {
      state = mapper().state(scancode) > +16384;
    } else if(specifier == InputSpecifier::Trigger) {
      state = mapper().state(scancode) < 0;
    }
  } else {
    state = 0;
  }
}

bool DigitalInput::isPressed() const { return state; }
bool DigitalInput::wasPressed() const { return previousState; }

DigitalInput::DigitalInput(const char *label, const char *configName) : MappedInput(label, configName) {
}

//

void AnalogInput::poll() {
  previousState = state;
  if(Mouse::isAnyAxis(scancode)) {
    if(input.acquired()) {
      state = mapper().state(scancode);
    } else {
      state = 0;
    }
  } else if(Joypad::isAnyAxis(scancode)) {
    state = mapper().state(scancode) / 8192;
  }
}

AnalogInput::AnalogInput(const char *label, const char *configName) : MappedInput(label, configName) {
}

//

void HotkeyInput::poll() {
  DigitalInput::poll();
  if(mainWindow->isActive() && state != previousState) {
    state ? pressed() : released();
  }
}

HotkeyInput::HotkeyInput(const char *label, const char *configName) : DigitalInput(label, configName) {
}

//

void InputGroup::attach(MappedInput *input) {
  input->parent = this;
  add(input);
}

void InputGroup::bind() {
  for(unsigned i = 0; i < size(); i++) {
    (*this)[i]->bind();
  }
}

void InputGroup::poll() {
  for(unsigned i = 0; i < size(); i++) {
    (*this)[i]->poll();
  }
}

void InputGroup::cache() {
  for(unsigned i = 0; i < size(); i++) {
    (*this)[i]->cache();
  }
}

void InputGroup::flushCache() {
  for(unsigned i = 0; i < size(); i++) {
    MappedInput &input = *((*this)[i]);
    input.cachedState = 0;
  }
}

InputGroup::InputGroup(unsigned category_, const char *label_) : category(category_), label(label_) {
  mapper().add(this);
}

//

InputMapper& mapper() {
  static InputMapper mapper;
  return mapper;
}

void InputMapper::calibrate() {
  calibrated = true;
  audio.clear();
  QMessageBox::information(settingsWindow, "Joypad Calibration",
    "Joypads must be calibrated prior to mapping. Please ensure that "
    "all axes and analog buttons are not pressed or moved in any specific "
    "direction, and then press ok."
  );

  poll();
  for(unsigned i = 0; i < Joypad::Count; i++) {
    for(unsigned axis = 0; axis < Joypad::Axes; axis++) {
      int16_t n = state(joypad(i).axis(axis));
      isTrigger[i][axis] = n < -16384 || n > +16384;
    }
  }
}

void InputMapper::bind() {
  for(unsigned i = 0; i < size(); i++) {
    (*this)[i]->bind();
  }
}

void InputMapper::poll() {
  activeState = !activeState;
  input.poll(stateTable[activeState]);

  modifier = 0;
  for(unsigned i = 0; i < Keyboard::Count; i++) {
    if(state(keyboard(i)[Keyboard::Shift])) modifier |= InputModifier::Shift;
    if(state(keyboard(i)[Keyboard::Control])) modifier |= InputModifier::Control;
    if(state(keyboard(i)[Keyboard::Alt])) modifier |= InputModifier::Alt;
    if(state(keyboard(i)[Keyboard::Super])) modifier |= InputModifier::Super;
  }

  for(unsigned i = 0; i < size(); i++) {
    (*this)[i]->poll();
  }

  for(unsigned i = 0; i < Scancode::Limit; i++) {
    if(state(i) != previousState(i)) {
      utility.inputEvent(i);
      diskBrowser->inputEvent(i);
      inputSettingsWindow->inputEvent(i);
    }
  }
}

void InputMapper::cache() {
  if(config().input.focusPolicy == Configuration::Input::FocusPolicyIgnoreInput && !mainWindow->isActive()) {
    for(unsigned i = 0; i < size(); i++) {
      InputGroup &group = *((*this)[i]);
      group.flushCache();
    }
  } else {
    for(unsigned i = 0; i < size(); i++) {
      InputGroup &group = *((*this)[i]);
      if(group.category == InputCategory::Port1 || group.category == InputCategory::Port2) {
        group.cache();
      }
    }
  }
}

int16_t InputMapper::status(bool port, unsigned device, unsigned index, unsigned id) {
  int16_t result = 0;

  if(port == InputCategory::Port1 && port1) result = port1->status(index, id);
  if(port == InputCategory::Port2 && port2) result = port2->status(index, id);

  if(movie.state == Movie::Playback) {
    result = movie.read();
  } else if(movie.state == Movie::Record) {
    movie.write(result);
  }

  return result;
}

string InputMapper::modifierString() const {
  string name;
  if(modifier & InputModifier::Shift) name << "Shift+";
  if(modifier & InputModifier::Control) name << "Control+";
  if(modifier & InputModifier::Alt) name << "Alt+";
  if(modifier & InputModifier::Super) name << "Super+";
  return name;
}

int16_t InputMapper::state(uint16_t scancode) const { return stateTable[activeState][scancode]; }
int16_t InputMapper::previousState(uint16_t scancode) const { return stateTable[!activeState][scancode]; }
unsigned InputMapper::distance(uint16_t scancode) const { return abs(state(scancode) - previousState(scancode)); }

InputMapper::InputMapper() : port1(0), port2(0) {
  calibrated = false;
  for(unsigned i = 0; i < Joypad::Count; i++) {
    for(unsigned axis = 0; axis < Joypad::Axes; axis++) {
      isTrigger[i][axis] = false;
    }
  }

  activeState = 0;
  for(unsigned i = 0; i < Scancode::Limit; i++) {
    stateTable[0][i] = stateTable[1][i] = 0;
  }
}
