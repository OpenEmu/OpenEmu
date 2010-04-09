#include "cartridge.cpp"
#include "state.cpp"
#include "window.cpp"

//returns true if requested code is a button, and it has just been pressed down
bool Utility::isButtonDown(uint16_t inputCode, InputObject &object) {
  if(inputCode != object.code) return false;

  if(object.codetype != InputCode::KeyboardButton
  && object.codetype != InputCode::MouseButton
  && object.codetype != InputCode::JoypadHat
  && object.codetype != InputCode::JoypadAxis
  && object.codetype != InputCode::JoypadButton) return false;

  int16_t state     = inputManager.state(object.code);
  int16_t lastState = inputManager.lastState(object.code);

  if(object.codetype == InputCode::JoypadHat) {
    switch(object.modifier) {
      case InputObject::Up:    return (state & joypad<>::hat_up   ) && !(lastState & joypad<>::hat_up   );
      case InputObject::Down:  return (state & joypad<>::hat_down ) && !(lastState & joypad<>::hat_down );
      case InputObject::Left:  return (state & joypad<>::hat_left ) && !(lastState & joypad<>::hat_left );
      case InputObject::Right: return (state & joypad<>::hat_right) && !(lastState & joypad<>::hat_right);
    }
  } else if(object.codetype == InputCode::JoypadAxis) {
    switch(object.modifier) {
      case InputObject::Lo: return (state < -16384) && !(lastState < -16384);
      case InputObject::Hi: return (state > +16384) && !(lastState > +16384);
      case InputObject::Trigger: return (state < 0) && !(lastState < 0);
    }
  } else {
    return (state == 1) && !(lastState == 1);
  }

  return false;  //fall-through for modifier-less hats / axes
}

void Utility::inputEvent(uint16_t code) {
  //forward key-press event
  //(in case window is currently active and capturing a new button / axis assignment)
  inputCaptureWindow->inputEvent(code);

  for(unsigned i = 0; i < keyboard<>::count; i++) {
    if(code == keyboard<>::index(i, keyboard<>::escape) && inputManager.state(code)) {
      if(mainWindow->isActive() && input.acquired()) {
        //release mouse capture
        input.unacquire();
        return;  //do not trigger other UI actions that may be bound to escape key
      }
    }
  }

  if(mainWindow->isActive()) {
    bool resizeWindow = false;

    if(isButtonDown(code, inputUiGeneral.loadCartridge)) {
      diskBrowser->loadCartridge();
    }

    if(isButtonDown(code, inputUiGeneral.pauseEmulation)) {
      application.pause = !application.pause;
      if(application.pause) audio.clear();
    }

    if(isButtonDown(code, inputUiGeneral.resetSystem)) {
      modifySystemState(Reset);
    }

    if(isButtonDown(code, inputUiGeneral.powerCycleSystem)) {
      modifySystemState(PowerCycle);
    }

    if(isButtonDown(code, inputUiGeneral.captureScreenshot)) {
      //tell SNES::Interface to save a screenshot at the next video_refresh() event
      interface.saveScreenshot = true;
    }

    if(isButtonDown(code, inputUiGeneral.showStateManager)) {
      toolsWindow->showStateManager();
    }

    if(isButtonDown(code, inputUiGeneral.quickLoad1)) utility.quickLoad(0);
    if(isButtonDown(code, inputUiGeneral.quickLoad2)) utility.quickLoad(1);
    if(isButtonDown(code, inputUiGeneral.quickLoad3)) utility.quickLoad(2);
    if(isButtonDown(code, inputUiGeneral.quickSave1)) utility.quickSave(0);
    if(isButtonDown(code, inputUiGeneral.quickSave2)) utility.quickSave(1);
    if(isButtonDown(code, inputUiGeneral.quickSave3)) utility.quickSave(2);

    if(isButtonDown(code, inputUiGeneral.lowerSpeed)) {
      config.system.speed--;
      updateEmulationSpeed();
      mainWindow->syncUi();
    }

    if(isButtonDown(code, inputUiGeneral.raiseSpeed)) {
      config.system.speed++;
      updateEmulationSpeed();
      mainWindow->syncUi();
    }

    if(isButtonDown(code, inputUiGeneral.toggleCheatSystem)) {
      if(SNES::cheat.enabled() == false) {
        SNES::cheat.enable();
        showMessage("Cheat system enabled.");
      } else {
        SNES::cheat.disable();
        showMessage("Cheat system disabled.");
      }
    }

    if(isButtonDown(code, inputUiGeneral.toggleFullscreen)) {
      config.video.isFullscreen = !config.video.isFullscreen;
      updateFullscreenState();
      resizeMainWindow();
      mainWindow->syncUi();
    }

    if(isButtonDown(code, inputUiGeneral.toggleMenu)) {
      mainWindow->menuBar->setVisible(!mainWindow->menuBar->isVisibleTo(mainWindow));
      resizeWindow = true;
    }

    if(isButtonDown(code, inputUiGeneral.toggleStatus)) {
      mainWindow->statusBar->setVisible(!mainWindow->statusBar->isVisibleTo(mainWindow));
      resizeWindow = true;
    }

    //prevent calling twice when toggleMenu == toggleStatus
    if(resizeWindow == true) {
      resizeMainWindow();
    }

    if(isButtonDown(code, inputUiGeneral.exitEmulator)) {
      application.terminate = true;
    }
  }
}

