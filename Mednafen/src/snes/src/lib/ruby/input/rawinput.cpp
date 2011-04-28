//RawInput driver
//author: byuu

//this driver utilizes RawInput (WM_INPUT) to capture keyboard and mouse input.
//although this requires WinXP or newer, it is the only way to uniquely identify
//and independently map multiple keyboards and mice. DirectInput merges all
//keyboards and mice into one device per.
//
//as WM_INPUT lacks specific RAWINPUT structures for gamepads, giving only raw
//data, and because DirectInput supports up to 16 joypads, DirectInput is used
//for joypad mapping.
//
//further, Xbox 360 controllers are explicitly detected and supported through
//XInput. this is because under DirectInput, the LT / RT (trigger) buttons are
//merged into a single Z-axis -- making it impossible to detect both buttons
//being pressed at the same time. with XInput, the state of both trigger
//buttons can be read independently.
//
//so in essence, this is actually more of a hybrid driver.

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <xinput.h>

namespace ruby {

static DWORD WINAPI RawInputThreadProc(void*);
static LRESULT CALLBACK RawInputWindowProc(HWND, UINT, WPARAM, LPARAM);

class RawInput {
public:
  HANDLE mutex;
  HWND hwnd;
  bool initialized;
  bool ready;

  struct Device {
    HANDLE handle;
  };

  struct Keyboard : Device {
    bool state[nall::Keyboard::Size];

