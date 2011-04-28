class CanvasObject : public QWidget {
public:
  void dragEnterEvent(QDragEnterEvent*);
  void dropEvent(QDropEvent*);
  void keyPressEvent(QKeyEvent*);
  void keyReleaseEvent(QKeyEvent*);
};

class CanvasWidget : public CanvasObject {
public:
  QPaintEngine* paintEngine() const;
  void mouseReleaseEvent(QMouseEvent*);
  void paintEvent(QPaintEvent*);
};

class MainWindow : public QbWindow {
  Q_OBJECT

public:
  QMenuBar *menuBar;
  QStatusBar *statusBar;
  QVBoxLayout *layout;
  QMenu *system;
    QAction *system_load;
    QMenu *system_loadSpecial;
      QAction *system_loadSpecial_bsxSlotted;
      QAction *system_loadSpecial_bsx;
      QAction *system_loadSpecial_sufamiTurbo;
      QAction *system_loadSpecial_superGameBoy;
    QbCheckAction *system_power;
    QAction *system_reset;
    QMenu *system_port1;
      QbRadioAction *system_port1_none;
      QbRadioAction *system_port1_gamepad;
      QbRadioAction *system_port1_asciipad;
      QbRadioAction *system_port1_multitap;
      QbRadioAction *system_port1_mouse;
    QMenu *system_port2;
      QbRadioAction *system_port2_none;
      QbRadioAction *system_port2_gamepad;
      QbRadioAction *system_port2_asciipad;
      QbRadioAction *system_port2_multitap;
      QbRadioAction *system_port2_mouse;
      QbRadioAction *system_port2_superscope;
      QbRadioAction *system_port2_justifier;
      QbRadioAction *system_port2_justifiers;
    QAction *system_exit;
  QMenu *settings;
    QMenu *settings_videoMode;
      QbRadioAction *settings_videoMode_1x;
      QbRadioAction *settings_videoMode_2x;
      QbRadioAction *settings_videoMode_3x;
      QbRadioAction *settings_videoMode_4x;
      QbRadioAction *settings_videoMode_5x;
      QbCheckAction *settings_videoMode_correctAspectRatio;
      QbRadioAction *settings_videoMode_ntsc;
      QbRadioAction *settings_videoMode_pal;
    QMenu *settings_videoFilter;
      QAction *settings_videoFilter_configure;
      QbRadioAction *settings_videoFilter_none;
      array<QbRadioAction*> settings_videoFilter_list;
    QbCheckAction *settings_smoothVideo;
    QbCheckAction *settings_muteAudio;
    QMenu *settings_emulationSpeed;
      QbRadioAction *settings_emulationSpeed_slowest;
      QbRadioAction *settings_emulationSpeed_slow;
      QbRadioAction *settings_emulationSpeed_normal;
      QbRadioAction *settings_emulationSpeed_fast;
      QbRadioAction *settings_emulationSpeed_fastest;
      QbCheckAction *settings_emulationSpeed_syncVideo;
      QbCheckAction *settings_emulationSpeed_syncAudio;
    QAction *settings_configuration;
  QMenu *tools;
    QMenu *tools_movies;
      QAction *tools_movies_play;
      QAction *tools_movies_stop;
      QAction *tools_movies_recordFromPowerOn;
      QAction *tools_movies_recordFromHere;
    QAction *tools_captureScreenshot;
    QAction *tools_dialog;
    QAction *tools_debugger;
  QMenu *help;
    QAction *help_documentation;
    QAction *help_license;
    QAction *help_about;

  CanvasObject *canvasContainer;
  QVBoxLayout *canvasLayout;
  CanvasWidget *canvas;
  QLabel *systemState;

  void syncUi();
  bool isActive();
  void closeEvent(QCloseEvent*);
  MainWindow();

public slots:
  void loadCartridge();
  void loadBsxSlottedCartridge();
  void loadBsxCartridge();
  void loadSufamiTurboCartridge();
  void loadSuperGameBoyCartridge();
  void power();
  void reset();
  void setPort1None();
  void setPort1Gamepad();
  void setPort1Asciipad();
  void setPort1Multitap();
  void setPort1Mouse();
  void setPort2None();
  void setPort2Gamepad();
  void setPort2Asciipad();
  void setPort2Multitap();
  void setPort2Mouse();
  void setPort2SuperScope();
  void setPort2Justifier();
  void setPort2Justifiers();
  void quit();
  void setVideoMode1x();
  void setVideoMode2x();
  void setVideoMode3x();
  void setVideoMode4x();
  void setVideoMode5x();
  void toggleAspectCorrection();
  void setVideoNtsc();
  void setVideoPal();
  void configureFilter();
  void setFilter();
  void toggleSmoothVideo();
  void muteAudio();
  void setSpeedSlowest();
  void setSpeedSlow();
  void setSpeedNormal();
  void setSpeedFast();
  void setSpeedFastest();
  void syncVideo();
  void syncAudio();
  void showConfigWindow();
  void playMovie();
  void stopMovie();
  void recordMovieFromPowerOn();
  void recordMovieFromHere();
  void saveScreenshot();
  void showDebugger();
  void showToolsDialog();
  void showDocumentation();
  void showLicense();
  void showAbout();
};

extern MainWindow *mainWindow;
