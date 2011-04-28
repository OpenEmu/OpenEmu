struct InputSpecifier { enum { None, Up, Down, Left, Right, Lo, Hi, Trigger }; };
struct InputModifier { enum { None = 0, Shift = 1, Control = 2, Alt = 4, Super = 8 }; };
struct InputCategory { enum { Port1 = 0, Port2 = 1, UserInterface = 2, Hidden = 3 }; };

struct InputGroup;

struct MappedInput {
  InputGroup *parent;
  string name;
  string label;
  unsigned specifier;
  unsigned modifier;
  unsigned scancode;
  bool modifierOverride;
  int16_t state;
  int16_t previousState;
  int16_t cachedState;

  void bind();
  virtual void poll() = 0;
  virtual void cache();

  MappedInput(const char*, const char*);
};

struct DigitalInput : MappedInput {
  void poll();

  bool isPressed() const;
  bool wasPressed() const;

  DigitalInput(const char*, const char*);
};

struct AnalogInput : MappedInput {
  void poll();

  AnalogInput(const char*, const char*);
};

struct HotkeyInput : DigitalInput {
  void poll();
  virtual void pressed() {}
  virtual void released() {}

  HotkeyInput(const char*, const char*);
};

struct InputGroup : public array<MappedInput*> {
  unsigned category;
  string label;

  void attach(MappedInput*);
  void bind();
  void poll();
  void cache();
  void flushCache();
  virtual int16_t status(unsigned, unsigned) const { return 0; }

  InputGroup(unsigned, const char*);
};

struct InputMapper : public array<InputGroup*> {
  InputGroup *port1;
  InputGroup *port2;

  bool calibrated;
  bool isTrigger[Joypad::Count][Joypad::Axes];

  bool activeState;
  int16_t stateTable[2][Scancode::Limit];
  unsigned modifier;

  void calibrate();
  void bind();
  void poll();
  void cache();
  int16_t status(bool, unsigned, unsigned, unsigned);

  string modifierString() const;
  int16_t state(uint16_t) const;
  int16_t previousState(uint16_t) const;
  unsigned distance(uint16_t) const;

  InputMapper();
};

InputMapper& mapper();

#include "controller.hpp"
#include "userinterface.hpp"