    void update(RAWINPUT *input) {
      unsigned code  = input->data.keyboard.MakeCode;
      unsigned flags = input->data.keyboard.Flags;

      #define map(id, flag, name) if(code == id) state[name] = (bool)(flags == flag);
      map(0x0001, 0, nall::Keyboard::Escape)
      map(0x003b, 0, nall::Keyboard::F1)
      map(0x003c, 0, nall::Keyboard::F2)
      map(0x003d, 0, nall::Keyboard::F3)
      map(0x003e, 0, nall::Keyboard::F4)
      map(0x003f, 0, nall::Keyboard::F5)
      map(0x0040, 0, nall::Keyboard::F6)
      map(0x0041, 0, nall::Keyboard::F7)
      map(0x0042, 0, nall::Keyboard::F8)
      map(0x0043, 0, nall::Keyboard::F9)
      map(0x0044, 0, nall::Keyboard::F10)
      map(0x0057, 0, nall::Keyboard::F11)
      map(0x0058, 0, nall::Keyboard::F12)

      map(0x0037, 2, nall::Keyboard::PrintScreen)
      map(0x0046, 0, nall::Keyboard::ScrollLock)
      map(0x001d, 4, nall::Keyboard::Pause)
      map(0x0029, 0, nall::Keyboard::Tilde)

      map(0x0002, 0, nall::Keyboard::Num1)
      map(0x0003, 0, nall::Keyboard::Num2)
      map(0x0004, 0, nall::Keyboard::Num3)
      map(0x0005, 0, nall::Keyboard::Num4)
      map(0x0006, 0, nall::Keyboard::Num5)
      map(0x0007, 0, nall::Keyboard::Num6)
      map(0x0008, 0, nall::Keyboard::Num7)
      map(0x0009, 0, nall::Keyboard::Num8)
      map(0x000a, 0, nall::Keyboard::Num9)
      map(0x000b, 0, nall::Keyboard::Num0)

      map(0x000c, 0, nall::Keyboard::Dash)
      map(0x000d, 0, nall::Keyboard::Equal)
      map(0x000e, 0, nall::Keyboard::Backspace)

      map(0x0052, 2, nall::Keyboard::Insert)
      map(0x0053, 2, nall::Keyboard::Delete)
      map(0x0047, 2, nall::Keyboard::Home)
      map(0x004f, 2, nall::Keyboard::End)
      map(0x0049, 2, nall::Keyboard::PageUp)
      map(0x0051, 2, nall::Keyboard::PageDown)

      map(0x001e, 0, nall::Keyboard::A)
      map(0x0030, 0, nall::Keyboard::B)
      map(0x002e, 0, nall::Keyboard::C)
      map(0x0020, 0, nall::Keyboard::D)
      map(0x0012, 0, nall::Keyboard::E)
      map(0x0021, 0, nall::Keyboard::F)
      map(0x0022, 0, nall::Keyboard::G)
      map(0x0023, 0, nall::Keyboard::H)
      map(0x0017, 0, nall::Keyboard::I)
      map(0x0024, 0, nall::Keyboard::J)
      map(0x0025, 0, nall::Keyboard::K)
      map(0x0026, 0, nall::Keyboard::L)
      map(0x0032, 0, nall::Keyboard::M)
      map(0x0031, 0, nall::Keyboard::N)
      map(0x0018, 0, nall::Keyboard::O)
      map(0x0019, 0, nall::Keyboard::P)
      map(0x0010, 0, nall::Keyboard::Q)
      map(0x0013, 0, nall::Keyboard::R)
      map(0x001f, 0, nall::Keyboard::S)
      map(0x0014, 0, nall::Keyboard::T)
      map(0x0016, 0, nall::Keyboard::U)
      map(0x002f, 0, nall::Keyboard::V)
      map(0x0011, 0, nall::Keyboard::W)
      map(0x002d, 0, nall::Keyboard::X)
      map(0x0015, 0, nall::Keyboard::Y)
      map(0x002c, 0, nall::Keyboard::Z)

      map(0x001a, 0, nall::Keyboard::LeftBracket)
      map(0x001b, 0, nall::Keyboard::RightBracket)
      map(0x002b, 0, nall::Keyboard::Backslash)
      map(0x0027, 0, nall::Keyboard::Semicolon)
      map(0x0028, 0, nall::Keyboard::Apostrophe)
      map(0x0033, 0, nall::Keyboard::Comma)
      map(0x0034, 0, nall::Keyboard::Period)
      map(0x0035, 0, nall::Keyboard::Slash)

      map(0x004f, 0, nall::Keyboard::Keypad1)
      map(0x0050, 0, nall::Keyboard::Keypad2)
      map(0x0051, 0, nall::Keyboard::Keypad3)
      map(0x004b, 0, nall::Keyboard::Keypad4)
      map(0x004c, 0, nall::Keyboard::Keypad5)
      map(0x004d, 0, nall::Keyboard::Keypad6)
      map(0x0047, 0, nall::Keyboard::Keypad7)
      map(0x0048, 0, nall::Keyboard::Keypad8)
      map(0x0049, 0, nall::Keyboard::Keypad9)
      map(0x0052, 0, nall::Keyboard::Keypad0)

      map(0x0053, 0, nall::Keyboard::Point)
      map(0x001c, 2, nall::Keyboard::Enter)
      map(0x004e, 0, nall::Keyboard::Add)
      map(0x004a, 0, nall::Keyboard::Subtract)
      map(0x0037, 0, nall::Keyboard::Multiply)
      map(0x0035, 2, nall::Keyboard::Divide)

      map(0x0045, 0, nall::Keyboard::NumLock)
      map(0x003a, 0, nall::Keyboard::CapsLock)

      //Pause signals 0x1d:4 + 0x45:0, whereas NumLock signals only 0x45:0.
      //this makes it impractical to detect both Pause+NumLock independently.
      //workaround: always detect Pause; detect NumLock only when Pause is released.
      if(state[nall::Keyboard::Pause]) state[nall::Keyboard::NumLock] = false;

      map(0x0048, 2, nall::Keyboard::Up)
      map(0x0050, 2, nall::Keyboard::Down)
      map(0x004b, 2, nall::Keyboard::Left)
      map(0x004d, 2, nall::Keyboard::Right)

      map(0x000f, 0, nall::Keyboard::Tab)
      map(0x001c, 0, nall::Keyboard::Return)
      map(0x0039, 0, nall::Keyboard::Spacebar)
      map(0x005d, 2, nall::Keyboard::Menu)

      //merge left and right modifiers to one ID
      if(code == 0x002a && flags == 0) state[nall::Keyboard::Shift] = 1;  //left shift
      if(code == 0x002a && flags == 1) state[nall::Keyboard::Shift] = 0;
      if(code == 0x0036 && flags == 0) state[nall::Keyboard::Shift] = 1;  //right shift
      if(code == 0x0036 && flags == 1) state[nall::Keyboard::Shift] = 0;

      if(code == 0x001d && flags == 0) state[nall::Keyboard::Control] = 1;  //left control
      if(code == 0x001d && flags == 1) state[nall::Keyboard::Control] = 0;
      if(code == 0x001d && flags == 2) state[nall::Keyboard::Control] = 1;  //right control
      if(code == 0x001d && flags == 3) state[nall::Keyboard::Control] = 0;

      if(code == 0x0038 && flags == 0) state[nall::Keyboard::Alt] = 1;  //left alt
      if(code == 0x0038 && flags == 1) state[nall::Keyboard::Alt] = 0;
      if(code == 0x0038 && flags == 2) state[nall::Keyboard::Alt] = 1;  //right alt
      if(code == 0x0038 && flags == 3) state[nall::Keyboard::Alt] = 0;

      if(code == 0x005b && flags == 2) state[nall::Keyboard::Super] = 1;  //left super
      if(code == 0x005b && flags == 3) state[nall::Keyboard::Super] = 0;
      if(code == 0x005c && flags == 2) state[nall::Keyboard::Super] = 1;  //right super
      if(code == 0x005c && flags == 3) state[nall::Keyboard::Super] = 0;
      #undef map
    }

