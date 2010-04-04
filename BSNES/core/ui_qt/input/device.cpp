//===========
//InputDevice
//===========

InputDevice::InputDevice(SNES::Input::DeviceID i, bool p, const char *n) : InputGroup(n), id(i), port(p) {
}

//======
//Joypad
//======

int16_t Joypad::state(unsigned index) const {
  if(config.input.allowInvalidInput == false) {
    //SNES D-pads have central pivot point, making up+down or left+right combinations impossible.
    //some software programs rely on this, and will crash if these combinations are allowed.
    if(index == SNES::Input::JoypadDown  && up.state  ) return 0;
    if(index == SNES::Input::JoypadRight && left.state) return 0;
  }

  switch(index) {
    case SNES::Input::JoypadUp:     return up.state;
    case SNES::Input::JoypadDown:   return down.state;
    case SNES::Input::JoypadLeft:   return left.state;
    case SNES::Input::JoypadRight:  return right.state;
    case SNES::Input::JoypadA:      return a.state;
    case SNES::Input::JoypadB:      return b.state;
    case SNES::Input::JoypadX:      return x.state;
    case SNES::Input::JoypadY:      return y.state;
    case SNES::Input::JoypadL:      return l.state;
    case SNES::Input::JoypadR:      return r.state;
    case SNES::Input::JoypadSelect: return select.state;
    case SNES::Input::JoypadStart:  return start.state;
  }

  return 0;
}

Joypad::Joypad(SNES::Input::DeviceID id, bool port, const char *name,
string &up_t, string &down_t, string &left_t, string &right_t, string &a_t, string &b_t,
string &x_t, string &y_t, string &l_t, string &r_t, string &select_t, string &start_t
) :
InputDevice(id, port, name),
up    (InputObject::Button, "Up",     up_t),
down  (InputObject::Button, "Down",   down_t),
left  (InputObject::Button, "Left",   left_t),
right (InputObject::Button, "Right",  right_t),
a     (InputObject::Button, "A",      a_t),
b     (InputObject::Button, "B",      b_t),
x     (InputObject::Button, "X",      x_t),
y     (InputObject::Button, "Y",      y_t),
l     (InputObject::Button, "L",      l_t),
r     (InputObject::Button, "R",      r_t),
select(InputObject::Button, "Select", select_t),
start (InputObject::Button, "Start",  start_t) {
  attach(up); attach(down); attach(left); attach(right); attach(a); attach(b);
  attach(x); attach(y); attach(l); attach(r); attach(select); attach(start);
}

//=====
//Mouse
//=====

int16_t Mouse::state(unsigned index) const {
  switch(index) {
    case SNES::Input::MouseX:     return x.state;
    case SNES::Input::MouseY:     return y.state;
    case SNES::Input::MouseLeft:  return left.state;
    case SNES::Input::MouseRight: return right.state;
  }

  return 0;
}

Mouse::Mouse(SNES::Input::DeviceID id, bool port, const char *name,
string &x_t, string &y_t, string &left_t, string &right_t
) :
InputDevice(id, port, name),
x    (InputObject::Axis,   "X-axis",       x_t),
y    (InputObject::Axis,   "Y-axis",       y_t),
left (InputObject::Button, "Left button",  left_t),
right(InputObject::Button, "Right button", right_t) {
  attach(x); attach(y); attach(left); attach(right);
}

//==========
//SuperScope
//==========

int16_t SuperScope::state(unsigned index) const {
  switch(index) {
    case SNES::Input::SuperScopeX:       return x.state;
    case SNES::Input::SuperScopeY:       return y.state;
    case SNES::Input::SuperScopeTrigger: return trigger.state;
    case SNES::Input::SuperScopeCursor:  return cursor.state;
    case SNES::Input::SuperScopeTurbo:   return turbo.state;
    case SNES::Input::SuperScopePause:   return pause.state;
  }

  return 0;
}

SuperScope::SuperScope(SNES::Input::DeviceID id, bool port, const char *name,
string &x_t, string &y_t, string &trigger_t, string &cursor_t, string &turbo_t, string &pause_t
) :
InputDevice(id, port, name),
x      (InputObject::Axis,   "X-axis",  x_t),
y      (InputObject::Axis,   "Y-axis",  y_t),
trigger(InputObject::Button, "Trigger", trigger_t),
cursor (InputObject::Button, "Cursor",  cursor_t),
turbo  (InputObject::Button, "Turbo",   turbo_t),
pause  (InputObject::Button, "Pause",   pause_t) {
  attach(x); attach(y); attach(trigger); attach(cursor); attach(turbo); attach(pause);
}

