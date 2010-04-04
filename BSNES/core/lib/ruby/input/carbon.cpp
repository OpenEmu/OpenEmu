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
    memset(table, 0, nall::input_limit * sizeof(int16_t));

    KeyMap keys;
    GetKeys(keys);
    uint8_t *keymap = (uint8_t*)keys;

    #define map(id, name) table[keyboard<0>::name] = (bool)(keymap[id >> 3] & (1 << (id & 7)))
    map(0x35, escape);

    map(0x7a, f1);
    map(0x78, f2);
    map(0x63, f3);
    map(0x76, f4);
    map(0x60, f5);
    map(0x61, f6);
    map(0x62, f7);
    map(0x64, f8);
    map(0x65, f9);
    map(0x6d, f10);
    map(0x67, f11);
  //map(0x??, f12);

    map(0x69, print_screen);
  //map(0x??, scroll_lock);
    map(0x71, pause);

    map(0x32, tilde);
    map(0x12, num_1);
    map(0x13, num_2);
    map(0x14, num_3);
    map(0x15, num_4);
    map(0x17, num_5);
    map(0x16, num_6);
    map(0x1a, num_7);
    map(0x1c, num_8);
    map(0x19, num_9);
    map(0x1d, num_0);

    map(0x1b, dash);
    map(0x18, equal);
    map(0x33, backspace);

    map(0x72, insert);
    map(0x75, delete_);
    map(0x73, home);
    map(0x77, end);
    map(0x74, page_up);
    map(0x79, page_down);

    map(0x00, a);
    map(0x0b, b);
    map(0x08, c);
    map(0x02, d);
    map(0x0e, e);
    map(0x03, f);
    map(0x05, g);
    map(0x04, h);
    map(0x22, i);
    map(0x26, j);
    map(0x28, k);
    map(0x25, l);
    map(0x2e, m);
    map(0x2d, n);
    map(0x1f, o);
    map(0x23, p);
    map(0x0c, q);
    map(0x0f, r);
    map(0x01, s);
    map(0x11, t);
    map(0x20, u);
    map(0x09, v);
    map(0x0d, w);
    map(0x07, x);
    map(0x10, y);
    map(0x06, z);

    map(0x21, lbracket);
    map(0x1e, rbracket);
    map(0x2a, backslash);
    map(0x29, semicolon);
    map(0x27, apostrophe);
    map(0x2b, comma);
    map(0x2f, period);
    map(0x2c, slash);

    map(0x52, pad_0);
    map(0x53, pad_1);
    map(0x54, pad_2);
    map(0x55, pad_3);
    map(0x56, pad_4);
    map(0x57, pad_5);
    map(0x58, pad_6);
    map(0x59, pad_7);
    map(0x5b, pad_8);
    map(0x5c, pad_9);

    map(0x45, add);
    map(0x4e, subtract);
    map(0x43, multiply);
    map(0x4b, divide);
    map(0x4c, enter);

    map(0x47, num_lock);
  //map(0x39, caps_lock);

    map(0x7e, up);
    map(0x7d, down);
    map(0x7b, left);
    map(0x7c, right);

    map(0x30, tab);
    map(0x24, return_);
    map(0x31, spacebar);

    map(0x3b, lctrl);
  //map(0x3b, rctrl);
    map(0x3a, lalt);
  //map(0x3a, ralt);
    map(0x38, lshift);
  //map(0x38, rshift);
    map(0x37, lsuper);
  //map(0x37, rsuper);
  //map(0x??, menu);
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