    Keyboard() {
      for(unsigned i = 0; i < nall::Keyboard::Size; i++) state[i] = false;
    }
  };

  struct Mouse : Device {
    signed xDistance;
    signed yDistance;
    signed zDistance;
    unsigned buttonState;

    void sync() {
      xDistance = 0;
      yDistance = 0;
      zDistance = 0;
    }

    void update(RAWINPUT *input) {
      if((input->data.mouse.usFlags & 1) == MOUSE_MOVE_RELATIVE) {
        xDistance += input->data.mouse.lLastX;
        yDistance += input->data.mouse.lLastY;
      }

      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) buttonState |=  1 << 0;
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP  ) buttonState &=~ 1 << 0;
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) buttonState |=  1 << 2;  //swap middle and right buttons,
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP  ) buttonState &=~ 1 << 2;  //for consistency with Linux:
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) buttonState |=  1 << 1;  //left = 0, middle = 1, right = 2
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP  ) buttonState &=~ 1 << 1;
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) buttonState |=  1 << 3;
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP  ) buttonState &=~ 1 << 3;
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) buttonState |=  1 << 4;
      if(input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP  ) buttonState &=~ 1 << 4;

      if(input->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
        zDistance += (int16_t)input->data.mouse.usButtonData;
      }
    }

    Mouse() {
      xDistance = yDistance = zDistance = 0;
      buttonState = 0;
    }
  };

  //keep track of gamepads for the sole purpose of distinguishing XInput devices
  //from all other devices. this is necessary, as DirectInput does not provide
  //a way to retrieve the necessary RIDI_DEVICENAME string.
  struct Gamepad : Device {
    bool isXInputDevice;
    uint16_t vendorId;
    uint16_t productId;
  };

  vector<Keyboard> lkeyboard;
  vector<Mouse>    lmouse;
  vector<Gamepad>  lgamepad;

  LRESULT window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if(msg == WM_INPUT) {
      unsigned size = 0;
      GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
      RAWINPUT *input = new RAWINPUT[size];
      GetRawInputData((HRAWINPUT)lparam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));
      WaitForSingleObject(mutex, INFINITE);

      if(input->header.dwType == RIM_TYPEKEYBOARD) {
        for(unsigned i = 0; i < lkeyboard.size(); i++) {
          if(input->header.hDevice == lkeyboard[i].handle) {
            lkeyboard[i].update(input);
            break;
          }
        }
      } else if(input->header.dwType == RIM_TYPEMOUSE) {
        for(unsigned i = 0; i < lmouse.size(); i++) {
          if(input->header.hDevice == lmouse[i].handle) {
            lmouse[i].update(input);
            break;
          }
        }
      }

      ReleaseMutex(mutex);
      //allow propogation of WM_INPUT message
      LRESULT result = DefRawInputProc(&input, size, sizeof(RAWINPUTHEADER));
      delete[] input;
      return result;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  //this is used to sort device IDs
  struct DevicePool {
    HANDLE handle;
    char name[4096];
    bool operator<(const DevicePool &pool) const { return strcmp(name, pool.name) < 0; }
  };

  int main() {
    //create an invisible window to act as a sink, capturing all WM_INPUT messages
    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(0);
    wc.lpfnWndProc = RawInputWindowProc;
    wc.lpszClassName = "RawInputClass";
    wc.lpszMenuName = 0;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    RegisterClass(&wc);

    hwnd = CreateWindow("RawInputClass", "RawInputClass", WS_POPUP,
      0, 0, 64, 64, 0, 0, GetModuleHandle(0), 0);

    //enumerate all HID devices
    unsigned devices = 0;
    GetRawInputDeviceList(NULL, &devices, sizeof(RAWINPUTDEVICELIST));
    RAWINPUTDEVICELIST *list = new RAWINPUTDEVICELIST[devices];
    GetRawInputDeviceList(list, &devices, sizeof(RAWINPUTDEVICELIST));

    //sort all devices by name. this has two important properties:
    //1) it consistently orders peripherals, so mapped IDs remain constant
    //2) it sorts the virtual keyboard and mouse to the bottom of the list
    //   (real devices start with \\?\HID#, virtual with \\?\Root#)
    DevicePool pool[devices];
    for(unsigned i = 0; i < devices; i++) {
      pool[i].handle = list[i].hDevice;
      unsigned size = sizeof(pool[i].name) - 1;
      GetRawInputDeviceInfo(list[i].hDevice, RIDI_DEVICENAME, &pool[i].name, &size);
    }
    nall::sort(pool, devices);
    delete[] list;

    for(unsigned i = 0; i < devices; i++) {
      RID_DEVICE_INFO info;
      info.cbSize = sizeof(RID_DEVICE_INFO);

      unsigned size = info.cbSize;
      GetRawInputDeviceInfo(pool[i].handle, RIDI_DEVICEINFO, &info, &size);

      if(info.dwType == RIM_TYPEKEYBOARD) {
        unsigned n = lkeyboard.size();
        lkeyboard[n].handle = pool[i].handle;
      } else if(info.dwType == RIM_TYPEMOUSE) {
        unsigned n = lmouse.size();
        lmouse[n].handle = pool[i].handle;
      } else if(info.dwType == RIM_TYPEHID) {
        //if this is a gamepad or joystick device ...
        if(info.hid.usUsagePage == 1 && (info.hid.usUsage == 4 || info.hid.usUsage == 5)) {
          //... then cache device information for later use
          unsigned n = lgamepad.size();
          lgamepad[n].handle = pool[i].handle;
          lgamepad[n].vendorId = (uint16_t)info.hid.dwVendorId;
          lgamepad[n].productId = (uint16_t)info.hid.dwProductId;

          //per MSDN: XInput devices have "IG_" in their device strings,
          //which is how they should be identified.
          const char *p = strstr(pool[i].name, "IG_");
          lgamepad[n].isXInputDevice = (bool)p;
        }
      }
    }

    RAWINPUTDEVICE device[2];
    //capture all keyboard input
    device[0].usUsagePage = 1;
    device[0].usUsage = 6;
    device[0].dwFlags = RIDEV_INPUTSINK;
    device[0].hwndTarget = hwnd;
    //capture all mouse input
    device[1].usUsagePage = 1;
    device[1].usUsage = 2;
    device[1].dwFlags = RIDEV_INPUTSINK;
    device[1].hwndTarget = hwnd;
    RegisterRawInputDevices(device, 2, sizeof(RAWINPUTDEVICE));

    WaitForSingleObject(mutex, INFINITE);
    ready = true;
    ReleaseMutex(mutex);

    while(true) {
      MSG msg;
      GetMessage(&msg, hwnd, 0, 0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    return 0;
  }

  RawInput() : initialized(false), ready(false) {
  }
};

static RawInput rawinput;

DWORD WINAPI RawInputThreadProc(void*) {
  return rawinput.main();
}

LRESULT CALLBACK RawInputWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return rawinput.window_proc(hwnd, msg, wparam, lparam);
}

