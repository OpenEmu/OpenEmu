#ifndef NALL_INPUT_HPP
#define NALL_INPUT_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nall/stdint.hpp>

namespace nall {
  enum { input_none = 0 };

  template<int number = -1> struct keyboard {
    enum {
      none = keyboard<number - 1>::limit,
      escape, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
      print_screen, scroll_lock, pause, tilde,
      num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9, num_0,
      dash, equal, backspace,
      insert, delete_, home, end, page_up, page_down,
      a, b, c, d, e, f, g, h, i, j, k, l, m,
      n, o, p, q, r, s, t, u, v, w, x, y, z,
      lbracket, rbracket, backslash, semicolon, apostrophe, comma, period, slash,
      pad_1, pad_2, pad_3, pad_4, pad_5, pad_6, pad_7, pad_8, pad_9, pad_0,
      point, enter, add, subtract, multiply, divide,
      num_lock, caps_lock,
      up, down, left, right,
      tab, return_, spacebar,
      lctrl, rctrl, lalt, ralt, lshift, rshift, lsuper, rsuper, menu,
      limit,
    };
  };

  template<> struct keyboard<-1> {
    enum { count = 16 };
    enum {
      none,
      escape, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
      print_screen, scroll_lock, pause, tilde,
      num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9, num_0,
      dash, equal, backspace,
      insert, delete_, home, end, page_up, page_down,
      a, b, c, d, e, f, g, h, i, j, k, l, m,
      n, o, p, q, r, s, t, u, v, w, x, y, z,
      lbracket, rbracket, backslash, semicolon, apostrophe, comma, period, slash,
      pad_1, pad_2, pad_3, pad_4, pad_5, pad_6, pad_7, pad_8, pad_9, pad_0,
      point, enter, add, subtract, multiply, divide,
      num_lock, caps_lock,
      up, down, left, right,
      tab, return_, spacebar,
      lctrl, rctrl, lalt, ralt, lshift, rshift, lsuper, rsuper, menu,
      length,  //number of syms per keyboard
      limit = 0,
    };

    static uint16_t index(unsigned keyboard_number, unsigned keyboard_enum) {
      if(keyboard_number >= count) return input_none;
      return limit + keyboard_number * length + keyboard_enum;
    }
  };

  template<int number = -1> struct mouse {
    enum { buttons = 8 };
    enum {
      none = mouse<number - 1>::limit,
      x, y, z,
      button,
      limit = button + buttons,
    };
  };

  template<> struct mouse<-1> {
    enum { count = 16, buttons = 8 };
    enum {
      none,
      x, y, z,
      button,
      length = button + buttons - none,  //number of syms per mouse
      limit = keyboard<keyboard<>::count - 1>::limit,
    };

    static uint16_t index(unsigned mouse_number, unsigned mouse_enum) {
      if(mouse_number >= count) return input_none;
      return limit + mouse_number * length + mouse_enum;
    }
  };

  template<int number = -1> struct joypad {
    enum { hats = 8, axes = 32, buttons = 96 };
    enum {
      none = joypad<number - 1>::limit,
      hat,
      axis = hat + hats,
      button = axis + axes,
      limit = button + buttons,
    };
  };

  template<> struct joypad<-1> {
    enum { count = 16, hats = 8, axes = 32, buttons = 96 };
    enum { hat_center = 0, hat_up = 1, hat_right = 2, hat_down = 4, hat_left = 8 };
    enum {
      none,
      hat,
      axis = hat + hats,
      button = axis + axes,
      length = button + buttons - none,  //number of syms per joypad
      limit = mouse<mouse<>::count - 1>::limit,
    };

    static uint16_t index(unsigned joypad_number, unsigned joypad_enum) {
      if(joypad_number >= count) return input_none;
      return limit + joypad_number * length + joypad_enum;
    }
  };

  enum { input_limit = joypad<joypad<>::count - 1>::limit };

  static const char keysym[][64] = {
    "none",
    "escape", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
    "print_screen", "scroll_lock", "pause", "tilde",
    "num_1", "num_2", "num_3", "num_4", "num_5", "num_6", "num_7", "num_8", "num_9", "num_0",
    "dash", "equal", "backspace",
    "insert", "delete", "home", "end", "page_up", "page_down",
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    "lbracket", "rbracket", "backslash", "semicolon", "apostrophe", "comma", "period", "slash",
    "pad_1", "pad_2", "pad_3", "pad_4", "pad_5", "pad_6", "pad_7", "pad_8", "pad_9", "pad_0",
    "point", "enter", "add", "subtract", "multiply", "divide",
    "num_lock", "caps_lock",
    "up", "down", "left", "right",
    "tab", "return", "spacebar",
    "lctrl", "rctrl", "lalt", "ralt", "lshift", "rshift", "lsuper", "rsuper", "menu",
    "limit",
  };

