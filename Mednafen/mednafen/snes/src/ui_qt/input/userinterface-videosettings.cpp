InputGroup userInterfaceVideoSettings(InputCategory::UserInterface, "Video Settings");

namespace UserInterfaceVideoSettings {

struct ToggleFullscreen : HotkeyInput {
  void pressed() {
    utility.toggleFullscreen();
  }

  ToggleFullscreen() : HotkeyInput("Toggle Fullscreen Mode", "input.userInterface.videoSettings.toggleFullscreen") {
    name = "Alt+KB0::Return";
    userInterfaceVideoSettings.attach(this);
  }
} toggleFullscreen;

struct SmoothVideoOutput : HotkeyInput {
  void pressed() {
    utility.toggleSmoothVideoOutput();
  }

  SmoothVideoOutput() : HotkeyInput("Smooth Video Output", "input.userInterface.videoSettings.smoothVideoOutput") {
    name = "Shift+KB0::S";
    userInterfaceVideoSettings.attach(this);
  }
} smoothVideoOutput;

struct SetNtscMode : HotkeyInput {
  void pressed() {
    utility.setNtscMode();
  }

  SetNtscMode() : HotkeyInput("Set NTSC Mode", "input.userInterface.videoSettings.ntscMode") {
    name = "Shift+KB0::N";
    userInterfaceVideoSettings.attach(this);
  }
} setNtscMode;

struct SetPalMode : HotkeyInput {
  void pressed() {
    utility.setPalMode();
  }

  SetPalMode() : HotkeyInput("Set PAL Mode", "input.userInterface.videoSettings.palMode") {
    name = "Shift+KB0::P";
    userInterfaceVideoSettings.attach(this);
  }
} setPalMode;

struct AspectCorrection : HotkeyInput {
  void pressed() {
    utility.toggleAspectCorrection();
  }

  AspectCorrection() : HotkeyInput("Aspect Correction", "input.userInterface.videoSettings.aspectCorrection") {
    name = "Shift+KB0::A";
    userInterfaceVideoSettings.attach(this);
  }
} aspectCorrection;

struct Scale1x : HotkeyInput {
  void pressed() {
    utility.setScale(1);
  }

  Scale1x() : HotkeyInput("Scale 1x", "input.userInterface.videoSettings.scale1x") {
    name = "Shift+KB0::Num1";
    userInterfaceVideoSettings.attach(this);
  }
} scale1x;

struct Scale2x : HotkeyInput {
  void pressed() {
    utility.setScale(2);
  }

  Scale2x() : HotkeyInput("Scale 2x", "input.userInterface.videoSettings.scale2x") {
    name = "Shift+KB0::Num2";
    userInterfaceVideoSettings.attach(this);
  }
} scale2x;

struct Scale3x : HotkeyInput {
  void pressed() {
    utility.setScale(3);
  }

  Scale3x() : HotkeyInput("Scale 3x", "input.userInterface.videoSettings.scale3x") {
    name = "Shift+KB0::Num3";
    userInterfaceVideoSettings.attach(this);
  }
} scale3x;

struct Scale4x : HotkeyInput {
  void pressed() {
    utility.setScale(4);
  }

  Scale4x() : HotkeyInput("Scale 4x", "input.userInterface.videoSettings.scale4x") {
    name = "Shift+KB0::Num4";
    userInterfaceVideoSettings.attach(this);
  }
} scale4x;

struct Scale5x : HotkeyInput {
  void pressed() {
    utility.setScale(5);
  }

  Scale5x() : HotkeyInput("Scale 5x", "input.userInterface.videoSettings.scale5x") {
    name = "Shift+KB0::Num5";
    userInterfaceVideoSettings.attach(this);
  }
} scale5x;

}
