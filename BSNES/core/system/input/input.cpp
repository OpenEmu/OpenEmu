#ifdef SYSTEM_CPP

Input input;

uint8 Input::port_read(bool portnumber) {
  port_t &p = port[portnumber];

  switch(p.device) {
    case DeviceJoypad: {
      if(p.counter0 >= 16) return 1;
      unsigned deviceid = (portnumber == 0 ? DeviceIDJoypad1 : DeviceIDJoypad2);
      return system.interface->input_poll(deviceid, p.counter0++);
    } //case DeviceJoypad

    case DeviceMultitap: {
      if(cpu.joylatch()) return 2; //when latch is high -- data2 = 1, data1 = 0

      unsigned deviceidx, deviceid0, deviceid1;
      if(portnumber == 0) {
        if(cpu.pio() & 0x40) {
          deviceidx = p.counter0;
          if(deviceidx >= 16) return 3;
          p.counter0++;

          deviceid0 = DeviceIDMultitap1A;
          deviceid1 = DeviceIDMultitap1B;
        } else {
          deviceidx = p.counter1;
          if(deviceidx >= 16) return 3;
          p.counter1++;

          deviceid0 = DeviceIDMultitap1C;
          deviceid1 = DeviceIDMultitap1D;
        }
      } else {
        if(cpu.pio() & 0x80) {
          deviceidx = p.counter0;
          if(deviceidx >= 16) return 3;
          p.counter0++;

          deviceid0 = DeviceIDMultitap2A;
          deviceid1 = DeviceIDMultitap2B;
        } else {
          deviceidx = p.counter1;
          if(deviceidx >= 16) return 3;
          p.counter1++;

          deviceid0 = DeviceIDMultitap2C;
          deviceid1 = DeviceIDMultitap2D;
        }
      }

      return (system.interface->input_poll(deviceid0, deviceidx) << 0)
           | (system.interface->input_poll(deviceid1, deviceidx) << 1);
    } //case DeviceMultitap

    case DeviceMouse: {
      if(p.counter0 >= 32) return 1;
      unsigned deviceid = (portnumber == 0 ? DeviceIDMouse1 : DeviceIDMouse2);

      int position_x = system.interface->input_poll(deviceid, MouseX);  //-n = left, 0 = center, +n = right
      int position_y = system.interface->input_poll(deviceid, MouseY);  //-n = up,   0 = center, +n = right

      bool direction_x = position_x < 0;  //0 = right, 1 = left
      bool direction_y = position_y < 0;  //0 = down,  1 = up

      if(position_x < 0) position_x = -position_x;  //abs(position_x)
      if(position_y < 0) position_y = -position_y;  //abs(position_x)

      position_x = min(127, position_x);  //range = 0 - 127
      position_y = min(127, position_y);  //range = 0 - 127

      switch(p.counter0++) { default:
        case  0: return 0;
        case  1: return 0;
        case  2: return 0;
        case  3: return 0;
        case  4: return 0;
        case  5: return 0;
        case  6: return 0;
        case  7: return 0;

        case  8: return system.interface->input_poll(deviceid, MouseRight);
        case  9: return system.interface->input_poll(deviceid, MouseLeft);
        case 10: return 0;  //speed (0 = slow, 1 = normal, 2 = fast, 3 = unused)
        case 11: return 0;  // ||

        case 12: return 0;  //signature
        case 13: return 0;  // ||
        case 14: return 0;  // ||
        case 15: return 1;  // ||

        case 16: return (direction_y) & 1;
        case 17: return (position_y >> 6) & 1;
        case 18: return (position_y >> 5) & 1;
        case 19: return (position_y >> 4) & 1;
        case 20: return (position_y >> 3) & 1;
        case 21: return (position_y >> 2) & 1;
        case 22: return (position_y >> 1) & 1;
        case 23: return (position_y >> 0) & 1;

        case 24: return (direction_x) & 1;
        case 25: return (position_x >> 6) & 1;
        case 26: return (position_x >> 5) & 1;
        case 27: return (position_x >> 4) & 1;
        case 28: return (position_x >> 3) & 1;
        case 29: return (position_x >> 2) & 1;
        case 30: return (position_x >> 1) & 1;
        case 31: return (position_x >> 0) & 1;
      }
    } //case DeviceMouse

    case DeviceSuperScope: {
      if(portnumber == 0) break;  //Super Scope in port 1 not supported ...
      if(p.counter0 >= 8) return 1;

      if(p.counter0 == 0) {
        //turbo is a switch; toggle is edge sensitive
        bool turbo = system.interface->input_poll(DeviceIDSuperScope, SuperScopeTurbo);
        if(turbo && !p.superscope.turbolock) {
          p.superscope.turbo = !p.superscope.turbo;  //toggle state
          p.superscope.turbolock = true;
        } else if(!turbo) {
          p.superscope.turbolock = false;
        }

        //trigger is a button
        //if turbo is active, trigger is level sensitive; otherwise it is edge sensitive
        p.superscope.trigger = false;
        bool trigger = system.interface->input_poll(DeviceIDSuperScope, SuperScopeTrigger);
        if(trigger && (p.superscope.turbo || !p.superscope.triggerlock)) {
          p.superscope.trigger = true;
          p.superscope.triggerlock = true;
        } else if(!trigger) {
          p.superscope.triggerlock = false;
        }

        //cursor is a button; it is always level sensitive
        p.superscope.cursor = system.interface->input_poll(DeviceIDSuperScope, SuperScopeCursor);

        //pause is a button; it is always edge sensitive
        p.superscope.pause = false;
        bool pause = system.interface->input_poll(DeviceIDSuperScope, SuperScopePause);
        if(pause && !p.superscope.pauselock) {
          p.superscope.pause = true;
          p.superscope.pauselock = true;
        } else if(!pause) {
          p.superscope.pauselock = false;
        }

        p.superscope.offscreen =
           p.superscope.x < 0 || p.superscope.x >= 256
        || p.superscope.y < 0 || p.superscope.y >= (ppu.overscan() ? 240 : 225);
      }

      switch(p.counter0++) {
        case 0: return p.superscope.trigger;
        case 1: return p.superscope.cursor;
        case 2: return p.superscope.turbo;
        case 3: return p.superscope.pause;
        case 4: return 0;
        case 5: return 0;
        case 6: return p.superscope.offscreen;
        case 7: return 0;  //noise (1 = yes)
      }
    } //case DeviceSuperScope

    case DeviceJustifier:
    case DeviceJustifiers: {
      if(portnumber == 0) break;  //Justifier in port 1 not supported ...
      if(p.counter0 >= 32) return 1;

      if(p.counter0 == 0) {
        p.justifier.trigger1 = system.interface->input_poll(DeviceIDJustifier1, JustifierTrigger);
        p.justifier.start1   = system.interface->input_poll(DeviceIDJustifier1, JustifierStart);

        if(p.device == DeviceJustifiers) {
          p.justifier.trigger2 = system.interface->input_poll(DeviceIDJustifier2, JustifierTrigger);
          p.justifier.start2   = system.interface->input_poll(DeviceIDJustifier2, JustifierStart);
        } else {
          p.justifier.x2 = -1;
          p.justifier.y2 = -1;

          p.justifier.trigger2 = false;
          p.justifier.start2   = false;
        }
      }

      switch(p.counter0++) {
        case  0: return 0;
        case  1: return 0;
        case  2: return 0;
        case  3: return 0;
        case  4: return 0;
        case  5: return 0;
        case  6: return 0;
        case  7: return 0;
        case  8: return 0;
        case  9: return 0;
        case 10: return 0;
        case 11: return 0;

        case 12: return 1;  //signature
        case 13: return 1;  // ||
        case 14: return 1;  // ||
        case 15: return 0;  // ||

        case 16: return 0;
        case 17: return 1;
        case 18: return 0;
        case 19: return 1;
        case 20: return 0;
        case 21: return 1;
        case 22: return 0;
        case 23: return 1;

        case 24: return p.justifier.trigger1;
        case 25: return p.justifier.trigger2;
        case 26: return p.justifier.start1;
        case 27: return p.justifier.start2;
        case 28: return p.justifier.active;

        case 29: return 0;
        case 30: return 0;
        case 31: return 0;
      }
    } //case DeviceJustifier(s)
  } //switch(p.device)

  //no device connected
  return 0;
}