  static const char* input_find(uint16_t key) {
    static char buffer[64];

    for(unsigned k = 0; k < keyboard<>::count; k++) {
      if(key >= keyboard<>::index(k, keyboard<>::none) && key < keyboard<>::index(k, keyboard<>::length)) {
        sprintf(buffer, "keyboard%.2d.%s", k, keysym[key - keyboard<>::index(k, keyboard<>::none)]);
        return buffer;
      }
    }

    for(unsigned m = 0; m < mouse<>::count; m++) {
      if(key == mouse<>::index(m, mouse<>::x)) { sprintf(buffer, "mouse%.2d.x", m); return buffer; }
      if(key == mouse<>::index(m, mouse<>::y)) { sprintf(buffer, "mouse%.2d.y", m); return buffer; }
      if(key == mouse<>::index(m, mouse<>::z)) { sprintf(buffer, "mouse%.2d.z", m); return buffer; }

      if(key >= mouse<>::index(m, mouse<>::button + 0)
      && key <  mouse<>::index(m, mouse<>::button + mouse<>::buttons)) {
        sprintf(buffer, "mouse%.2d.button%.2d", m, key - mouse<>::index(m, mouse<>::button));
        return buffer;
      }
    }

    for(unsigned j = 0; j < joypad<>::count; j++) {
      if(key >= joypad<>::index(j, joypad<>::hat + 0)
      && key <  joypad<>::index(j, joypad<>::hat + joypad<>::hats)) {
        sprintf(buffer, "joypad%.2d.hat%.2d", j, key - joypad<>::index(j, joypad<>::hat));
        return buffer;
      }

      if(key >= joypad<>::index(j, joypad<>::axis + 0)
      && key <  joypad<>::index(j, joypad<>::axis + joypad<>::axes)) {
        sprintf(buffer, "joypad%.2d.axis%.2d", j, key - joypad<>::index(j, joypad<>::axis));
        return buffer;
      }

      if(key >= joypad<>::index(j, joypad<>::button + 0)
      && key <  joypad<>::index(j, joypad<>::button + joypad<>::buttons)) {
        sprintf(buffer, "joypad%.2d.button%.2d", j, key - joypad<>::index(j, joypad<>::button));
        return buffer;
      }
    }

    return "none";
  }

  static char* input_find(char *out, uint16_t key) {
    strcpy(out, input_find(key));
    return out;
  }

  static uint16_t input_find(const char *key) {
    if(!memcmp(key, "keyboard", 8)) {
      key += 8;
      if(!*key || !*(key + 1)) return input_none;
      uint8_t k = (*key - '0') * 10 + (*(key + 1) - '0');
      if(k >= keyboard<>::count) return input_none;
      key += 2;

      if(*key++ != '.') return input_none;

      for(unsigned i = 0; i < keyboard<>::length; i++) {
        if(!strcmp(key, keysym[i])) return keyboard<>::index(k, i);
      }
    }

    if(!memcmp(key, "mouse", 5)) {
      key += 5;
      if(!*key || !*(key + 1)) return input_none;
      uint8_t m = (*key - '0') * 10 + (*(key + 1) - '0');
      if(m >= mouse<>::count) return input_none;
      key += 2;

      if(!strcmp(key, ".x")) return mouse<>::index(m, mouse<>::x);
      if(!strcmp(key, ".y")) return mouse<>::index(m, mouse<>::y);
      if(!strcmp(key, ".z")) return mouse<>::index(m, mouse<>::z);

      if(!memcmp(key, ".button", 7)) {
        key += 7;
        if(!*key || !*(key + 1)) return input_none;
        uint8_t button = (*key - '0') * 10 + (*(key + 1) - '0');
        if(button >= mouse<>::buttons) return input_none;
        return mouse<>::index(m, mouse<>::button + button);
      }

      return input_none;
    }

    if(!memcmp(key, "joypad", 6)) {
      key += 6;
      if(!*key || !*(key + 1)) return input_none;
      uint8_t j = (*key - '0') * 10 + (*(key + 1) - '0');
      if(j >= joypad<>::count) return input_none;
      key += 2;

      if(!memcmp(key, ".hat", 4)) {
        key += 4;
        if(!*key || !*(key + 1)) return input_none;
        uint8_t hat = (*key - '0') * 10 + (*(key + 1) - '0');
        if(hat >= joypad<>::hats) return input_none;
        return joypad<>::index(j, joypad<>::hat + hat);
      }

      if(!memcmp(key, ".axis", 5)) {
        key += 5;
        if(!*key || !*(key + 1)) return input_none;
        uint8_t axis = (*key - '0') * 10 + (*(key + 1) - '0');
        if(axis >= joypad<>::axes) return input_none;
        return joypad<>::index(j, joypad<>::axis + axis);
      }

      if(!memcmp(key, ".button", 7)) {
        key += 7;
        if(!*key || !*(key + 1)) return input_none;
        uint8_t button = (*key - '0') * 10 + (*(key + 1) - '0');
        if(button >= joypad<>::buttons) return input_none;
        return joypad<>::index(j, joypad<>::button + button);
      }

      return input_none;
    }

    return input_none;
  }
}

#endif
