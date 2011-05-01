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
    LPDIRECTINPUTDEVICE8 gamepad[Joypad::Count];
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
    memset(table, 0, Scancode::Limit * sizeof(int16_t));

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

      #define key(id) table[keyboard(0)[id]]

      key(Keyboard::Escape) = (bool)(state[0x01] & 0x80);
      key(Keyboard::F1    ) = (bool)(state[0x3b] & 0x80);
      key(Keyboard::F2    ) = (bool)(state[0x3c] & 0x80);
      key(Keyboard::F3    ) = (bool)(state[0x3d] & 0x80);
      key(Keyboard::F4    ) = (bool)(state[0x3e] & 0x80);
      key(Keyboard::F5    ) = (bool)(state[0x3f] & 0x80);
      key(Keyboard::F6    ) = (bool)(state[0x40] & 0x80);
      key(Keyboard::F7    ) = (bool)(state[0x41] & 0x80);
      key(Keyboard::F8    ) = (bool)(state[0x42] & 0x80);
      key(Keyboard::F9    ) = (bool)(state[0x43] & 0x80);
      key(Keyboard::F10   ) = (bool)(state[0x44] & 0x80);
      key(Keyboard::F11   ) = (bool)(state[0x57] & 0x80);
      key(Keyboard::F12   ) = (bool)(state[0x58] & 0x80);

      key(Keyboard::PrintScreen) = (bool)(state[0xb7] & 0x80);
      key(Keyboard::ScrollLock ) = (bool)(state[0x46] & 0x80);
      key(Keyboard::Pause      ) = (bool)(state[0xc5] & 0x80);
      key(Keyboard::Tilde      ) = (bool)(state[0x29] & 0x80);

      key(Keyboard::Num1) = (bool)(state[0x02] & 0x80);
      key(Keyboard::Num2) = (bool)(state[0x03] & 0x80);
      key(Keyboard::Num3) = (bool)(state[0x04] & 0x80);
      key(Keyboard::Num4) = (bool)(state[0x05] & 0x80);
      key(Keyboard::Num5) = (bool)(state[0x06] & 0x80);
      key(Keyboard::Num6) = (bool)(state[0x07] & 0x80);
      key(Keyboard::Num7) = (bool)(state[0x08] & 0x80);
      key(Keyboard::Num8) = (bool)(state[0x09] & 0x80);
      key(Keyboard::Num9) = (bool)(state[0x0a] & 0x80);
      key(Keyboard::Num0) = (bool)(state[0x0b] & 0x80);

      key(Keyboard::Dash     ) = (bool)(state[0x0c] & 0x80);
      key(Keyboard::Equal    ) = (bool)(state[0x0d] & 0x80);
      key(Keyboard::Backspace) = (bool)(state[0x0e] & 0x80);

      key(Keyboard::Insert  ) = (bool)(state[0xd2] & 0x80);
      key(Keyboard::Delete  ) = (bool)(state[0xd3] & 0x80);
      key(Keyboard::Home    ) = (bool)(state[0xc7] & 0x80);
      key(Keyboard::End     ) = (bool)(state[0xcf] & 0x80);
      key(Keyboard::PageUp  ) = (bool)(state[0xc9] & 0x80);
      key(Keyboard::PageDown) = (bool)(state[0xd1] & 0x80);

      key(Keyboard::A) = (bool)(state[0x1e] & 0x80);
      key(Keyboard::B) = (bool)(state[0x30] & 0x80);
      key(Keyboard::C) = (bool)(state[0x2e] & 0x80);
      key(Keyboard::D) = (bool)(state[0x20] & 0x80);
      key(Keyboard::E) = (bool)(state[0x12] & 0x80);
      key(Keyboard::F) = (bool)(state[0x21] & 0x80);
      key(Keyboard::G) = (bool)(state[0x22] & 0x80);
      key(Keyboard::H) = (bool)(state[0x23] & 0x80);
      key(Keyboard::I) = (bool)(state[0x17] & 0x80);
      key(Keyboard::J) = (bool)(state[0x24] & 0x80);
      key(Keyboard::K) = (bool)(state[0x25] & 0x80);
      key(Keyboard::L) = (bool)(state[0x26] & 0x80);
      key(Keyboard::M) = (bool)(state[0x32] & 0x80);
      key(Keyboard::N) = (bool)(state[0x31] & 0x80);
      key(Keyboard::O) = (bool)(state[0x18] & 0x80);
      key(Keyboard::P) = (bool)(state[0x19] & 0x80);
      key(Keyboard::Q) = (bool)(state[0x10] & 0x80);
      key(Keyboard::R) = (bool)(state[0x13] & 0x80);
      key(Keyboard::S) = (bool)(state[0x1f] & 0x80);
      key(Keyboard::T) = (bool)(state[0x14] & 0x80);
      key(Keyboard::U) = (bool)(state[0x16] & 0x80);
      key(Keyboard::V) = (bool)(state[0x2f] & 0x80);
      key(Keyboard::W) = (bool)(state[0x11] & 0x80);
      key(Keyboard::X) = (bool)(state[0x2d] & 0x80);
      key(Keyboard::Y) = (bool)(state[0x15] & 0x80);
      key(Keyboard::Z) = (bool)(state[0x2c] & 0x80);

      key(Keyboard::LeftBracket ) = (bool)(state[0x1a] & 0x80);
      key(Keyboard::RightBracket) = (bool)(state[0x1b] & 0x80);
      key(Keyboard::Backslash   ) = (bool)(state[0x2b] & 0x80);
      key(Keyboard::Semicolon   ) = (bool)(state[0x27] & 0x80);
      key(Keyboard::Apostrophe  ) = (bool)(state[0x28] & 0x80);
      key(Keyboard::Comma       ) = (bool)(state[0x33] & 0x80);
      key(Keyboard::Period      ) = (bool)(state[0x34] & 0x80);
      key(Keyboard::Slash       ) = (bool)(state[0x35] & 0x80);

      key(Keyboard::Keypad0) = (bool)(state[0x4f] & 0x80);
      key(Keyboard::Keypad1) = (bool)(state[0x50] & 0x80);
      key(Keyboard::Keypad2) = (bool)(state[0x51] & 0x80);
      key(Keyboard::Keypad3) = (bool)(state[0x4b] & 0x80);
      key(Keyboard::Keypad4) = (bool)(state[0x4c] & 0x80);
      key(Keyboard::Keypad5) = (bool)(state[0x4d] & 0x80);
      key(Keyboard::Keypad6) = (bool)(state[0x47] & 0x80);
      key(Keyboard::Keypad7) = (bool)(state[0x48] & 0x80);
      key(Keyboard::Keypad8) = (bool)(state[0x49] & 0x80);
      key(Keyboard::Keypad9) = (bool)(state[0x52] & 0x80);
      key(Keyboard::Point  ) = (bool)(state[0x53] & 0x80);

      key(Keyboard::Add     ) = (bool)(state[0x4e] & 0x80);
      key(Keyboard::Subtract) = (bool)(state[0x4a] & 0x80);
      key(Keyboard::Multiply) = (bool)(state[0x37] & 0x80);
      key(Keyboard::Divide  ) = (bool)(state[0xb5] & 0x80);
      key(Keyboard::Enter   ) = (bool)(state[0x9c] & 0x80);

      key(Keyboard::NumLock ) = (bool)(state[0x45] & 0x80);
      key(Keyboard::CapsLock) = (bool)(state[0x3a] & 0x80);

      key(Keyboard::Up   ) = (bool)(state[0xc8] & 0x80);
      key(Keyboard::Down ) = (bool)(state[0xd0] & 0x80);
      key(Keyboard::Left ) = (bool)(state[0xcb] & 0x80);
      key(Keyboard::Right) = (bool)(state[0xcd] & 0x80);

      key(Keyboard::Tab     ) = (bool)(state[0x0f] & 0x80);
      key(Keyboard::Return  ) = (bool)(state[0x1c] & 0x80);
      key(Keyboard::Spacebar) = (bool)(state[0x39] & 0x80);
      key(Keyboard::Menu   ) = (bool)(state[0xdd] & 0x80);

      key(Keyboard::Shift  ) = (bool)(state[0x2a] & 0x80) || (bool)(state[0x36] & 0x80);
      key(Keyboard::Control) = (bool)(state[0x1d] & 0x80) || (bool)(state[0x9d] & 0x80);
      key(Keyboard::Alt    ) = (bool)(state[0x38] & 0x80) || (bool)(state[0xb8] & 0x80);
      key(Keyboard::Super  ) = (bool)(state[0xdb] & 0x80) || (bool)(state[0xdc] & 0x80);

      #undef key
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

      table[mouse(0).axis(0)] = state.lX;
      table[mouse(0).axis(1)] = state.lY;
      table[mouse(0).axis(2)] = state.lZ / WHEEL_DELTA;
      for(unsigned n = 0; n < Mouse::Buttons; n++) {
        table[mouse(0).button(n)] = (bool)state.rgbButtons[n];
      }

      //on Windows, 0 = left, 1 = right, 2 = middle
      //swap middle and right buttons for consistency with Linux
      int16_t temp = table[mouse(0).button(1)];
      table[mouse(0).button(1)] = table[mouse(0).button(2)];
      table[mouse(0).button(2)] = temp;
    }

    //=========
    //Joypad(s)
    //=========

    for(unsigned i = 0; i < Joypad::Count; i++) {
      if(!device.gamepad[i]) continue;

      if(FAILED(device.gamepad[i]->Poll())) {
        device.gamepad[i]->Acquire();
        continue;
      }

      DIJOYSTATE2 state;
      device.gamepad[i]->GetDeviceState(sizeof(DIJOYSTATE2), &state);

      //POV hats
      for(unsigned n = 0; n < min((unsigned)Joypad::Hats, 4); n++) {
        //POV value is in clockwise-hundredth degree units.
        unsigned pov = state.rgdwPOV[n];
        //some drivers report a centered POV hat as -1U, others as 65535U.
        //>= 36000 will match both, as well as invalid ranges.
        if(pov < 36000) {
          if(pov >= 31500 || pov <=  4500) table[joypad(i).hat(n)] |= Joypad::HatUp;
          if(pov >=  4500 && pov <= 13500) table[joypad(i).hat(n)] |= Joypad::HatRight;
          if(pov >= 13500 && pov <= 22500) table[joypad(i).hat(n)] |= Joypad::HatDown;
          if(pov >= 22500 && pov <= 31500) table[joypad(i).hat(n)] |= Joypad::HatLeft;
        }
      }

      //axes
      table[joypad(i).axis(0)] = state.lX;
      table[joypad(i).axis(1)] = state.lY;
      table[joypad(i).axis(2)] = state.lZ;
      table[joypad(i).axis(3)] = state.lRx;
      table[joypad(i).axis(4)] = state.lRy;
      table[joypad(i).axis(5)] = state.lRz;

      //buttons
      for(unsigned n = 0; n < min((unsigned)Joypad::Buttons, 128); n++) {
        table[joypad(i).button(n)] = (bool)state.rgbButtons[n];
      }
    }

    return true;
  }

  bool init_joypad(const DIDEVICEINSTANCE *instance) {
    unsigned n;
    for(n = 0; n < Joypad::Count; n++) { if(!device.gamepad[n]) break; }
    if(n >= Joypad::Count) return DIENUM_STOP;

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
    for(n = Joypad::Count - 1; n >= 0; n--) { if(device.gamepad[n]) break; }
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
    for(unsigned i = 0; i < Joypad::Count; i++) device.gamepad[i] = 0;
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

    for(unsigned i = 0; i < Joypad::Count; i++) {
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
    for(unsigned i = 0; i < Joypad::Count; i++) device.gamepad[i] = 0;
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