//=========
//Justifier
//=========

int16_t Justifier::state(unsigned index) const {
  switch(index) {
    case SNES::Input::JustifierX:       return x.state;
    case SNES::Input::JustifierY:       return y.state;
    case SNES::Input::JustifierTrigger: return trigger.state;
    case SNES::Input::JustifierStart:   return start.state;
  }

  return 0;
}

Justifier::Justifier(SNES::Input::DeviceID id, bool port, const char *name,
string &x_t, string &y_t, string &trigger_t, string &start_t
) :
InputDevice(id, port, name),
x      (InputObject::Axis,   "X-axis",  x_t),
y      (InputObject::Axis,   "Y-axis",  y_t),
trigger(InputObject::Button, "Trigger", trigger_t),
start  (InputObject::Button, "Start",   start_t) {
  attach(x); attach(y); attach(trigger); attach(start);
}

//===============
//InputDevicePool
//===============

void InputDevicePool::attach(InputDevice &device) {
  list.add(&device);
}

void InputDevicePool::bind() {
  for(unsigned i = 0; i < list.size(); i++) list[i]->bind();
}

void InputDevicePool::clear() {
  for(unsigned i = 0; i < list.size(); i++) list[i]->clear();
}

void InputDevicePool::poll(const int16_t *table) {
  for(unsigned i = 0; i < list.size(); i++) list[i]->poll(table);
}

InputDevice* InputDevicePool::find(SNES::Input::DeviceID id) {
  for(unsigned i = 0; i < list.size(); i++) {
    if(list[i]->id == id) return list[i];
  }

  return 0;
}

InputDevicePool::InputDevicePool() : list(*this) {
}

//

Joypad joypad1(SNES::Input::DeviceIDJoypad1, InputDevice::Port1, "Joypad",
config.input.joypad1.up, config.input.joypad1.down, config.input.joypad1.left,   config.input.joypad1.right,
config.input.joypad1.a,  config.input.joypad1.b,    config.input.joypad1.x,      config.input.joypad1.y,
config.input.joypad1.l,  config.input.joypad1.r,    config.input.joypad1.select, config.input.joypad1.start);

Joypad joypad2(SNES::Input::DeviceIDJoypad2, InputDevice::Port2, "Joypad",
config.input.joypad2.up, config.input.joypad2.down, config.input.joypad2.left,   config.input.joypad2.right,
config.input.joypad2.a,  config.input.joypad2.b,    config.input.joypad2.x,      config.input.joypad2.y,
config.input.joypad2.l,  config.input.joypad2.r,    config.input.joypad2.select, config.input.joypad2.start);

Joypad multitap1a(SNES::Input::DeviceIDMultitap1A, InputDevice::Port1, "Multitap - Port 1",
config.input.multitap1a.up, config.input.multitap1a.down, config.input.multitap1a.left,   config.input.multitap1a.right,
config.input.multitap1a.a,  config.input.multitap1a.b,    config.input.multitap1a.x,      config.input.multitap1a.y,
config.input.multitap1a.l,  config.input.multitap1a.r,    config.input.multitap1a.select, config.input.multitap1a.start);

Joypad multitap1b(SNES::Input::DeviceIDMultitap1B, InputDevice::Port1, "Multitap - Port 2",
config.input.multitap1b.up, config.input.multitap1b.down, config.input.multitap1b.left,   config.input.multitap1b.right,
config.input.multitap1b.a,  config.input.multitap1b.b,    config.input.multitap1b.x,      config.input.multitap1b.y,
config.input.multitap1b.l,  config.input.multitap1b.r,    config.input.multitap1b.select, config.input.multitap1b.start);

Joypad multitap1c(SNES::Input::DeviceIDMultitap1C, InputDevice::Port1, "Multitap - Port 3",
config.input.multitap1c.up, config.input.multitap1c.down, config.input.multitap1c.left,   config.input.multitap1c.right,
config.input.multitap1c.a,  config.input.multitap1c.b,    config.input.multitap1c.x,      config.input.multitap1c.y,
config.input.multitap1c.l,  config.input.multitap1c.r,    config.input.multitap1c.select, config.input.multitap1c.start);

