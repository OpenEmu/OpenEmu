#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace ruby {

static BOOL CALLBACK DI_EnumJoypadsCallback(const DIDEVICEINSTANCE*, void*);
static BOOL CALLBACK DI_EnumJoypadAxesCallback(const DIDEVICEOBJECTINSTANCE*, void*);

using namespace nall;

class pInputDI {
public:
  struct {
    LPDIRECTINPUT8 context;
    LPDIRECTINPUTDEVICE8 keyboard;
    LPDIRECTINPUTDEVICE8 mouse;
    LPDIRECTINPUTDEVICE8 gamepad[joypad<>::count];
    bool mouseacquired;
  } device;

  struct {
    HWND handle;
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

  bool set(const string& name, const any& value) {
    if(name == Input::Handle) {
      settings.handle = (HWND)any_cast<uintptr_t>(value);
      return true;
    }

    return false;
  }

  bool poll(int16_t *table) {
    memset(table, 0, nall::input_limit * sizeof(int16_t));

    //========
    //Keyboard
    //========

    if(device.keyboard) {
      uint8_t state[256];
      if(FAILED(device.keyboard->GetDeviceState(sizeof state, state))) {
        device.keyboard->Acquire();
        if(FAILED(device.keyboard->GetDeviceState(sizeof state, state))) {
          memset(state, 0, sizeof state);
        }
      }

      table[keyboard<0>::escape] = (bool)(state[0x01] & 0x80);
      table[keyboard<0>::f1    ] = (bool)(state[0x3b] & 0x80);
      table[keyboard<0>::f2    ] = (bool)(state[0x3c] & 0x80);
      table[keyboard<0>::f3    ] = (bool)(state[0x3d] & 0x80);
      table[keyboard<0>::f4    ] = (bool)(state[0x3e] & 0x80);
      table[keyboard<0>::f5    ] = (bool)(state[0x3f] & 0x80);
      table[keyboard<0>::f6    ] = (bool)(state[0x40] & 0x80);
      table[keyboard<0>::f7    ] = (bool)(state[0x41] & 0x80);
      table[keyboard<0>::f8    ] = (bool)(state[0x42] & 0x80);
      table[keyboard<0>::f9    ] = (bool)(state[0x43] & 0x80);
      table[keyboard<0>::f10   ] = (bool)(state[0x44] & 0x80);
      table[keyboard<0>::f11   ] = (bool)(state[0x57] & 0x80);
      table[keyboard<0>::f12   ] = (bool)(state[0x58] & 0x80);

      table[keyboard<0>::print_screen] = (bool)(state[0xb7] & 0x80);
      table[keyboard<0>::scroll_lock ] = (bool)(state[0x46] & 0x80);
      table[keyboard<0>::pause       ] = (bool)(state[0xc5] & 0x80);
      table[keyboard<0>::tilde       ] = (bool)(state[0x29] & 0x80);

      table[keyboard<0>::num_1] = (bool)(state[0x02] & 0x80);
      table[keyboard<0>::num_2] = (bool)(state[0x03] & 0x80);
      table[keyboard<0>::num_3] = (bool)(state[0x04] & 0x80);
      table[keyboard<0>::num_4] = (bool)(state[0x05] & 0x80);
      table[keyboard<0>::num_5] = (bool)(state[0x06] & 0x80);
      table[keyboard<0>::num_6] = (bool)(state[0x07] & 0x80);
      table[keyboard<0>::num_7] = (bool)(state[0x08] & 0x80);
      table[keyboard<0>::num_8] = (bool)(state[0x09] & 0x80);
      table[keyboard<0>::num_9] = (bool)(state[0x0a] & 0x80);
      table[keyboard<0>::num_0] = (bool)(state[0x0b] & 0x80);

      table[keyboard<0>::dash     ] = (bool)(state[0x0c] & 0x80);
      table[keyboard<0>::equal    ] = (bool)(state[0x0d] & 0x80);
      table[keyboard<0>::backspace] = (bool)(state[0x0e] & 0x80);

      table[keyboard<0>::insert   ] = (bool)(state[0xd2] & 0x80);
      table[keyboard<0>::delete_  ] = (bool)(state[0xd3] & 0x80);
      table[keyboard<0>::home     ] = (bool)(state[0xc7] & 0x80);
      table[keyboard<0>::end      ] = (bool)(state[0xcf] & 0x80);
      table[keyboard<0>::page_up  ] = (bool)(state[0xc9] & 0x80);
      table[keyboard<0>::page_down] = (bool)(state[0xd1] & 0x80);

      table[keyboard<0>::a] = (bool)(state[0x1e] & 0x80);
      table[keyboard<0>::b] = (bool)(state[0x30] & 0x80);
      table[keyboard<0>::c] = (bool)(state[0x2e] & 0x80);
      table[keyboard<0>::d] = (bool)(state[0x20] & 0x80);
      table[keyboard<0>::e] = (bool)(state[0x12] & 0x80);
      table[keyboard<0>::f] = (bool)(state[0x21] & 0x80);
      table[keyboard<0>::g] = (bool)(state[0x22] & 0x80);
      table[keyboard<0>::h] = (bool)(state[0x23] & 0x80);
      table[keyboard<0>::i] = (bool)(state[0x17] & 0x80);
      table[keyboard<0>::j] = (bool)(state[0x24] & 0x80);
      table[keyboard<0>::k] = (bool)(state[0x25] & 0x80);
      table[keyboard<0>::l] = (bool)(state[0x26] & 0x80);
      table[keyboard<0>::m] = (bool)(state[0x32] & 0x80);
      table[keyboard<0>::n] = (bool)(state[0x31] & 0x80);
      table[keyboard<0>::o] = (bool)(state[0x18] & 0x80);
      table[keyboard<0>::p] = (bool)(state[0x19] & 0x80);
      table[keyboard<0>::q] = (bool)(state[0x10] & 0x80);
      table[keyboard<0>::r] = (bool)(state[0x13] & 0x80);
      table[keyboard<0>::s] = (bool)(state[0x1f] & 0x80);
      table[keyboard<0>::t] = (bool)(state[0x14] & 0x80);
      table[keyboard<0>::u] = (bool)(state[0x16] & 0x80);
      table[keyboard<0>::v] = (bool)(state[0x2f] & 0x80);
      table[keyboard<0>::w] = (bool)(state[0x11] & 0x80);
      table[keyboard<0>::x] = (bool)(state[0x2d] & 0x80);
      table[keyboard<0>::y] = (bool)(state[0x15] & 0x80);
      table[keyboard<0>::z] = (bool)(state[0x2c] & 0x80);

      table[keyboard<0>::lbracket  ] = (bool)(state[0x1a] & 0x80);
      table[keyboard<0>::rbracket  ] = (bool)(state[0x1b] & 0x80);
      table[keyboard<0>::backslash ] = (bool)(state[0x2b] & 0x80);
      table[keyboard<0>::semicolon ] = (bool)(state[0x27] & 0x80);
      table[keyboard<0>::apostrophe] = (bool)(state[0x28] & 0x80);
      table[keyboard<0>::comma     ] = (bool)(state[0x33] & 0x80);
      table[keyboard<0>::period    ] = (bool)(state[0x34] & 0x80);
      table[keyboard<0>::slash     ] = (bool)(state[0x35] & 0x80);

      table[keyboard<0>::pad_0] = (bool)(state[0x4f] & 0x80);
      table[keyboard<0>::pad_1] = (bool)(state[0x50] & 0x80);
      table[keyboard<0>::pad_2] = (bool)(state[0x51] & 0x80);
      table[keyboard<0>::pad_3] = (bool)(state[0x4b] & 0x80);
      table[keyboard<0>::pad_4] = (bool)(state[0x4c] & 0x80);
      table[keyboard<0>::pad_5] = (bool)(state[0x4d] & 0x80);
      table[keyboard<0>::pad_6] = (bool)(state[0x47] & 0x80);
      table[keyboard<0>::pad_7] = (bool)(state[0x48] & 0x80);
      table[keyboard<0>::pad_8] = (bool)(state[0x49] & 0x80);
      table[keyboard<0>::pad_9] = (bool)(state[0x52] & 0x80);
      table[keyboard<0>::point] = (bool)(state[0x53] & 0x80);

      table[keyboard<0>::add]      = (bool)(state[0x4e] & 0x80);
      table[keyboard<0>::subtract] = (bool)(state[0x4a] & 0x80);
      table[keyboard<0>::multiply] = (bool)(state[0x37] & 0x80);
      table[keyboard<0>::divide]   = (bool)(state[0xb5] & 0x80);
      table[keyboard<0>::enter]    = (bool)(state[0x9c] & 0x80);

      table[keyboard<0>::num_lock ] = (bool)(state[0x45] & 0x80);
      table[keyboard<0>::caps_lock] = (bool)(state[0x3a] & 0x80);

      table[keyboard<0>::up   ] = (bool)(state[0xc8] & 0x80);
      table[keyboard<0>::down ] = (bool)(state[0xd0] & 0x80);
      table[keyboard<0>::left ] = (bool)(state[0xcb] & 0x80);
      table[keyboard<0>::right] = (bool)(state[0xcd] & 0x80);

      table[keyboard<0>::tab     ] = (bool)(state[0x0f] & 0x80);
      table[keyboard<0>::return_ ] = (bool)(state[0x1c] & 0x80);
      table[keyboard<0>::spacebar] = (bool)(state[0x39] & 0x80);

      table[keyboard<0>::lctrl ] = (bool)(state[0x1d] & 0x80);
      table[keyboard<0>::rctrl ] = (bool)(state[0x9d] & 0x80);
      table[keyboard<0>::lalt  ] = (bool)(state[0x38] & 0x80);
      table[keyboard<0>::ralt  ] = (bool)(state[0xb8] & 0x80);
      table[keyboard<0>::lshift] = (bool)(state[0x2a] & 0x80);
      table[keyboard<0>::rshift] = (bool)(state[0x36] & 0x80);
      table[keyboard<0>::lsuper] = (bool)(state[0xdb] & 0x80);
      table[keyboard<0>::rsuper] = (bool)(state[0xdc] & 0x80);
      table[keyboard<0>::menu  ] = (bool)(state[0xdd] & 0x80);
    }

    //=====
    //Mouse
    //=====

    if(device.mouse) {
      DIMOUSESTATE2 state;
      if(FAILED(device.mouse->GetDeviceState(sizeof(DIMOUSESTATE2), (void*)&state))) {
        device.mouse->Acquire();
        if(FAILED(device.mouse->GetDeviceState(sizeof(DIMOUSESTATE2), (void*)&state))) {
          memset(&state, 0, sizeof(DIMOUSESTATE2));
        }
      }

      table[mouse<0>::x] = state.lX;
      table[mouse<0>::y] = state.lY;
      table[mouse<0>::z] = state.lZ / WHEEL_DELTA;
      for(unsigned n = 0; n < mouse<>::buttons; n++) {
        table[mouse<0>::button + n] = (bool)state.rgbButtons[n];
      }

      //on Windows, 0 = left, 1 = right, 2 = middle
      //swap middle and right buttons for consistency with Linux
      int16_t temp = table[mouse<0>::button + 1];
      table[mouse<0>::button + 1] = table[mouse<0>::button + 2];
      table[mouse<0>::button + 2] = temp;
    }

    //=========
    //Joypad(s)
    //=========

    for(unsigned i = 0; i < joypad<>::count; i++) {
      if(!device.gamepad[i]) continue;
      unsigned index = joypad<>::index(i, joypad<>::none);

      if(FAILED(device.gamepad[i]->Poll())) {
        device.gamepad[i]->Acquire();
        continue;
      }

      DIJOYSTATE2 state;
      device.gamepad[i]->GetDeviceState(sizeof(DIJOYSTATE2), &state);

      //POV hats
      for(unsigned n = 0; n < min((unsigned)joypad<>::hats, 4); n++) {
        //POV value is in clockwise-hundredth degree units.
        unsigned pov = state.rgdwPOV[n];
        //some drivers report a centered POV hat as -1U, others as 65535U.
        //>= 36000 will match both, as well as invalid ranges.
        if(pov < 36000) {
          if(pov >= 31500 || pov <=  4500) table[index + joypad<>::hat + n] |= joypad<>::hat_up;
          if(pov >=  4500 && pov <= 13500) table[index + joypad<>::hat + n] |= joypad<>::hat_right;
          if(pov >= 13500 && pov <= 22500) table[index + joypad<>::hat + n] |= joypad<>::hat_down;
          if(pov >= 22500 && pov <= 31500) table[index + joypad<>::hat + n] |= joypad<>::hat_left;
        }
      }

      //axes
      table[index + joypad<>::axis + 0] = state.lX;
      table[index + joypad<>::axis + 1] = state.lY;
      table[index + joypad<>::axis + 2] = state.lZ;
      table[index + joypad<>::axis + 3] = state.lRx;
      table[index + joypad<>::axis + 4] = state.lRy;
      table[index + joypad<>::axis + 5] = state.lRz;

      //buttons
      for(unsigned n = 0; n < min((unsigned)joypad<>::buttons, 128); n++) {
        table[index + joypad<>::button + n] = (bool)state.rgbButtons[n];
      }
    }

    return true;
  }

