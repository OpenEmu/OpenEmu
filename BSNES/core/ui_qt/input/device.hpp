struct InputDevice : InputGroup {
  SNES::Input::DeviceID id;
  enum Port { Port1, Port2 };
  const bool port;

  InputDevice(SNES::Input::DeviceID i, bool p, const char *n);
};

struct Joypad : InputDevice {
  InputObject up, down, left, right, a, b, x, y, l, r, select, start;

  int16_t state(unsigned index) const;
  Joypad(SNES::Input::DeviceID id, bool port, const char *name,
  string&, string&, string&, string&, string&, string&,
  string&, string&, string&, string&, string&, string&);
};

struct Mouse : InputDevice {
  InputObject x, y, left, right;

  int16_t state(unsigned index) const;
  Mouse(SNES::Input::DeviceID id, bool port, const char *name,
  string&, string&, string&, string&);
};

struct SuperScope : InputDevice {
  InputObject x, y, trigger, cursor, turbo, pause;

  int16_t state(unsigned index) const;
  SuperScope(SNES::Input::DeviceID id, bool port, const char *name,
  string&, string&, string&, string&, string&, string&);
};

struct Justifier : InputDevice {
  InputObject x, y, trigger, start;

  int16_t state(unsigned index) const;
  Justifier(SNES::Input::DeviceID id, bool port, const char *name,
  string&, string&, string&, string&);
};

struct InputDevicePool : public array<InputDevice*> {
  void attach(InputDevice &device);
  void bind();
  void clear();
  void poll(const int16_t *table);
  InputDevice* find(SNES::Input::DeviceID id);
  InputDevicePool();

private:
  array<InputDevice*> &list;
};

struct InputSnesDevicePool : public InputDevicePool {
  InputSnesDevicePool();
};

extern Joypad joypad1;
extern Joypad joypad2;
extern Joypad multitap1a;
extern Joypad multitap1b;
extern Joypad multitap1c;
extern Joypad multitap1d;
extern Joypad multitap2a;
extern Joypad multitap2b;
extern Joypad multitap2c;
extern Joypad multitap2d;
extern Mouse mouse1;
extern Mouse mouse2;
extern SuperScope superscope;
extern Justifier justifier1;
extern Justifier justifier2;
extern InputSnesDevicePool inputPool;
