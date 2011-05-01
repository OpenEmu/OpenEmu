//================
//SDL input driver
//================
//Keyboard and mouse are controlled directly via Xlib,
//as SDL cannot capture input from windows it does not create itself.
//SDL is used only to handle joysticks / gamepads.

#include <SDL/SDL.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace ruby {

struct pInputSDL {
  #include "xlibkeys.hpp"

  struct {
    Display *display;
    Window rootwindow;
    Cursor InvisibleCursor;
    SDL_Joystick *gamepad[Joypad::Count];

    unsigned screenwidth, screenheight;
    unsigned relativex, relativey;
    bool mouseacquired;

    //mouse device settings
    int accel_numerator;
    int accel_denominator;
    int threshold;
  } device;

  struct {
    uintptr_t handle;
  } settings;

  bool cap(const string& name) {
    if(name == Input::Handle) return true;
    if(name == Input::KeyboardSupport) return true;
    if(name == Input::MouseSupport) return true;
    if(name == Input::JoypadSupport) return true;
    return false;
  }

  any get(const string& name) {
    if(name == Input::Handle) return (uintptr_t)settings.handle;
    return false;
  }

  bool set(const string& name, const any &value) {
    if(name == Input::Handle) {
      settings.handle = any_cast<uintptr_t>(value);
      return true;
    }

    return false;
  }

  bool acquire() {
    if(acquired()) return true;

    if(XGrabPointer(device.display, settings.handle, True, 0, GrabModeAsync, GrabModeAsync,
    device.rootwindow, device.InvisibleCursor, CurrentTime) == GrabSuccess) {
      //backup existing cursor acceleration settings
      XGetPointerControl(device.display, &device.accel_numerator, &device.accel_denominator, &device.threshold);

      //disable cursor acceleration
      XChangePointerControl(device.display, True, False, 1, 1, 0);

      //center cursor (so that first relative poll returns 0, 0 if mouse has not moved)
      XWarpPointer(device.display, None, device.rootwindow, 0, 0, 0, 0, device.screenwidth / 2, device.screenheight / 2);

      return device.mouseacquired = true;
    } else {
      return device.mouseacquired = false;
    }
  }

  bool unacquire() {
    if(acquired()) {
      //restore cursor acceleration and release cursor
      XChangePointerControl(device.display, True, True, device.accel_numerator, device.accel_denominator, device.threshold);
      XUngrabPointer(device.display, CurrentTime);
      device.mouseacquired = false;
    }
    return true;
  }

  bool acquired() {
    return device.mouseacquired;
  }