Joypad multitap1d(SNES::Input::DeviceIDMultitap1D, InputDevice::Port1, "Multitap - Port 4",
config.input.multitap1d.up, config.input.multitap1d.down, config.input.multitap1d.left,   config.input.multitap1d.right,
config.input.multitap1d.a,  config.input.multitap1d.b,    config.input.multitap1d.x,      config.input.multitap1d.y,
config.input.multitap1d.l,  config.input.multitap1d.r,    config.input.multitap1d.select, config.input.multitap1d.start);

Joypad multitap2a(SNES::Input::DeviceIDMultitap2A, InputDevice::Port2, "Multitap - Port 1",
config.input.multitap2a.up, config.input.multitap2a.down, config.input.multitap2a.left,   config.input.multitap2a.right,
config.input.multitap2a.a,  config.input.multitap2a.b,    config.input.multitap2a.x,      config.input.multitap2a.y,
config.input.multitap2a.l,  config.input.multitap2a.r,    config.input.multitap2a.select, config.input.multitap2a.start);

Joypad multitap2b(SNES::Input::DeviceIDMultitap2B, InputDevice::Port2, "Multitap - Port 2",
config.input.multitap2b.up, config.input.multitap2b.down, config.input.multitap2b.left,   config.input.multitap2b.right,
config.input.multitap2b.a,  config.input.multitap2b.b,    config.input.multitap2b.x,      config.input.multitap2b.y,
config.input.multitap2b.l,  config.input.multitap2b.r,    config.input.multitap2b.select, config.input.multitap2b.start);

Joypad multitap2c(SNES::Input::DeviceIDMultitap2C, InputDevice::Port2, "Multitap - Port 3",
config.input.multitap2c.up, config.input.multitap2c.down, config.input.multitap2c.left,   config.input.multitap2c.right,
config.input.multitap2c.a,  config.input.multitap2c.b,    config.input.multitap2c.x,      config.input.multitap2c.y,
config.input.multitap2c.l,  config.input.multitap2c.r,    config.input.multitap2c.select, config.input.multitap2c.start);

Joypad multitap2d(SNES::Input::DeviceIDMultitap2D, InputDevice::Port2, "Multitap - Port 4",
config.input.multitap2d.up, config.input.multitap2d.down, config.input.multitap2d.left,   config.input.multitap2d.right,
config.input.multitap2d.a,  config.input.multitap2d.b,    config.input.multitap2d.x,      config.input.multitap2d.y,
config.input.multitap2d.l,  config.input.multitap2d.r,    config.input.multitap2d.select, config.input.multitap2d.start);

Mouse mouse1(SNES::Input::DeviceIDMouse1, InputDevice::Port1, "Mouse",
config.input.mouse1.x, config.input.mouse1.y, config.input.mouse1.left, config.input.mouse1.right);

Mouse mouse2(SNES::Input::DeviceIDMouse2, InputDevice::Port2, "Mouse",
config.input.mouse2.x, config.input.mouse2.y, config.input.mouse2.left, config.input.mouse2.right);

SuperScope superscope(SNES::Input::DeviceIDSuperScope, InputDevice::Port2, "Super Scope",
config.input.superscope.x,       config.input.superscope.y,
config.input.superscope.trigger, config.input.superscope.cursor,
config.input.superscope.turbo,   config.input.superscope.pause);

Justifier justifier1(SNES::Input::DeviceIDJustifier1, InputDevice::Port2, "Justifier 1",
config.input.justifier1.x,       config.input.justifier1.y,
config.input.justifier1.trigger, config.input.justifier1.start);

Justifier justifier2(SNES::Input::DeviceIDJustifier2, InputDevice::Port2, "Justifier 2",
config.input.justifier2.x,       config.input.justifier2.y,
config.input.justifier2.trigger, config.input.justifier2.start);

InputSnesDevicePool inputPool;

InputSnesDevicePool::InputSnesDevicePool() {
  attach(joypad1);
  attach(joypad2);
  attach(multitap1a);
  attach(multitap1b);
  attach(multitap1c);
  attach(multitap1d);
  attach(multitap2a);
  attach(multitap2b);
  attach(multitap2c);
  attach(multitap2d);
  attach(mouse1);
  attach(mouse2);
  attach(superscope);
  attach(justifier1);
  attach(justifier2);
}