class XInput {
public:
  HMODULE libxinput;
  DWORD WINAPI (*pXInputGetState)(DWORD, XINPUT_STATE*);

  struct Gamepad {
    unsigned id;

    int16_t hat;
    int16_t axis[6];
    bool button[10];

    void poll(XINPUT_STATE &state) {
      hat = Joypad::HatCenter;
      if(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP   ) hat |= Joypad::HatUp;
      if(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) hat |= Joypad::HatRight;
      if(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ) hat |= Joypad::HatDown;
      if(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ) hat |= Joypad::HatLeft;

      axis[0] = (int16_t)state.Gamepad.sThumbLX;
      axis[1] = (int16_t)state.Gamepad.sThumbLY;
      axis[2] = (int16_t)state.Gamepad.sThumbRX;
      axis[3] = (int16_t)state.Gamepad.sThumbRY;

      //transform left and right trigger ranges:
      //from: 0 (low, eg released) to 255 (high, eg pressed all the way down)
      //to: +32767 (low) to -32768 (high)
      uint16_t triggerX = state.Gamepad.bLeftTrigger;
      uint16_t triggerY = state.Gamepad.bRightTrigger;

      triggerX = (triggerX << 8) | triggerX;
      triggerY = (triggerY << 8) | triggerY;

      axis[4] = (~triggerX) - 32768;
      axis[5] = (~triggerY) - 32768;

      button[0] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
      button[1] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
      button[2] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
      button[3] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
      button[4] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
      button[5] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
      button[6] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
      button[7] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
      button[8] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
      button[9] = (bool)(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
    }

    Gamepad() {
      hat = Joypad::HatCenter;
      for(unsigned n = 0; n < 6; n++) axis[n] = 0;
      for(unsigned n = 0; n < 10; n++) button[n] = false;
    }
  };