  bool poll(int16_t *table) {
    memset(table, 0, Scancode::Limit * sizeof(int16_t));

    //========
    //Keyboard
    //========

    x_poll(table);

    //=====
    //Mouse
    //=====

    Window root_return, child_return;
    int root_x_return = 0, root_y_return = 0;
    int win_x_return = 0, win_y_return = 0;
    unsigned int mask_return = 0;
    XQueryPointer(device.display, settings.handle,
      &root_return, &child_return, &root_x_return, &root_y_return,
      &win_x_return, &win_y_return, &mask_return);

    if(acquired()) {
      XWindowAttributes attributes;
      XGetWindowAttributes(device.display, settings.handle, &attributes);

      //absolute -> relative conversion
      table[mouse(0).axis(0)] = (int16_t)(root_x_return - device.screenwidth  / 2);
      table[mouse(0).axis(1)] = (int16_t)(root_y_return - device.screenheight / 2);

      if(table[mouse(0).axis(0)] != 0 || table[mouse(0).axis(1)] != 0) {
        //if mouse movement occurred, re-center mouse for next poll
        XWarpPointer(device.display, None, device.rootwindow, 0, 0, 0, 0, device.screenwidth / 2, device.screenheight / 2);
      }
    } else {
      table[mouse(0).axis(0)] = (int16_t)(root_x_return - device.relativex);
      table[mouse(0).axis(1)] = (int16_t)(root_y_return - device.relativey);

      device.relativex = root_x_return;
      device.relativey = root_y_return;
    }

    //manual device polling is limited to only five buttons ...
    table[mouse(0).button(0)] = (bool)(mask_return & Button1Mask);
    table[mouse(0).button(1)] = (bool)(mask_return & Button2Mask);
    table[mouse(0).button(2)] = (bool)(mask_return & Button3Mask);
    table[mouse(0).button(3)] = (bool)(mask_return & Button4Mask);
    table[mouse(0).button(4)] = (bool)(mask_return & Button5Mask);

    //=========
    //Joypad(s)
    //=========

    SDL_JoystickUpdate();
    for(unsigned i = 0; i < Joypad::Count; i++) {
      if(!device.gamepad[i]) continue;

      //POV hats
      unsigned hats = min((unsigned)Joypad::Hats, SDL_JoystickNumHats(device.gamepad[i]));
      for(unsigned hat = 0; hat < hats; hat++) {
        uint8_t state = SDL_JoystickGetHat(device.gamepad[i], hat);
        if(state & SDL_HAT_UP   ) table[joypad(i).hat(hat)] |= Joypad::HatUp;
        if(state & SDL_HAT_RIGHT) table[joypad(i).hat(hat)] |= Joypad::HatRight;
        if(state & SDL_HAT_DOWN ) table[joypad(i).hat(hat)] |= Joypad::HatDown;
        if(state & SDL_HAT_LEFT ) table[joypad(i).hat(hat)] |= Joypad::HatLeft;
      }

      //axes
      unsigned axes = min((unsigned)Joypad::Axes, SDL_JoystickNumAxes(device.gamepad[i]));
      for(unsigned axis = 0; axis < axes; axis++) {
        table[joypad(i).axis(axis)] = (int16_t)SDL_JoystickGetAxis(device.gamepad[i], axis);
      }

      //buttons
      for(unsigned button = 0; button < Joypad::Buttons; button++) {
        table[joypad(i).button(button)] = (bool)SDL_JoystickGetButton(device.gamepad[i], button);
      }
    }

    return true;
  }

  bool init() {
    x_init();
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_IGNORE);

    device.display = XOpenDisplay(0);
    device.rootwindow = DefaultRootWindow(device.display);
    XWindowAttributes attributes;
    XGetWindowAttributes(device.display, device.rootwindow, &attributes);
    device.screenwidth  = attributes.width;
    device.screenheight = attributes.height;

    //Xlib: "because XShowCursor(false) would be too easy."
    //create a fully transparent cursor named InvisibleCursor,
    //for use while acquire() / XGrabPointer() is active.
    Pixmap pixmap;
    XColor black, unused;
    static char invisible_data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    Colormap colormap = DefaultColormap(device.display, DefaultScreen(device.display));
    XAllocNamedColor(device.display, colormap, "black", &black, &unused);
    pixmap = XCreateBitmapFromData(device.display, settings.handle, invisible_data, 8, 8);
    device.InvisibleCursor = XCreatePixmapCursor(device.display, pixmap, pixmap, &black, &black, 0, 0);
    XFreePixmap(device.display, pixmap);
    XFreeColors(device.display, colormap, &black.pixel, 1, 0);

    device.mouseacquired = false;
    device.relativex = 0;
    device.relativey = 0;

    unsigned joypads = min((unsigned)Joypad::Count, SDL_NumJoysticks());
    for(unsigned i = 0; i < joypads; i++) device.gamepad[i] = SDL_JoystickOpen(i);

    return true;
  }

  void term() {
    unacquire();
    XFreeCursor(device.display, device.InvisibleCursor);

    for(unsigned i = 0; i < Joypad::Count; i++) {
      if(device.gamepad[i]) SDL_JoystickClose(device.gamepad[i]);
      device.gamepad[i] = 0;
    }

    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    XCloseDisplay(device.display);
  }

  pInputSDL() {
    for(unsigned i = 0; i < Joypad::Count; i++) device.gamepad[i] = 0;
    settings.handle = 0;
  }
};

DeclareInput(SDL)

};
