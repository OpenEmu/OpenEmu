#define None XNone
#define Window XWindow
#include <X11/Xlib.h>
#undef None
#undef Window

struct LibXtst : public library {
  function<int (Display*, unsigned int, Bool, unsigned long)> XTestFakeKeyEvent;

  LibXtst() {
    if(open("Xtst")) {
      XTestFakeKeyEvent = sym("XTestFakeKeyEvent");
    }
  }
} libXtst;

char* userpath(char *path) {
  *path = 0;
  struct passwd *userinfo = getpwuid(getuid());
  if(userinfo) strcpy(path, userinfo->pw_dir);
  return path;
}

char *getcwd(char *path) {
  return getcwd(path, PATH_MAX);
}

void initargs(int &argc, char **&argv) {
}

void supressScreenSaver() {
  if(!libXtst.XTestFakeKeyEvent) return;

  //XSetScreenSaver(timeout = 0) does not work
  //XResetScreenSaver() does not work
  //XScreenSaverSuspend() does not work
  //DPMSDisable() does not work
  //XSendEvent(KeyPressMask) does not work
  //use XTest extension to send fake keypress every ~20 seconds.
  //keycode of 255 does not map to any actual key,
  //but it will block screensaver and power management.
  Display *display = XOpenDisplay(0);
  libXtst.XTestFakeKeyEvent(display, 255, True,  0);
  libXtst.XTestFakeKeyEvent(display, 255, False, 0);
  XCloseDisplay(display);
}

