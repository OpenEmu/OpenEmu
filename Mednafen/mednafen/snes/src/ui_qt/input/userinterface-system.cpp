InputGroup userInterfaceSystem(InputCategory::UserInterface, "System");

namespace UserInterfaceSystem {

struct LoadCartridge : HotkeyInput {
  void pressed() {
    diskBrowser->loadCartridge();
  }

  LoadCartridge() : HotkeyInput("Load Cartridge", "input.userInterface.system.loadCartridge") {
    name = "Shift+KB0::L";
    userInterfaceSystem.attach(this);
  }
} loadCartridge;

struct LoadBsxSlottedCartridge : HotkeyInput {
  void pressed() {
    loaderWindow->loadBsxSlottedCartridge("", "");
  }

  LoadBsxSlottedCartridge() : HotkeyInput("Load BS-X Slotted Cartridge", "input.userInterface.system.loadBsxSlottedcartridge") {
    userInterfaceSystem.attach(this);
  }
} loadBsxSlottedCartridge;

struct LoadBsxCartridge : HotkeyInput {
  void pressed() {
    loaderWindow->loadBsxCartridge(config().path.bsx, "");
  }

  LoadBsxCartridge() : HotkeyInput("Load BS-X Cartridge", "input.userInterface.system.loadBsxCartridge") {
    userInterfaceSystem.attach(this);
  }
} loadBsxCartridge;

struct LoadSufamiTurboCartridge : HotkeyInput {
  void pressed() {
    loaderWindow->loadSufamiTurboCartridge(config().path.st, "", "");
  }

  LoadSufamiTurboCartridge() : HotkeyInput("Load Sufami Turbo Cartridge", "input.userInterface.system.loadSufamiTurboCartridge") {
    userInterfaceSystem.attach(this);
  }
} loadSufamiTurboCartridge;

struct LoadSuperGameBoyCartridge : HotkeyInput {
  void pressed() {
    loaderWindow->loadSuperGameBoyCartridge(config().path.sgb, "");
  }

  LoadSuperGameBoyCartridge() : HotkeyInput("Load Super Game Boy Cartridge", "input.userInterface.system.loadSuperGameBoyCartridge") {
    userInterfaceSystem.attach(this);
  }
} loadSuperGameBoyCartridge;

struct PowerCycle : HotkeyInput {
  void pressed() {
    utility.modifySystemState(Utility::PowerCycle);
  }

  PowerCycle() : HotkeyInput("Power Cycle", "input.userInterface.system.powerCycle") {
    userInterfaceSystem.attach(this);
  }
} powerCycle;

struct Reset : HotkeyInput {
  void pressed() {
    utility.modifySystemState(Utility::Reset);
  }

  Reset() : HotkeyInput("Reset", "input.userInterface.system.reset") {
    userInterfaceSystem.attach(this);
  }
} reset;

struct Pause : HotkeyInput {
  void pressed() {
    application.pause = !application.pause;
    if(application.pause) audio.clear();
  }

  Pause() : HotkeyInput("Pause", "input.userInterface.system.pause") {
    name = "KB0::Pause";
    userInterfaceSystem.attach(this);
  }
} pause;

}
