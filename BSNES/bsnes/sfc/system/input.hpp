struct Input {
  enum class Device : unsigned {
    Joypad,
    Multitap,
    Mouse,
    SuperScope,
    Justifier,
    Justifiers,
    USART,
    None,
  };

  enum class JoypadID : unsigned {
    B  =  0, Y    =  1, Select =  2, Start =  3,
    Up =  4, Down =  5, Left   =  6, Right =  7,
    A  =  8, X    =  9, L      = 10, R     = 11,
  };

  enum class MouseID : unsigned {
    X = 0, Y = 1, Left = 2, Right = 3,
  };

  enum class SuperScopeID : unsigned {
    X = 0, Y = 1, Trigger = 2, Cursor = 3, Turbo = 4, Pause = 5,
  };

  enum class JustifierID : unsigned {
    X = 0, Y = 1, Trigger = 2, Start = 3,
  };

  Controller *port1;
  Controller *port2;

  void connect(bool port, Input::Device id);
  Input();
  ~Input();
};

extern Input input;
