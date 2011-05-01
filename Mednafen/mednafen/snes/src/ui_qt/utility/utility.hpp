class Utility {
public:
  //utility.cpp
  void inputEvent(uint16_t scancode);
  void showMessage(const char *message);
  void updateSystemState();
  void acquireMouse();
  void unacquireMouse();

  void updateAvSync();
  void updateVideoMode();
  void updateColorFilter();
  void updatePixelShader();
  void updateHardwareFilter();
  void updateSoftwareFilter();
  void updateEmulationSpeed();
  void updateControllers();

  //system-state.cpp
  enum system_state_t { LoadCartridge, UnloadCartridge, PowerOn, PowerOff, PowerCycle, Reset };
  void modifySystemState(system_state_t state);

  //window.cpp
  void updateFullscreenState();
  void constrainSize(unsigned &x, unsigned &y, unsigned max);
  void resizeMainWindow();
  void toggleSynchronizeVideo();
  void toggleSynchronizeAudio();
  void setNtscMode();
  void setPalMode();
  void toggleSmoothVideoOutput();
  void toggleAspectCorrection();
  void setScale(unsigned);
  void toggleFullscreen();
  void toggleMenubar();
  void toggleStatusbar();
};

extern Utility utility;
