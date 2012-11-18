#ifndef NALL_INPUT_HPP
#define NALL_INPUT_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nall/stdint.hpp>
#include <nall/string.hpp>

namespace nall {

struct Keyboard;
Keyboard& keyboard(unsigned = 0);

static const char KeyboardScancodeName[][64] = {
  "Escape", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
  "PrintScreen", "ScrollLock", "Pause", "Tilde",
  "Num1", "Num2", "Num3", "Num4", "Num5", "Num6", "Num7", "Num8", "Num9", "Num0",
  "Dash", "Equal", "Backspace",
  "Insert", "Delete", "Home", "End", "PageUp", "PageDown",
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
  "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
  "LeftBracket", "RightBracket", "Backslash", "Semicolon", "Apostrophe", "Comma", "Period", "Slash",
  "Keypad1", "Keypad2", "Keypad3", "Keypad4", "Keypad5", "Keypad6", "Keypad7", "Keypad8", "Keypad9", "Keypad0",
  "Point", "Enter", "Add", "Subtract", "Multiply", "Divide",
  "NumLock", "CapsLock",
  "Up", "Down", "Left", "Right",
  "Tab", "Return", "Spacebar", "Menu",
  "Shift", "Control", "Alt", "Super",
};

struct Keyboard {
  const unsigned ID;
  enum { Base = 1 };
  enum { Count = 8, Size = 128 };

  enum Scancode {
    Escape, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    PrintScreen, ScrollLock, Pause, Tilde,
    Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
    Dash, Equal, Backspace,
    Insert, Delete, Home, End, PageUp, PageDown,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    LeftBracket, RightBracket, Backslash, Semicolon, Apostrophe, Comma, Period, Slash,
    Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9, Keypad0,
    Point, Enter, Add, Subtract, Multiply, Divide,
    NumLock, CapsLock,
    Up, Down, Left, Right,
    Tab, Return, Spacebar, Menu,
    Shift, Control, Alt, Super,
    Limit,
  };

  static signed numberDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(keyboard(i).belongsTo(scancode)) return i;
    }
    return -1;
  }

  static signed keyDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(keyboard(i).isKey(scancode)) return scancode - keyboard(i).key(Escape);
    }
    return -1;
  }

  static signed modifierDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(keyboard(i).isModifier(scancode)) return scancode - keyboard(i).key(Shift);
    }
    return -1;
  }

  static bool isAnyKey(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(keyboard(i).isKey(scancode)) return true;
    }
    return false;
  }

  static bool isAnyModifier(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(keyboard(i).isModifier(scancode)) return true;
    }
    return false;
  }

  static uint16_t decode(const char *name) {
    string s(name);
    if(!strbegin(name, "KB")) return 0;
    s.ltrim("KB");
    unsigned id = decimal(s);
    auto pos = strpos(s, "::");
    if(!pos) return 0;
    s = substr(s, pos() + 2);
    for(unsigned i = 0; i < Limit; i++) {
      if(s == KeyboardScancodeName[i]) return Base + Size * id + i;
    }
    return 0;
  }

  string encode(uint16_t code) const {
    unsigned index = 0;
    for(unsigned i = 0; i < Count; i++) {
      if(code >= Base + Size * i && code < Base + Size * (i + 1)) {
        index = code - (Base + Size * i);
        break;
      }
    }
    return { "KB", ID, "::", KeyboardScancodeName[index] };
  }

  uint16_t operator[](Scancode code) const { return Base + ID * Size + code; }
  uint16_t key(unsigned id) const { return Base + Size * ID + id; }
  bool isKey(unsigned id) const { return id >= key(Escape) && id <= key(Menu); }
  bool isModifier(unsigned id) const { return id >= key(Shift) && id <= key(Super); }
  bool belongsTo(uint16_t scancode) const { return isKey(scancode) || isModifier(scancode); }

  Keyboard(unsigned ID_) : ID(ID_) {}
};

inline Keyboard& keyboard(unsigned id) {
  static Keyboard kb0(0), kb1(1), kb2(2), kb3(3), kb4(4), kb5(5), kb6(6), kb7(7);
  switch(id) { default:
    case 0: return kb0; case 1: return kb1; case 2: return kb2; case 3: return kb3;
    case 4: return kb4; case 5: return kb5; case 6: return kb6; case 7: return kb7;
  }
}

