struct Input {
  struct Device {
    enum e {
      None,
      Joypad,
      Multitap,
      Mouse,
      SuperScope,
      Justifier,
      Justifiers,
      Serial,
    } i;
  };

  struct JoypadID {
    enum {
      B  =  0, Y    =  1, Select =  2, Start =  3,
      Up =  4, Down =  5, Left   =  6, Right =  7,
      A  =  8, X    =  9, L      = 10, R     = 11,
    } i;
  };

  struct MouseID {
    enum e {
      X = 0, Y = 1, Left = 2, Right = 3,
    } i;
  };

  struct SuperScopeID {
    enum e {
      X = 0, Y = 1, Trigger = 2, Cursor = 3, Turbo = 4, Pause = 5,
    } i;
  };

  struct JustifierID {
    enum e {
      X = 0, Y = 1, Trigger = 2, Start = 3,
    } i;
  };

  Controller *port1;
  Controller *port2;

  void connect(bool port, Input::Device::e id);
  Input();
  ~Input();
};

extern Input input;
