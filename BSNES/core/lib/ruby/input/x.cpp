#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

namespace ruby {

class pInputX {
public:
  Display *display;
  #include "xlibkeys.hpp"

  bool cap(const string& name) {
    if(name == Input::KeyboardSupport) return true;
    return false;
  }

  any get(const string& name) {
    return false;
  }

  bool set(const string& name, const any &value) {
    return false;
  }

  bool acquire() { return false; }
  bool unacquire() { return false; }
  bool acquired() { return false; }

  bool poll(int16_t *table) {
    memset(table, 0, input_limit * sizeof(int16_t));

    char state[32];
    XQueryKeymap(display, state);

    for(unsigned i = 0; i < keyboard<>::length; i++) {
      uint8_t code = keycode[i];
      if(code == 0) continue;  //unmapped
      table[i] = (bool)(state[code >> 3] & (1 << (code & 7)));
    }

    return true;
  }

  bool init() {
    init_keycodes();
    display = XOpenDisplay(0);
    return true;
  }

  void term() {
    XCloseDisplay(display);
  }
};

DeclareInput(X)

};