//scan all input; update cursor positions if needed
void Input::update() {
  system.interface->input_poll();
  port_t &p = port[1];

  switch(p.device) {
    case DeviceSuperScope: {
      int x = system.interface->input_poll(DeviceIDSuperScope, SuperScopeX);
      int y = system.interface->input_poll(DeviceIDSuperScope, SuperScopeY);
      x += p.superscope.x;
      y += p.superscope.y;
      p.superscope.x = max(-16, min(256 + 16, x));
      p.superscope.y = max(-16, min(240 + 16, y));

      latchx = p.superscope.x;
      latchy = p.superscope.y;
    } break;

    case DeviceJustifier:
    case DeviceJustifiers: {
      int x1 = system.interface->input_poll(DeviceIDJustifier1, JustifierX);
      int y1 = system.interface->input_poll(DeviceIDJustifier1, JustifierY);
      x1 += p.justifier.x1;
      y1 += p.justifier.y1;
      p.justifier.x1 = max(-16, min(256 + 16, x1));
      p.justifier.y1 = max(-16, min(240 + 16, y1));

      int x2 = system.interface->input_poll(DeviceIDJustifier2, JustifierX);
      int y2 = system.interface->input_poll(DeviceIDJustifier2, JustifierY);
      x2 += p.justifier.x2;
      y2 += p.justifier.y2;
      p.justifier.x2 = max(-16, min(256 + 16, x2));
      p.justifier.y2 = max(-16, min(240 + 16, y2));

      if(p.justifier.active == 0) {
        latchx = p.justifier.x1;
        latchy = p.justifier.y1;
      } else {
        latchx = (p.device == DeviceJustifiers ? p.justifier.x2 : -1);
        latchy = (p.device == DeviceJustifiers ? p.justifier.y2 : -1);
      }
    } break;
  }

  if(latchy < 0 || latchy >= (ppu.overscan() ? 240 : 225) || latchx < 0 || latchx >= 256) {
    //cursor is offscreen, set to invalid position so counters are not latched
    latchx = ~0;
    latchy = ~0;
  } else {
    //cursor is onscreen
    latchx += 40;  //offset trigger position to simulate hardware latching delay
    latchx <<= 2;  //dot -> clock conversion
    latchx +=  2;  //align trigger on half-dot ala interrupts (speed optimization for sCPU::add_clocks)
  }
}