//display message in main window statusbar area for three seconds
void Utility::showMessage(const char *message) {
  mainWindow->statusBar->showMessage(utf8() << message, 3000);
}

//updates system state text at bottom-right of main window statusbar
void Utility::updateSystemState() {
  string text;

  if(SNES::cartridge.loaded() == false) {
    text = "No cartridge loaded";
  } else if(application.power == false) {
    text = "Power off";
  } else if(application.pause == true || application.autopause == true) {
    text = "Paused";
  } else if(SNES::ppu.status.frames_updated == true) {
    SNES::ppu.status.frames_updated = false;
    text << (int)SNES::ppu.status.frames_executed;
    text << " fps";
  } else {
    //nothing to update
    return;
  }

  mainWindow->systemState->setText(utf8() << text);
}

void Utility::acquireMouse() {
  if(SNES::cartridge.loaded()) {
    if(SNES::config.controller_port1 == SNES::Input::DeviceMouse
    || SNES::config.controller_port2 == SNES::Input::DeviceMouse
    || SNES::config.controller_port2 == SNES::Input::DeviceSuperScope
    || SNES::config.controller_port2 == SNES::Input::DeviceJustifier
    || SNES::config.controller_port2 == SNES::Input::DeviceJustifiers
    ) input.acquire();
  }
}

void Utility::unacquireMouse() {
  input.unacquire();
}

void Utility::updateAvSync() {
  video.set(Video::Synchronize, config.video.synchronize);
  audio.set(Audio::Synchronize, config.audio.synchronize);
}

void Utility::updateVideoMode() {
  if(config.video.context->region == 0) {
    SNES::video.set_mode(SNES::Video::ModeNTSC);
  } else {
    SNES::video.set_mode(SNES::Video::ModePAL);
  }
}

void Utility::updateColorFilter() {
  filter.contrast = config.video.contrastAdjust;
  filter.brightness = config.video.brightnessAdjust;
  filter.gamma = 100 + config.video.gammaAdjust;
  filter.gamma_ramp = config.video.enableGammaRamp;
  filter.colortable_update();
}

void Utility::updateHardwareFilter() {
  video.set(Video::Filter, config.video.context->hwFilter);
}

void Utility::updateSoftwareFilter() {
  filter.renderer = config.video.context->swFilter;

/*libfilter::FilterInterface::FilterType type;
  switch(config.video.context->swFilter) { default:
    case 0: type = libfilter::FilterInterface::Direct;   break;
    case 1: type = libfilter::FilterInterface::Scanline; break;
    case 2: type = libfilter::FilterInterface::Scale2x;  break;
    case 3: type = libfilter::FilterInterface::LQ2x;     break;
    case 4: type = libfilter::FilterInterface::HQ2x;     break;
    case 5: type = libfilter::FilterInterface::NTSC;     break;
  }
  libfilter::filter.set(type);

  if(type == libfilter::FilterInterface::NTSC) {
    libfilter::filter_ntsc.adjust(0, 0, 0, 0, 0, config.video.enableNtscMergeFields);
  }*/
}

void Utility::updateEmulationSpeed() {
  config.system.speed = max(0, min(4, (signed)config.system.speed));

  double scale[] = { 0.50, 0.75, 1.00, 1.50, 2.00 };
  unsigned outfreq = config.audio.outputFrequency;
  unsigned infreq  = config.audio.inputFrequency * scale[config.system.speed] + 0.5;

  audio.set(Audio::Resample, true);  //always resample (required for volume adjust + frequency scaler)
  audio.set(Audio::ResampleRatio, (double)infreq / (double)outfreq);
}

void Utility::updateControllers() {
  SNES::input.port_set_device(0, SNES::config.controller_port1);
  SNES::input.port_set_device(1, SNES::config.controller_port2);
}