static const char MouseScancodeName[][64] = {
  "Xaxis", "Yaxis", "Zaxis",
  "Button0", "Button1", "Button2", "Button3", "Button4", "Button5", "Button6", "Button7",
};

struct Mouse;
Mouse& mouse(unsigned = 0);

struct Mouse {
  const unsigned ID;
  enum { Base = Keyboard::Base + Keyboard::Size * Keyboard::Count };
  enum { Count = 8, Size = 16 };
  enum { Axes = 3, Buttons = 8 };

  enum Scancode {
    Xaxis, Yaxis, Zaxis,
    Button0, Button1, Button2, Button3, Button4, Button5, Button6, Button7,
    Limit,
  };

  static signed numberDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(mouse(i).belongsTo(scancode)) return i;
    }
    return -1;
  }

  static signed axisDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(mouse(i).isAxis(scancode)) return scancode - mouse(i).axis(0);
    }
    return -1;
  }

  static signed buttonDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(mouse(i).isButton(scancode)) return scancode - mouse(i).button(0);
    }
    return -1;
  }

  static bool isAnyAxis(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(mouse(i).isAxis(scancode)) return true;
    }
    return false;
  }

  static bool isAnyButton(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(mouse(i).isButton(scancode)) return true;
    }
    return false;
  }

  static uint16_t decode(const char *name) {
    string s(name);
    if(!strbegin(name, "MS")) return 0;
    s.ltrim("MS");
    unsigned id = decimal(s);
    auto pos = strpos(s, "::");
    if(!pos) return 0;
    s = substr(s, pos() + 2);
    for(unsigned i = 0; i < Limit; i++) {
      if(s == MouseScancodeName[i]) return Base + Size * id + i;
    }
    return 0;
  }

  string encode(uint16_t code) const {
    unsigned index = 0;
    for(unsigned i = 0; i < Count; i++) {
      if(code >= Base + Size * i && code < Base + Size * (i + 1)) {
        index = code - (Base + Size * i);
        break;
      }
    }
    return { "MS", ID, "::", MouseScancodeName[index] };
  }

  uint16_t operator[](Scancode code) const { return Base + ID * Size + code; }
  uint16_t axis(unsigned id) const { return Base + Size * ID + Xaxis + id; }
  uint16_t button(unsigned id) const { return Base + Size * ID + Button0 + id; }
  bool isAxis(unsigned id) const { return id >= axis(0) && id <= axis(2); }
  bool isButton(unsigned id) const { return id >= button(0) && id <= button(7); }
  bool belongsTo(uint16_t scancode) const { return isAxis(scancode) || isButton(scancode); }

  Mouse(unsigned ID_) : ID(ID_) {}
};

inline Mouse& mouse(unsigned id) {
  static Mouse ms0(0), ms1(1), ms2(2), ms3(3), ms4(4), ms5(5), ms6(6), ms7(7);
  switch(id) { default:
    case 0: return ms0; case 1: return ms1; case 2: return ms2; case 3: return ms3;
    case 4: return ms4; case 5: return ms5; case 6: return ms6; case 7: return ms7;
  }
}

static const char JoypadScancodeName[][64] = {
  "Hat0", "Hat1", "Hat2", "Hat3", "Hat4", "Hat5", "Hat6", "Hat7",
  "Axis0", "Axis1", "Axis2", "Axis3", "Axis4", "Axis5", "Axis6", "Axis7",
  "Axis8", "Axis9", "Axis10", "Axis11", "Axis12", "Axis13", "Axis14", "Axis15",
  "Button0", "Button1", "Button2", "Button3", "Button4", "Button5", "Button6", "Button7",
  "Button8", "Button9", "Button10", "Button11", "Button12", "Button13", "Button14", "Button15",
  "Button16", "Button17", "Button18", "Button19", "Button20", "Button21", "Button22", "Button23",
  "Button24", "Button25", "Button26", "Button27", "Button28", "Button29", "Button30", "Button31",
};

struct Joypad;
Joypad& joypad(unsigned = 0);

struct Joypad {
  const unsigned ID;
  enum { Base = Mouse::Base + Mouse::Size * Mouse::Count };
  enum { Count = 8, Size = 64 };
  enum { Hats = 8, Axes = 16, Buttons = 32 };

