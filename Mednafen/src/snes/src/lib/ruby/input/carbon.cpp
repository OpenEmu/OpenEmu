namespace ruby {

class pInputCarbon {
public:
  bool cap(const string& name) {
    return false;
  }

  any get(const string& name) {
    return false;
  }

  bool set(const string& name, const any& value) {
    return false;
  }

  bool acquire() { return false; }
  bool unacquire() { return false; }
  bool acquired() { return false; }

  bool poll(int16_t *table) {
    memset(table, 0, Scancode::Limit * sizeof(int16_t));

    KeyMap keys;
    GetKeys(keys);
    uint8_t *keymap = (uint8_t*)keys;

    #define map(id, name) table[keyboard(0)[name]] = (bool)(keymap[id >> 3] & (1 << (id & 7)))
    map(0x35, Keyboard::Escape);

    map(0x7a, Keyboard::F1);
    map(0x78, Keyboard::F2);
    map(0x63, Keyboard::F3);
    map(0x76, Keyboard::F4);
    map(0x60, Keyboard::F5);
    map(0x61, Keyboard::F6);
    map(0x62, Keyboard::F7);
    map(0x64, Keyboard::F8);
    map(0x65, Keyboard::F9);
    map(0x6d, Keyboard::F10);
    map(0x67, Keyboard::F11);
  //map(0x??, Keyboard::F12);

    map(0x69, Keyboard::PrintScreen);
  //map(0x??, Keyboard::ScrollLock);
    map(0x71, Keyboard::Pause);

    map(0x32, Keyboard::Tilde);
    map(0x12, Keyboard::Num1);
    map(0x13, Keyboard::Num2);
    map(0x14, Keyboard::Num3);
    map(0x15, Keyboard::Num4);
    map(0x17, Keyboard::Num5);
    map(0x16, Keyboard::Num6);
    map(0x1a, Keyboard::Num7);
    map(0x1c, Keyboard::Num8);
    map(0x19, Keyboard::Num9);
    map(0x1d, Keyboard::Num0);

    map(0x1b, Keyboard::Dash);
    map(0x18, Keyboard::Equal);
    map(0x33, Keyboard::Backspace);

    map(0x72, Keyboard::Insert);
    map(0x75, Keyboard::Delete);
    map(0x73, Keyboard::Home);
    map(0x77, Keyboard::End);
    map(0x74, Keyboard::PageUp);
    map(0x79, Keyboard::PageDown);

    map(0x00, Keyboard::A);
    map(0x0b, Keyboard::B);
    map(0x08, Keyboard::C);
    map(0x02, Keyboard::D);
    map(0x0e, Keyboard::E);
    map(0x03, Keyboard::F);
    map(0x05, Keyboard::G);
    map(0x04, Keyboard::H);
    map(0x22, Keyboard::I);
    map(0x26, Keyboard::J);
    map(0x28, Keyboard::K);
    map(0x25, Keyboard::L);
    map(0x2e, Keyboard::M);
    map(0x2d, Keyboard::N);
    map(0x1f, Keyboard::O);
    map(0x23, Keyboard::P);
    map(0x0c, Keyboard::Q);
    map(0x0f, Keyboard::R);
    map(0x01, Keyboard::S);
    map(0x11, Keyboard::T);
    map(0x20, Keyboard::U);
    map(0x09, Keyboard::V);
    map(0x0d, Keyboard::W);
    map(0x07, Keyboard::X);
    map(0x10, Keyboard::Y);
    map(0x06, Keyboard::Z);

    map(0x21, Keyboard::LeftBracket);
    map(0x1e, Keyboard::RightBracket);
    map(0x2a, Keyboard::Backslash);
    map(0x29, Keyboard::Semicolon);
    map(0x27, Keyboard::Apostrophe);
    map(0x2b, Keyboard::Comma);
    map(0x2f, Keyboard::Period);
    map(0x2c, Keyboard::Slash);

    map(0x53, Keyboard::Keypad1);
    map(0x54, Keyboard::Keypad2);
    map(0x55, Keyboard::Keypad3);
    map(0x56, Keyboard::Keypad4);
    map(0x57, Keyboard::Keypad5);
    map(0x58, Keyboard::Keypad6);
    map(0x59, Keyboard::Keypad7);
    map(0x5b, Keyboard::Keypad8);
    map(0x5c, Keyboard::Keypad9);
    map(0x52, Keyboard::Keypad0);

  //map(0x??, Keyboard::Point);
    map(0x4c, Keyboard::Enter);
    map(0x45, Keyboard::Add);
    map(0x4e, Keyboard::Subtract);
    map(0x43, Keyboard::Multiply);
    map(0x4b, Keyboard::Divide);

    map(0x47, Keyboard::NumLock);
  //map(0x39, Keyboard::CapsLock);

    map(0x7e, Keyboard::Up);
    map(0x7d, Keyboard::Down);
    map(0x7b, Keyboard::Left);
    map(0x7c, Keyboard::Right);

    map(0x30, Keyboard::Tab);
    map(0x24, Keyboard::Return);
    map(0x31, Keyboard::Spacebar);
  //map(0x??, Keyboard::Menu);

    map(0x38, Keyboard::Shift);
    map(0x3b, Keyboard::Control);
    map(0x3a, Keyboard::Alt);
    map(0x37, Keyboard::Super);
    #undef map

    return true;
  }

  bool init() {
    return true;
  }

  void term() {
  }
};

DeclareInput(Carbon)

};
