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
    memset(table, 0, Scancode::Limit * sizeof(int16_t));
    x_poll(table);
    return true;
  }

  bool init() {
    x_init();
    display = XOpenDisplay(0);
    return true;
  }

  void term() {
    XCloseDisplay(display);
  }
};

DeclareInput(X)

};
