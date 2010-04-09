class Utility {
public:
  //utility.cpp
  //bool isButtonDown(uint16_t inputCode, InputObject &object);
  void inputEvent(uint16_t code);
  void showMessage(const char *message);
  void updateSystemState();
  void acquireMouse();
  void unacquireMouse();

  void updateAvSync();
  void updateVideoMode();
  void updateColorFilter();
  void updateHardwareFilter();
  void updateSoftwareFilter();
  void updateEmulationSpeed();
  void updateControllers();

  //cartridge.cpp
  struct Cartridge {
    string name, baseName, slotAName, slotBName;
    bool patchApplied;
  } cartridge;

  string selectCartridge();
  string selectFolder(const char *title);
  void loadCartridge(const char*);
  bool loadCartridgeNormal(const char*);
  bool loadCartridgeBsxSlotted(const char*, const char*);
  bool loadCartridgeBsx(const char*, const char*);
  bool loadCartridgeSufamiTurbo(const char*, const char *, const char*);
  bool loadCartridgeSuperGameBoy(const char*, const char*);
  void saveMemory();
  void unloadCartridge();

  enum system_state_t { LoadCartridge, UnloadCartridge, PowerOn, PowerOff, PowerCycle, Reset };
  void modifySystemState(system_state_t state);

  bool loadCartridge(const char*, SNES::MappedRAM&);
  bool loadMemory(const char*, const char*, SNES::MappedRAM&);
  bool saveMemory(const char*, const char*, SNES::MappedRAM&);
  void loadCheats();
  void saveCheats();

  string filepath(const char *filename, const char *pathname);
  string basename(const char *filename);
  string basepath(const char *filename);

  //state.cpp
  bool saveStatesSupported();
  void quickLoad(uint8 slot);
  void quickSave(uint8 slot);
  void loadStateInfo(lstring&);
  void setStateDescription(uint8 slot, const char *description);
  void loadState(uint8 slot);
  void saveState(uint8 slot, const char *description);
  void deleteState(uint8 slot);

  //window.cpp
 /* void showCentered(QWidget *window);
  void updateFullscreenState();
  void constrainSize(unsigned &x, unsigned &y, unsigned max);
  void resizeMainWindow();
  */
} utility;