  vector<Gamepad> lgamepad;

  void poll() {
    if(!pXInputGetState) return;

    for(unsigned i = 0; i < lgamepad.size(); i++) {
      XINPUT_STATE state;
      DWORD result = pXInputGetState(lgamepad[i].id, &state);
      if(result == ERROR_SUCCESS) lgamepad[i].poll(state);
    }
  }

  void init() {
    if(!pXInputGetState) return;

    //XInput only supports up to four controllers
    for(unsigned i = 0; i <= 3; i++) {
      XINPUT_STATE state;
      DWORD result = pXInputGetState(i, &state);
      if(result == ERROR_SUCCESS) {
        //valid controller detected, add to gamepad list
        unsigned n = lgamepad.size();
        lgamepad[n].id = i;
      }
    }
  }

  XInput() : pXInputGetState(0) {
    //bind xinput1 dynamically, as it does not ship with Windows Vista or below
    libxinput = LoadLibraryA("xinput1_3.dll");
    if(!libxinput) libxinput = LoadLibraryA("xinput1_2.dll");
    if(!libxinput) libxinput = LoadLibraryA("xinput1_1.dll");
    if(!libxinput) return;
    pXInputGetState = (DWORD WINAPI (*)(DWORD, XINPUT_STATE*))GetProcAddress(libxinput, "XInputGetState");
  }

  ~XInput() {
    if(libxinput) FreeLibrary(libxinput);
  }
};

static BOOL CALLBACK DirectInput_EnumJoypadsCallback(const DIDEVICEINSTANCE*, void*);
static BOOL CALLBACK DirectInput_EnumJoypadAxesCallback(const DIDEVICEOBJECTINSTANCE*, void*);

class DirectInput {
public:
  HWND handle;
  LPDIRECTINPUT8 context;
  struct Gamepad {
    LPDIRECTINPUTDEVICE8 handle;

    int16_t hat[4];
    int16_t axis[6];
    bool button[128];

    void poll(DIJOYSTATE2 &state) {
      //POV hats
      for(unsigned n = 0; n < 4; n++) {
        hat[n] = Joypad::HatCenter;

        //POV value is in clockwise-hundredth degree units
        unsigned pov = state.rgdwPOV[n];

        //some drivers report a centered POV hat as -1U, others as 65535U.
        //>= 36000 will match both, as well as invalid ranges.
        if(pov >= 36000) continue;

        if(pov >= 31500 || pov <=  4500) hat[n] |= Joypad::HatUp;
        if(pov >=  4500 && pov <= 13500) hat[n] |= Joypad::HatRight;
        if(pov >= 13500 && pov <= 22500) hat[n] |= Joypad::HatDown;
        if(pov >= 22500 && pov <= 31500) hat[n] |= Joypad::HatLeft;
      }

      //axes
      axis[0] = state.lX;
      axis[1] = state.lY;
      axis[2] = state.lZ;
      axis[3] = state.lRx;
      axis[4] = state.lRy;
      axis[5] = state.lRz;

      //buttons
      for(unsigned n = 0; n < 128; n++) {
        button[n] = (bool)state.rgbButtons[n];
      }
    }