  enum Scancode {
    Hat0, Hat1, Hat2, Hat3, Hat4, Hat5, Hat6, Hat7,
    Axis0, Axis1, Axis2, Axis3, Axis4, Axis5, Axis6, Axis7,
    Axis8, Axis9, Axis10, Axis11, Axis12, Axis13, Axis14, Axis15,
    Button0, Button1, Button2, Button3, Button4, Button5, Button6, Button7,
    Button8, Button9, Button10, Button11, Button12, Button13, Button14, Button15,
    Button16, Button17, Button18, Button19, Button20, Button21, Button22, Button23,
    Button24, Button25, Button26, Button27, Button28, Button29, Button30, Button31,
    Limit,
  };

  enum Hat { HatCenter = 0, HatUp = 1, HatRight = 2, HatDown = 4, HatLeft = 8 };

  static signed numberDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(joypad(i).belongsTo(scancode)) return i;
    }
    return -1;
  }

  static signed hatDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(joypad(i).isHat(scancode)) return scancode - joypad(i).hat(0);
    }
    return -1;
  }

  static signed axisDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(joypad(i).isAxis(scancode)) return scancode - joypad(i).axis(0);
    }
    return -1;
  }

  static signed buttonDecode(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(joypad(i).isButton(scancode)) return scancode - joypad(i).button(0);
    }
    return -1;
  }

  static bool isAnyHat(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(joypad(i).isHat(scancode)) return true;
    }
    return false;
  }

  static bool isAnyAxis(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(joypad(i).isAxis(scancode)) return true;
    }
    return false;
  }

  static bool isAnyButton(uint16_t scancode) {
    for(unsigned i = 0; i < Count; i++) {
      if(joypad(i).isButton(scancode)) return true;
    }
    return false;
  }

  static uint16_t decode(const char *name) {
    string s(name);
    if(!strbegin(name, "JP")) return 0;
    s.ltrim("JP");
    unsigned id = decimal(s);
    auto pos = strpos(s, "::");
    if(!pos) return 0;
    s = substr(s, pos() + 2);
    for(unsigned i = 0; i < Limit; i++) {
      if(s == JoypadScancodeName[i]) return Base + Size * id + i;
    }
    return 0;
  }

  string encode(uint16_t code) const {
    unsigned index = 0;
    for(unsigned i = 0; i < Count; i++) {
      if(code >= Base + Size * i && code < Base + Size * (i + 1)) {
        index = code - (Base + Size * i);
      }
    }
    return { "JP", ID, "::", JoypadScancodeName[index] };
  }

  uint16_t operator[](Scancode code) const { return Base + ID * Size + code; }
  uint16_t hat(unsigned id) const { return Base + Size * ID + Hat0 + id; }
  uint16_t axis(unsigned id) const { return Base + Size * ID + Axis0 + id; }
  uint16_t button(unsigned id) const { return Base + Size * ID + Button0 + id; }
  bool isHat(unsigned id) const { return id >= hat(0) && id <= hat(7); }
  bool isAxis(unsigned id) const { return id >= axis(0) && id <= axis(15); }
  bool isButton(unsigned id) const { return id >= button(0) && id <= button(31); }
  bool belongsTo(uint16_t scancode) const { return isHat(scancode) || isAxis(scancode) || isButton(scancode); }

  Joypad(unsigned ID_) : ID(ID_) {}
};

inline Joypad& joypad(unsigned id) {
  static Joypad jp0(0), jp1(1), jp2(2), jp3(3), jp4(4), jp5(5), jp6(6), jp7(7);
  switch(id) { default:
    case 0: return jp0; case 1: return jp1; case 2: return jp2; case 3: return jp3;
    case 4: return jp4; case 5: return jp5; case 6: return jp6; case 7: return jp7;
  }
}

struct Scancode {
  enum { None = 0, Limit = Joypad::Base + Joypad::Size * Joypad::Count };

  static uint16_t decode(const char *name) {
    uint16_t code;
    code = Keyboard::decode(name);
    if(code) return code;
    code = Mouse::decode(name);
    if(code) return code;
    code = Joypad::decode(name);
    if(code) return code;
    return None;
  }

  static string encode(uint16_t code) {
    for(unsigned i = 0; i < Keyboard::Count; i++) {
      if(keyboard(i).belongsTo(code)) return keyboard(i).encode(code);
    }
    for(unsigned i = 0; i < Mouse::Count; i++) {
      if(mouse(i).belongsTo(code)) return mouse(i).encode(code);
    }
    for(unsigned i = 0; i < Joypad::Count; i++) {
      if(joypad(i).belongsTo(code)) return joypad(i).encode(code);
    }
    return "None";
  }
};

}

#endif
