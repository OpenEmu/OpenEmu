#ifdef CONTROLLER_CPP

//The Super Scope is a light-gun: it detects the CRT beam cannon position,
//and latches the counters by toggling iobit. This only works on controller
//port 2, as iobit there is connected to the PPU H/V counter latch.
//(PIO $4201.d7)

//It is obviously not possible to perfectly simulate an IR light detecting
//a CRT beam cannon, hence this class will read the PPU raster counters.

//A Super Scope can still technically be used in port 1, however it would
//require manual polling of PIO ($4201.d6) to determine when iobit was written.
//Note that no commercial game ever utilizes a Super Scope in port 1.

void SuperScope::enter() {
  unsigned prev = 0;
  while(true) {
    unsigned next = cpu.vcounter() * 1364 + cpu.hcounter();

    if(offscreen == false) {
      unsigned target = y * 1364 + (x + 24) * 4;
      if(next >= target && prev < target) {
        //CRT raster detected, toggle iobit to latch counters
        iobit(0);
        iobit(1);
      }
    }

    if(next < prev) {
      //Vcounter wrapped back to zero; update cursor coordinates for start of new frame
      int nx = interface->inputPoll(port, (unsigned)Input::Device::SuperScope, (unsigned)Input::SuperScopeID::X);
      int ny = interface->inputPoll(port, (unsigned)Input::Device::SuperScope, (unsigned)Input::SuperScopeID::Y);
      nx += x;
      ny += y;
      x = max(-16, min(256 + 16, nx));
      y = max(-16, min(240 + 16, ny));
      offscreen = (x < 0 || y < 0 || x >= 256 || y >= (ppu.overscan() ? 240 : 225));
    }

    prev = next;
    step(2);
  }
}

uint2 SuperScope::data() {
  if(counter >= 8) return 1;

  if(counter == 0) {
    //turbo is a switch; toggle is edge sensitive
    bool newturbo = interface->inputPoll(port, (unsigned)Input::Device::SuperScope, (unsigned)Input::SuperScopeID::Turbo);
    if(newturbo && !turbo) {
      turbo = !turbo;  //toggle state
      turbolock = true;
    } else {
      turbolock = false;
    }

    //trigger is a button
    //if turbo is active, trigger is level sensitive; otherwise, it is edge sensitive
    trigger = false;
    bool newtrigger = interface->inputPoll(port, (unsigned)Input::Device::SuperScope, (unsigned)Input::SuperScopeID::Trigger);
    if(newtrigger && (turbo || !triggerlock)) {
      trigger = true;
      triggerlock = true;
    } else if(!newtrigger) {
      triggerlock = false;
    }

    //cursor is a button; it is always level sensitive
    cursor = interface->inputPoll(port, (unsigned)Input::Device::SuperScope, (unsigned)Input::SuperScopeID::Cursor);

    //pause is a button; it is always edge sensitive
    pause = false;
    bool newpause = interface->inputPoll(port, (unsigned)Input::Device::SuperScope, (unsigned)Input::SuperScopeID::Pause);
    if(newpause && !pauselock) {
      pause = true;
      pauselock = true;
    } else if(!newpause) {
      pauselock = false;
    }

    offscreen = (x < 0 || y < 0 || x >= 256 || y >= (ppu.overscan() ? 240 : 225));
  }

  switch(counter++) {
  case 0: return offscreen ? 0 : trigger;
  case 1: return cursor;
  case 2: return turbo;
  case 3: return pause;
  case 4: return 0;
  case 5: return 0;
  case 6: return offscreen;
  case 7: return 0;  //noise (1 = yes)
  }
}

void SuperScope::latch(bool data) {
  if(latched == data) return;
  latched = data;
  counter = 0;
}

SuperScope::SuperScope(bool port) : Controller(port) {
  create(Controller::Enter, 21477272);
  latched = 0;
  counter = 0;

  //center cursor onscreen
  x = 256 / 2;
  y = 240 / 2;

  trigger   = false;
  cursor    = false;
  turbo     = false;
  pause     = false;
  offscreen = false;

  turbolock   = false;
  triggerlock = false;
  pauselock   = false;
}

#endif
