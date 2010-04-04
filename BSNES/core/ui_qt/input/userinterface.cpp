//==============
//InputGroupPool
//==============

void InputGroupPool::attach(InputGroup &group) {
  list.add(&group);
}

void InputGroupPool::bind() {
  for(unsigned i = 0; i < list.size(); i++) list[i]->bind();
}

void InputGroupPool::clear() {
  for(unsigned i = 0; i < list.size(); i++) list[i]->clear();
}

void InputGroupPool::poll(const int16_t *table) {
  for(unsigned i = 0; i < list.size(); i++) list[i]->poll(table);
}

InputGroupPool::InputGroupPool() : list(*this) {
}

//

InputUiGeneral inputUiGeneral;
InputUiPool inputUiPool;

InputUiGeneral::InputUiGeneral() : InputGroup("General"),
loadCartridge(InputObject::Button, "Load cartridge", config.input.uiGeneral.loadCartridge),
pauseEmulation(InputObject::Button, "Pause emulation", config.input.uiGeneral.pauseEmulation),
resetSystem(InputObject::Button, "Reset system", config.input.uiGeneral.resetSystem),
powerCycleSystem(InputObject::Button, "Power cycle system", config.input.uiGeneral.powerCycleSystem),

captureScreenshot(InputObject::Button, "Capture screenshot", config.input.uiGeneral.captureScreenshot),
showStateManager(InputObject::Button, "Show state manager window", config.input.uiGeneral.showStateManager),
quickLoad1(InputObject::Button, "Load from temporary state 1", config.input.uiGeneral.quickLoad1),
quickLoad2(InputObject::Button, "Load from temporary state 2", config.input.uiGeneral.quickLoad2),
quickLoad3(InputObject::Button, "Load from temporary state 3", config.input.uiGeneral.quickLoad3),
quickSave1(InputObject::Button, "Save to temporary state 1", config.input.uiGeneral.quickSave1),
quickSave2(InputObject::Button, "Save to temporary state 2", config.input.uiGeneral.quickSave2),
quickSave3(InputObject::Button, "Save to temporary state 3", config.input.uiGeneral.quickSave3),

lowerSpeed(InputObject::Button, "Decrease emulation speed", config.input.uiGeneral.lowerSpeed),
raiseSpeed(InputObject::Button, "Increase emulation speed", config.input.uiGeneral.raiseSpeed),
toggleCheatSystem(InputObject::Button, "Toggle cheat system on or off", config.input.uiGeneral.toggleCheatSystem),
toggleFullscreen(InputObject::Button, "Toggle fullscreen mode", config.input.uiGeneral.toggleFullscreen),
toggleMenu(InputObject::Button, "Toggle menubar", config.input.uiGeneral.toggleMenu),
toggleStatus(InputObject::Button, "Toggle statusbar", config.input.uiGeneral.toggleStatus),
exitEmulator(InputObject::Button, "Exit emulator", config.input.uiGeneral.exitEmulator) {
  attach(loadCartridge);
  attach(pauseEmulation);
  attach(resetSystem);
  attach(powerCycleSystem);

  attach(captureScreenshot);
  attach(showStateManager);
  attach(quickLoad1);
  attach(quickLoad2);
  attach(quickLoad3);
  attach(quickSave1);
  attach(quickSave2);
  attach(quickSave3);

  attach(lowerSpeed);
  attach(raiseSpeed);
  attach(toggleCheatSystem);
  attach(toggleFullscreen);
  attach(toggleMenu);
  attach(toggleStatus);
  attach(exitEmulator);
}

InputUiPool::InputUiPool() {
  attach(inputUiGeneral);
}
