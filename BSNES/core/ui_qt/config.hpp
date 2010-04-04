class Configuration : public configuration {
public:
  struct System {
    string video, audio, input;
    bool crashedOnLastRun;
    unsigned speed;
    bool autoSaveMemory;
    bool autoHideMenus;
  } system;

  struct File {
    bool autodetect_type;
    bool bypass_patch_crc32;
  } file;

  struct DiskBrowser {
    bool showPanel;
  } diskBrowser;

  struct Path {
    string base;     //binary path
    string user;     //user profile path (bsnes.cfg, ...)
    string current;  //current working directory (path to currently loaded cartridge)
    string rom, save, state, patch, cheat, data;
    string bsx, st, sgb;
  } path;

  struct Video {
    bool isFullscreen;
    bool synchronize;
    signed contrastAdjust, brightnessAdjust, gammaAdjust;
    bool enableGammaRamp;
    double ntscAspectRatio, palAspectRatio;

    struct Context {
      bool correctAspectRatio;
      unsigned multiplier, region;
      unsigned hwFilter, swFilter;
    } *context, windowed, fullscreen;
  } video;

  struct Audio {
    bool synchronize;
    bool mute;
    unsigned volume, latency, outputFrequency, inputFrequency;
  } audio;

  struct Input {
    enum policy_t { FocusPolicyPauseEmulation, FocusPolicyIgnoreInput, FocusPolicyAllowInput };
    unsigned focusPolicy;
    bool allowInvalidInput;

    struct Joypad {
      string up, down, left, right, a, b, x, y, l, r, select, start;
    } joypad1, joypad2,
      multitap1a, multitap1b, multitap1c, multitap1d,
      multitap2a, multitap2b, multitap2c, multitap2d;

    struct Mouse {
      string x, y, left, right;
    } mouse1, mouse2;

    struct SuperScope {
      string x, y, trigger, turbo, cursor, pause;
    } superscope;

    struct Justifier {
      string x, y, trigger, start;
    } justifier1, justifier2;

    struct UiGeneral {
      string loadCartridge;
      string pauseEmulation;
      string resetSystem;
      string powerCycleSystem;
      string captureScreenshot;
      string showStateManager;
      string quickLoad1;
      string quickLoad2;
      string quickLoad3;
      string quickSave1;
      string quickSave2;
      string quickSave3;
      string lowerSpeed;
      string raiseSpeed;
      string toggleCheatSystem;
      string toggleFullscreen;
      string toggleMenu;
      string toggleStatus;
      string exitEmulator;
    } uiGeneral;
  } input;

  struct Geometry {
    string mainWindow;
    string loaderWindow;
    string htmlViewerWindow;
    string aboutWindow;
    string diskBrowser;
    string folderCreator;

    string settingsWindow;
    string inputCaptureWindow;
    string inputMouseCaptureWindow;
    string inputCalibrationWindow;

    string toolsWindow;

    string debugger;
    string breakpointEditor;
    string memoryEditor;
    string vramViewer;
  } geometry;

  bool load(const char *filename);
  void attachJoypad(Configuration::Input::Joypad &joypad, const char *name);
  Configuration();
} config;