void Input::port_set_device(bool portnumber, unsigned device) {
  port_t &p = port[portnumber];

  p.device = device;
  p.counter0 = 0;
  p.counter1 = 0;

  //set iobit to true if device is capable of latching PPU counters
  iobit = port[1].device == DeviceSuperScope
       || port[1].device == DeviceJustifier
       || port[1].device == DeviceJustifiers;
  latchx = -1;
  latchy = -1;

  if(device == DeviceSuperScope) {
    p.superscope.x = 256 / 2;
    p.superscope.y = 240 / 2;

    p.superscope.trigger   = false;
    p.superscope.cursor    = false;
    p.superscope.turbo     = false;
    p.superscope.pause     = false;
    p.superscope.offscreen = false;

    p.superscope.turbolock   = false;
    p.superscope.triggerlock = false;
    p.superscope.pauselock   = false;
  } else if(device == DeviceJustifier) {
    p.justifier.active = 0;
    p.justifier.x1 = 256 / 2;
    p.justifier.y1 = 240 / 2;
    p.justifier.x2 = -1;
    p.justifier.y2 = -1;

    p.justifier.trigger1 = false;
    p.justifier.trigger2 = false;
    p.justifier.start1 = false;
    p.justifier.start2 = false;
  } else if(device == DeviceJustifiers) {
    p.justifier.active = 0;
    p.justifier.x1 = 256 / 2 - 16;
    p.justifier.y1 = 240 / 2;
    p.justifier.x2 = 256 / 2 + 16;
    p.justifier.y2 = 240 / 2;

    p.justifier.trigger1 = false;
    p.justifier.trigger2 = false;
    p.justifier.start1 = false;
    p.justifier.start2 = false;
  }
}

void Input::poll() {
  port[0].counter0 = 0;
  port[0].counter1 = 0;
  port[1].counter0 = 0;
  port[1].counter1 = 0;

  port[1].justifier.active = !port[1].justifier.active;
}

void Input::init() {
}

#endif
