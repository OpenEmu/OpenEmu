InputGroup userInterfaceGeneral(InputCategory::UserInterface, "General");

namespace UserInterfaceGeneral {

struct ToggleMenubar : HotkeyInput {
  void pressed() {
    utility.toggleMenubar();
  }

  ToggleMenubar() : HotkeyInput("Toggle Menubar", "input.userInterface.general.toggleMenubar") {
    name = "KB0::Tab";
    userInterfaceGeneral.attach(this);
  }
} toggleMenubar;

struct ToggleStatusbar : HotkeyInput {
  void pressed() {
    utility.toggleStatusbar();
  }

  ToggleStatusbar() : HotkeyInput("Toggle Statusbar", "input.userInterface.general.toggleStatusbar") {
    name = "KB0::Tab";
    userInterfaceGeneral.attach(this);
  }
} toggleStatusbar;

struct ToggleCheatSystem : HotkeyInput {
  void pressed() {
    if(SNES::cheat.enabled() == false) {
      SNES::cheat.enable(true);
      utility.showMessage("Cheat system enabled.");
    } else {
      SNES::cheat.enable(false);
      utility.showMessage("Cheat system disabled.");
    }
  }

  ToggleCheatSystem() : HotkeyInput("Toggle Cheat System", "input.userInterface.general.toggleCheatSystem") {
    userInterfaceGeneral.attach(this);
  }
} toggleCheatSystem;

struct CaptureScreenshot : HotkeyInput {
  void pressed() {
    //tell SNES::Interface to save a screenshot at the next video_refresh() event
    interface.saveScreenshot = true;
  }

  CaptureScreenshot() : HotkeyInput("Capture Screenshot", "input.userInterface.general.captureScreenshot") {
    userInterfaceGeneral.attach(this);
  }
} captureScreenshot;

//put here instead of in a separate "Audio Settings" group,
//because there is only one audio option at present
struct MuteAudioOutput : HotkeyInput {
  void pressed() {
    mainWindow->settings_muteAudio->toggleChecked();
    config().audio.mute = mainWindow->settings_muteAudio->isChecked();
  }

  MuteAudioOutput() : HotkeyInput("Mute Audio Output", "input.userInterface.general.muteAudioOutput") {
    name = "Shift+KB0::M";
    userInterfaceGeneral.attach(this);
  }
} muteAudioOutput;

}