  bool init_joypad(const DIDEVICEINSTANCE *instance) {
    unsigned n;
    for(n = 0; n < joypad<>::count; n++) { if(!device.gamepad[n]) break; }
    if(n >= joypad<>::count) return DIENUM_STOP;

    if(FAILED(device.context->CreateDevice(instance->guidInstance, &device.gamepad[n], 0))) {
      return DIENUM_CONTINUE;  //continue and try next gamepad
    }

    device.gamepad[n]->SetDataFormat(&c_dfDIJoystick2);
    device.gamepad[n]->SetCooperativeLevel(settings.handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    device.gamepad[n]->EnumObjects(DI_EnumJoypadAxesCallback, (void*)this, DIDFT_ABSAXIS);

    return DIENUM_CONTINUE;
  }

  bool init_axis(const DIDEVICEOBJECTINSTANCE *instance) {
    signed n;
    for(n = joypad<>::count - 1; n >= 0; n--) { if(device.gamepad[n]) break; }
    if(n < 0) return DIENUM_STOP;

    DIPROPRANGE range;
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow = DIPH_BYID;
    range.diph.dwObj = instance->dwType;
    range.lMin = -32768;
    range.lMax = +32767;
    device.gamepad[n]->SetProperty(DIPROP_RANGE, &range.diph);
    
    return DIENUM_CONTINUE;
  }

  bool init() {
    device.context = 0;
    device.keyboard = 0;
    device.mouse = 0;
    for(unsigned i = 0; i < joypad<>::count; i++) device.gamepad[i] = 0;
    device.mouseacquired = false;

    DirectInput8Create(GetModuleHandle(0), 0x0800, IID_IDirectInput8, (void**)&device.context, 0);

    device.context->CreateDevice(GUID_SysKeyboard, &device.keyboard, 0);
    device.keyboard->SetDataFormat(&c_dfDIKeyboard);
    device.keyboard->SetCooperativeLevel(settings.handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    device.keyboard->Acquire();

    device.context->CreateDevice(GUID_SysMouse, &device.mouse, 0);
    device.mouse->SetDataFormat(&c_dfDIMouse2);
    HRESULT hr = device.mouse->SetCooperativeLevel(settings.handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    device.mouse->Acquire();

    device.context->EnumDevices(DI8DEVCLASS_GAMECTRL, DI_EnumJoypadsCallback, (void*)this, DIEDFL_ATTACHEDONLY);

    return true;
  }

  void term() {
    if(device.keyboard) {
      device.keyboard->Unacquire();
      device.keyboard->Release();
      device.keyboard = 0;
    }

    if(device.mouse) {
      device.mouse->Unacquire();
      device.mouse->Release();
      device.mouse = 0;
    }

    for(unsigned i = 0; i < joypad<>::count; i++) {
      if(device.gamepad[i]) {
        device.gamepad[i]->Unacquire();
        device.gamepad[i]->Release();
        device.gamepad[i] = 0;
      }
    }

    if(device.context) {
      device.context->Release();
      device.context = 0;
    }
  }

  bool acquire() {
    if(!device.mouse) return false;
    if(acquired() == false) {
      device.mouse->Unacquire();
      device.mouse->SetCooperativeLevel(settings.handle, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
      device.mouse->Acquire();
      device.mouseacquired = true;
    }
    return true;
  }

  bool unacquire() {
    if(!device.mouse) return false;
    if(acquired() == true) {
      device.mouse->Unacquire();
      device.mouse->SetCooperativeLevel(settings.handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
      device.mouse->Acquire();
      device.mouseacquired = false;
    }
    return true;
  }

  bool acquired() {
    return device.mouseacquired;
  }

  pInputDI() {
    device.context = 0;
    device.keyboard = 0;
    device.mouse = 0;
    for(unsigned i = 0; i < joypad<>::count; i++) device.gamepad[i] = 0;
    device.mouseacquired = false;

    settings.handle = 0;
  }

  ~pInputDI() { term(); }
};

BOOL CALLBACK DI_EnumJoypadsCallback(const DIDEVICEINSTANCE *instance, void *p) {
  return ((pInputDI*)p)->init_joypad(instance);
}

BOOL CALLBACK DI_EnumJoypadAxesCallback(const DIDEVICEOBJECTINSTANCE *instance, void *p) {
  return ((pInputDI*)p)->init_axis(instance);
}

DeclareInput(DI)

};