    Gamepad() {
      handle = 0;
      for(unsigned n = 0; n < 4; n++) hat[n] = Joypad::HatCenter;
      for(unsigned n = 0; n < 6; n++) axis[n] = 0;
      for(unsigned n = 0; n < 128; n++) button[n] = false;
    }
  };
  vector<Gamepad> lgamepad;

  void poll() {
    for(unsigned i = 0; i < lgamepad.size(); i++) {
      if(FAILED(lgamepad[i].handle->Poll())) {
        lgamepad[i].handle->Acquire();
        continue;
      }

      DIJOYSTATE2 state;
      lgamepad[i].handle->GetDeviceState(sizeof(DIJOYSTATE2), &state);
      lgamepad[i].poll(state);
    }
  }

  bool init_joypad(const DIDEVICEINSTANCE *instance) {
    //if this is an XInput device, do not acquire it via DirectInput ...
    //the XInput driver above will handle said device.
    for(unsigned i = 0; i < rawinput.lgamepad.size(); i++) {
      uint32_t guid = MAKELONG(rawinput.lgamepad[i].vendorId, rawinput.lgamepad[i].productId);
      if(guid == instance->guidProduct.Data1) {
        if(rawinput.lgamepad[i].isXInputDevice == true) {
          return DIENUM_CONTINUE;
        }
      }
    }

    if(FAILED(context->CreateDevice(instance->guidInstance, &device, 0))) {
      return DIENUM_CONTINUE;
    }

    device->SetDataFormat(&c_dfDIJoystick2);
    device->SetCooperativeLevel(handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    device->EnumObjects(DirectInput_EnumJoypadAxesCallback, (void*)this, DIDFT_ABSAXIS);
    unsigned n = lgamepad.size();
    lgamepad[n].handle = device;
    return DIENUM_CONTINUE;
  }

  bool init_axis(const DIDEVICEOBJECTINSTANCE *instance) {
    DIPROPRANGE range;
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow = DIPH_BYID;
    range.diph.dwObj = instance->dwType;
    range.lMin = -32768;
    range.lMax = +32767;
    device->SetProperty(DIPROP_RANGE, &range.diph);
    return DIENUM_CONTINUE;
  }

  void init(HWND handle_) {
    handle = handle_;
    DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&context, 0);
    context->EnumDevices(DI8DEVCLASS_GAMECTRL, DirectInput_EnumJoypadsCallback, (void*)this, DIEDFL_ATTACHEDONLY);
  }

  void term() {
    for(unsigned i = 0; i < lgamepad.size(); i++) {
      lgamepad[i].handle->Unacquire();
      lgamepad[i].handle->Release();
    }
    lgamepad.reset();

    if(context) {
      context->Release();
      context = 0;
    }
  }

private:
  LPDIRECTINPUTDEVICE8 device;
};

BOOL CALLBACK DirectInput_EnumJoypadsCallback(const DIDEVICEINSTANCE *instance, void *p) {
  return ((DirectInput*)p)->init_joypad(instance);
}

BOOL CALLBACK DirectInput_EnumJoypadAxesCallback(const DIDEVICEOBJECTINSTANCE *instance, void *p) {
  return ((DirectInput*)p)->init_axis(instance);
}

class pInputRaw {
public:
  XInput xinput;
  DirectInput dinput;

  bool acquire_mouse;
  bool cursor_visible;

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

  bool acquire() {
    acquire_mouse = true;
    if(cursor_visible == true) {
      ShowCursor(cursor_visible = false);
    }
    return acquired();
  }

