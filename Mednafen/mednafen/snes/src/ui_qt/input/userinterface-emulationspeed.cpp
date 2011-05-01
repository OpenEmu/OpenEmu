InputGroup userInterfaceEmulationSpeed(InputCategory::UserInterface, "Emulation Speed");

namespace UserInterfaceEmulationSpeed {

//slowdown and speedup do not work well with Vsync enabled, as it locks the
//speed to the monitor refresh rate. thus, when one is pressed, it is disabled
//until the key is released.

struct Slowdown : HotkeyInput {
  bool syncVideo;

  void pressed() {
    config().system.speed = 0;
    utility.updateEmulationSpeed();
    syncVideo = config().video.synchronize;
    if(syncVideo) {
      config().video.synchronize = false;
      utility.updateAvSync();
    }
    mainWindow->syncUi();
  }

  void released() {
    config().system.speed = 2;
    utility.updateEmulationSpeed();
    if(syncVideo) {
      config().video.synchronize = true;
      utility.updateAvSync();
    }
    mainWindow->syncUi();
  }

  Slowdown() : HotkeyInput("Slowdown", "input.userInterface.emulationSpeed.slowdown") {
    userInterfaceEmulationSpeed.attach(this);
  }
} slowdown;

struct Speedup : HotkeyInput {
  bool syncVideo;

  void pressed() {
    config().system.speed = 4;
    utility.updateEmulationSpeed();
    syncVideo = config().video.synchronize;
    if(syncVideo) {
      config().video.synchronize = false;
      utility.updateAvSync();
    }
    mainWindow->syncUi();
  }

  void released() {
    config().system.speed = 2;
    utility.updateEmulationSpeed();
    if(syncVideo) {
      config().video.synchronize = true;
      utility.updateAvSync();
    }
    mainWindow->syncUi();
  }

  Speedup() : HotkeyInput("Speedup", "input.userInterface.emulationSpeed.speedup") {
    name = "KB0::Tilde";
    userInterfaceEmulationSpeed.attach(this);
  }
} speedup;

struct Decrease : HotkeyInput {
  void pressed() {
    if(config().system.speed > 0) config().system.speed--;
    utility.updateEmulationSpeed();
    mainWindow->syncUi();
  }

  Decrease() : HotkeyInput("Decrease", "input.userInterface.emulationSpeed.decrease") {
    name = "Control+KB0::Divide";
    userInterfaceEmulationSpeed.attach(this);
  }
} decrease;

struct Increase : HotkeyInput {
  void pressed() {
    if(config().system.speed < 4) config().system.speed++;
    utility.updateEmulationSpeed();
    mainWindow->syncUi();
  }

  Increase() : HotkeyInput("Increase", "input.userInterface.emulationSpeed.increase") {
    name = "Control+KB0::Multiply";
    userInterfaceEmulationSpeed.attach(this);
  }
} increase;

struct SetSlowestSpeed : HotkeyInput {
  void pressed() {
    config().system.speed = 0;
    utility.updateEmulationSpeed();
    mainWindow->syncUi();
  }

  SetSlowestSpeed() : HotkeyInput("Set Slowest Speed", "input.userInterface.emulationSpeed.setSlowest") {
    name = "Control+KB0::Num1";
    userInterfaceEmulationSpeed.attach(this);
  }
} setSlowestSpeed;

struct SetSlowSpeed : HotkeyInput {
  void pressed() {
    config().system.speed = 1;
    utility.updateEmulationSpeed();
    mainWindow->syncUi();
  }

  SetSlowSpeed() : HotkeyInput("Set Slow Speed", "input.userInterface.emulationSpeed.setSlow") {
    name = "Control+KB0::Num2";
    userInterfaceEmulationSpeed.attach(this);
  }
} setSlowSpeed;

struct SetNormalSpeed : HotkeyInput {
  void pressed() {
    config().system.speed = 2;
    utility.updateEmulationSpeed();
    mainWindow->syncUi();
  }

  SetNormalSpeed() : HotkeyInput("Set Normal Speed", "input.userInterface.emulationSpeed.setNormal") {
    name = "Control+KB0::Num3";
    userInterfaceEmulationSpeed.attach(this);
  }
} setNormalSpeed;

struct SetFastSpeed : HotkeyInput {
  void pressed() {
    config().system.speed = 3;
    utility.updateEmulationSpeed();
    mainWindow->syncUi();
  }

  SetFastSpeed() : HotkeyInput("Set Fast Speed", "input.userInterface.emulationSpeed.setFast") {
    name = "Control+KB0::Num4";
    userInterfaceEmulationSpeed.attach(this);
  }
} setFastSpeed;

struct SetFastestSpeed : HotkeyInput {
  void pressed() {
    config().system.speed = 4;
    utility.updateEmulationSpeed();
    mainWindow->syncUi();
  }

  SetFastestSpeed() : HotkeyInput("Set Fastest Speed", "input.userInterface.emulationSpeed.setFastest") {
    name = "Control+KB0::Num5";
    userInterfaceEmulationSpeed.attach(this);
  }
} setFastestSpeed;

struct SynchronizeVideo : HotkeyInput {
  void pressed() {
    utility.toggleSynchronizeVideo();
  }

  SynchronizeVideo() : HotkeyInput("Synchronize Video", "input.userInterface.emulationSpeed.synchronizeVideo") {
    name = "Control+KB0::V";
    userInterfaceEmulationSpeed.attach(this);
  }
} synchronizeVideo;

struct SynchronizeAudio : HotkeyInput {
  void pressed() {
    utility.toggleSynchronizeAudio();
  }

  SynchronizeAudio() : HotkeyInput("Synchronize Audio", "input.userInterface.emulationSpeed.synchronizeAudio") {
    name = "Control+KB0::A";
    userInterfaceEmulationSpeed.attach(this);
  }
} synchronizeAudio;

}
