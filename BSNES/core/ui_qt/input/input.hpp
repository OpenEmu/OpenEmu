struct InputCode {
  enum type_t {
    KeyboardButton,
    MouseAxis,
    MouseButton,
    JoypadHat,
    JoypadAxis,
    JoypadButton,
    Unknown,
  };
  enum axistype_t {
    Stick,        //bi-directional, centered analog stick (min = -32768, inactive = 0, max = +32767)
    Trigger,      //uni-directional, pressure-sensitive analog trigger button (min = -32768, inactive = max = +32767)
    InvalidAxis,  //not a joypad axis code, or said joypad is not calibrated
  };
  static type_t type(uint16_t code);
  static axistype_t axisType(uint16_t code);
  static int mouseNumber(uint16_t code);
  static int joypadNumber(uint16_t code);
};

class InputManager {
public:
  void bind();
  void poll();
  void clear();
  void flush();

  int16_t state(uint16_t code) const;
  int16_t lastState(uint16_t code) const;
  int16_t getStatus(unsigned deviceid, unsigned id) const;

  void refresh();
  void calibrate(unsigned joypad);
  bool calibrated(unsigned joypad) const;
  InputCode::axistype_t axisType(unsigned joypad, unsigned axis) const;
  function<void (uint16_t)> onInput;

  InputManager();

private:
  bool activeState;
  int16_t stateTable[2][nall::input_limit];

  //joypad axis calibration data
  struct Pad {
    bool calibrated;
    InputCode::axistype_t axis[joypad<>::axes];
  } pad[joypad<>::count];
} inputManager;

struct InputGroup;

struct InputObject {
  InputGroup *parent;
  enum type_t { Button, Axis };
  enum modifier_t { None, Up, Down, Left, Right, Lo, Hi, Trigger };

  type_t type;                 //type of input this object is mapped to
  const char *name;            //plain-text name ("Up", "Down", ... "Start")
  string &id;                  //config-item reference ("joypad1.start") name ("joypad00.button00")
  uint16_t code;               //nall::input code ID
  InputCode::type_t codetype;  //hardware button / axis type
  modifier_t modifier;         //hardware specialization (joypad-axis "::lo", "::hi", etc)
  int16_t state;               //code state as of last inputManager.poll()

  void bind();
  void bind(uint16_t code);
  void poll(int16_t state);
  InputObject(type_t t, const char *n, string &s);
};

struct InputGroup : public array<InputObject*> {
  const char *name;

  void attach(InputObject &object);
  void bind();
  void clear();
  void poll(const int16_t *table);
  virtual int16_t state(unsigned index) const;
  InputGroup(const char *n);

private:
  array<InputObject*> &list;
};

#include "device.hpp"
#include "userinterface.hpp"
