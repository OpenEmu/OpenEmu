struct ControllerPort1 { enum { None, Gamepad, Asciipad, Multitap, Mouse }; };
struct ControllerPort2 { enum { None, Gamepad, Asciipad, Multitap, Mouse, SuperScope, Justifier, Justifiers }; };

namespace Controllers {

struct TurboInput : DigitalInput {
  unsigned holdHi;
  unsigned holdLo;
  unsigned counter;
  void cache();
  TurboInput(const char*, const char*);
};

struct Gamepad : InputGroup {
  DigitalInput up, down, left, right, b, a, y, x, l, r, select, start;
  TurboInput turboB, turboA, turboY, turboX, turboL, turboR;
  int16_t status(unsigned, unsigned) const;
  Gamepad(unsigned, const char*, const char*);
};

struct Multitap : InputGroup {
  Gamepad &port1, &port2, &port3, &port4;
  int16_t status(unsigned, unsigned) const;
  Multitap(Gamepad&, Gamepad&, Gamepad&, Gamepad&);
};

struct AsciiSwitch : DigitalInput {
  enum Mode { Off, Turbo, Auto } mode;
  void poll();
  AsciiSwitch(const char*, const char*);
};

struct AsciiSlowMotion : DigitalInput {
  bool enabled;
  unsigned holdHi;
  unsigned holdLo;
  unsigned counter;
  void poll();
  void cache();
  AsciiSlowMotion(const char*, const char*);
};

struct AsciiInput : DigitalInput {
  AsciiSwitch *asciiSwitch;
  unsigned holdHi;
  unsigned holdLo;
  unsigned counter;
  void cache();
  AsciiInput(const char*, const char*);
};

struct Asciipad : InputGroup {
  DigitalInput up, down, left, right;
  AsciiInput b, a, y, x, l, r;
  DigitalInput select, start;
  AsciiSwitch switchB, switchA, switchY, switchX, switchL, switchR;
  AsciiSlowMotion slowMotion;
  int16_t status(unsigned, unsigned) const;
  Asciipad(unsigned, const char*, const char*);
};

struct Mouse : InputGroup {
  AnalogInput x, y;
  DigitalInput left, right;
  int16_t status(unsigned, unsigned) const;
  Mouse(unsigned, const char*, const char*);
};

struct SuperScope : InputGroup {
  AnalogInput x, y;
  DigitalInput trigger, cursor, turbo, pause;
  int16_t status(unsigned, unsigned) const;
  SuperScope(unsigned, const char*, const char*);
};

struct Justifier : InputGroup {
  AnalogInput x, y;
  DigitalInput trigger, start;
  int16_t status(unsigned, unsigned) const;
  Justifier(unsigned, const char*, const char*);
};

struct Justifiers : InputGroup {
  Justifier &port1;
  Justifier &port2;
  int16_t status(unsigned, unsigned) const;
  Justifiers(Justifier&, Justifier&);
};

extern Gamepad gamepad1;
extern Asciipad asciipad1;
extern Gamepad multitap1a;
extern Gamepad multitap1b;
extern Gamepad multitap1c;
extern Gamepad multitap1d;
extern Multitap multitap1;
extern Mouse mouse1;

extern Gamepad gamepad2;
extern Asciipad asciipad2;
extern Gamepad multitap2a;
extern Gamepad multitap2b;
extern Gamepad multitap2c;
extern Gamepad multitap2d;
extern Multitap multitap2;
extern Mouse mouse2;
extern SuperScope superscope;
extern Justifier justifier1;
extern Justifier justifier2;
extern Justifiers justifiers;

}