  bool unacquire() {
    acquire_mouse = false;
    ReleaseCapture();
    ClipCursor(NULL);
    if(cursor_visible == false) {
      ShowCursor(cursor_visible = true);
    }
    return true;
  }

  bool acquired() {
    if(acquire_mouse == true) {
      SetFocus(settings.handle);
      SetCapture(settings.handle);
      RECT rc;
      GetWindowRect(settings.handle, &rc);
      ClipCursor(&rc);
    }
    return GetCapture() == settings.handle;
  }

  bool poll(int16_t *table) {
    memset(table, 0, Scancode::Limit * sizeof(int16_t));

    WaitForSingleObject(rawinput.mutex, INFINITE);

    //=========
    //Keyboards
    //=========
    for(unsigned i = 0; i < min(rawinput.lkeyboard.size(), (unsigned)Keyboard::Count); i++) {
      for(unsigned n = 0; n < nall::Keyboard::Size; n++) {
        table[keyboard(i).key(n)] = rawinput.lkeyboard[i].state[n];
      }
    }

    //====
    //Mice
    //====
    for(unsigned i = 0; i < min(rawinput.lmouse.size(), (unsigned)Mouse::Count); i++) {
      table[mouse(i).axis(0)] = rawinput.lmouse[i].xDistance;
      table[mouse(i).axis(1)] = rawinput.lmouse[i].yDistance;
      table[mouse(i).axis(2)] = rawinput.lmouse[i].zDistance;

      for(unsigned n = 0; n < min(5U, (unsigned)Mouse::Buttons); n++) {
        table[mouse(i).button(n)] = (bool)(rawinput.lmouse[i].buttonState & (1 << n));
      }

      rawinput.lmouse[i].sync();
    }

    ReleaseMutex(rawinput.mutex);

    unsigned joy = 0;

    //==================
    //XInput controllers
    //==================
    xinput.poll();
    for(unsigned i = 0; i < xinput.lgamepad.size(); i++) {
      if(joy >= Joypad::Count) break;

      table[joypad(i).hat(0)] = xinput.lgamepad[i].hat;

      for(unsigned axis = 0; axis < min(6U, (unsigned)Joypad::Axes); axis++) {
        table[joypad(i).axis(axis)] = xinput.lgamepad[i].axis[axis];
      }

      for(unsigned button = 0; button < min(10U, (unsigned)Joypad::Buttons); button++) {
        table[joypad(i).button(button)] = xinput.lgamepad[i].button[button];
      }
    }

    //=======================
    //DirectInput controllers
    //=======================
    dinput.poll();
    for(unsigned i = 0; i < dinput.lgamepad.size(); i++) {
      if(joy >= Joypad::Count) break;

      for(unsigned hat = 0; hat < min(4U, (unsigned)Joypad::Hats); hat++) {
        table[joypad(i).hat(hat)] = dinput.lgamepad[i].hat[hat];
      }

      for(unsigned axis = 0; axis < min(6U, (unsigned)Joypad::Axes); axis++) {
        table[joypad(i).axis(axis)] = dinput.lgamepad[i].axis[axis];
      }

      for(unsigned button = 0; button < min(128U, (unsigned)Joypad::Buttons); button++) {
        table[joypad(i).button(button)] = dinput.lgamepad[i].button[button];
      }
    }

    return true;
  }

  bool init() {
    //only spawn RawInput processing thread one time
    if(rawinput.initialized == false) {
      rawinput.initialized = true;
      rawinput.mutex = CreateMutex(NULL, FALSE, NULL);
      CreateThread(NULL, 0, RawInputThreadProc, 0, 0, NULL);

      //RawInput device calibration needs to finish before initializing DirectInput;
      //as it needs device GUIDs to distinguish XInput devices from ordinary joypads.
      bool ready = false;
      do {
        Sleep(10);
        WaitForSingleObject(rawinput.mutex, INFINITE);
        ready = rawinput.ready;
        ReleaseMutex(rawinput.mutex);
      } while(ready == false);
    }

    xinput.init();
    dinput.init(settings.handle);

    acquire_mouse = false;
    cursor_visible = true;
    return true;
  }

  void term() {
    unacquire();
    dinput.term();
  }

  pInputRaw() {
  }
};

DeclareInput(Raw)

};
