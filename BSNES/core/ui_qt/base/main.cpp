MainWindow::MainWindow() : QbWindow(config.geometry.mainWindow) {
  setObjectName("main-window");
  setWindowTitle(utf8() << bsnesTitle << " v" << bsnesVersion);

  //menu bar
  #if defined(PLATFORM_OSX)
  menuBar = new QMenuBar(0);
  #else
  menuBar = new QMenuBar;
  #endif

  system = menuBar->addMenu("System");
    system_load = system->addAction("Load Cartridge ...");
    system_load->setIcon(QIcon(":/16x16/document-open.png"));
    system_loadSpecial = system->addMenu("Load Special");
    system_loadSpecial->setIcon(QIcon(":/16x16/document-open.png"));
      system_loadSpecial_bsxSlotted = system_loadSpecial->addAction("Load BS-X Slotted Cartridge ...");
      system_loadSpecial_bsxSlotted->setIcon(QIcon(":/16x16/document-open.png"));
      system_loadSpecial_bsx = system_loadSpecial->addAction("Load BS-X Cartridge ...");
      system_loadSpecial_bsx->setIcon(QIcon(":/16x16/document-open.png"));
      system_loadSpecial_sufamiTurbo = system_loadSpecial->addAction("Load Sufami Turbo Cartridge ...");
      system_loadSpecial_sufamiTurbo->setIcon(QIcon(":/16x16/document-open.png"));
      system_loadSpecial_superGameBoy = system_loadSpecial->addAction("Load Super Game Boy Cartridge ...");
      system_loadSpecial_superGameBoy->setIcon(QIcon(":/16x16/document-open.png"));
    system->addSeparator();
    system->addAction(system_power = new QbCheckAction("Power", 0));
    system_reset = system->addAction("Reset");
    system_reset->setIcon(QIcon(":/16x16/view-refresh.png"));
    system->addSeparator();
    system_port1 = system->addMenu("Controller Port 1");
    system_port1->setIcon(QIcon(":/16x16/input-gaming.png"));
      system_port1->addAction(system_port1_none = new QbRadioAction("None", 0));
      system_port1->addAction(system_port1_joypad = new QbRadioAction("Joypad", 0));
      system_port1->addAction(system_port1_multitap = new QbRadioAction("Multitap", 0));
      system_port1->addAction(system_port1_mouse = new QbRadioAction("Mouse", 0));
    system_port2 = system->addMenu("Controller Port 2");
    system_port2->setIcon(QIcon(":/16x16/input-gaming.png"));
      system_port2->addAction(system_port2_none = new QbRadioAction("None", 0));
      system_port2->addAction(system_port2_joypad = new QbRadioAction("Joypad", 0));
      system_port2->addAction(system_port2_multitap = new QbRadioAction("Multitap", 0));
      system_port2->addAction(system_port2_mouse = new QbRadioAction("Mouse", 0));
      system_port2->addAction(system_port2_superscope = new QbRadioAction("Super Scope", 0));
      system_port2->addAction(system_port2_justifier = new QbRadioAction("Justifier", 0));
      system_port2->addAction(system_port2_justifiers = new QbRadioAction("Two Justifiers", 0));
    #if !defined(PLATFORM_OSX)
    system->addSeparator();
    #endif
    system_exit = system->addAction("Exit");
    system_exit->setIcon(QIcon(":/16x16/process-stop.png"));
    system_exit->setMenuRole(QAction::QuitRole);

  settings = menuBar->addMenu("Settings");
    settings_videoMode = settings->addMenu("Video Mode");
    settings_videoMode->setIcon(QIcon(":/16x16/video-display.png"));
      settings_videoMode->addAction(settings_videoMode_1x = new QbRadioAction("Scale 1x", 0));
      settings_videoMode->addAction(settings_videoMode_2x = new QbRadioAction("Scale 2x", 0));
      settings_videoMode->addAction(settings_videoMode_3x = new QbRadioAction("Scale 3x", 0));
      settings_videoMode->addAction(settings_videoMode_4x = new QbRadioAction("Scale 4x", 0));
      settings_videoMode->addAction(settings_videoMode_max = new QbRadioAction("Scale Max", 0));
      settings_videoMode->addSeparator();
      settings_videoMode->addAction(settings_videoMode_correctAspectRatio = new QbCheckAction("Correct Aspect Ratio", 0));
      settings_videoMode->addAction(settings_videoMode_fullscreen = new QbCheckAction("Fullscreen", 0));
      settings_videoMode->addSeparator();
      settings_videoMode->addAction(settings_videoMode_ntsc = new QbRadioAction("NTSC", 0));
      settings_videoMode->addAction(settings_videoMode_pal = new QbRadioAction("PAL", 0));

    if(filter.opened()) {
      settings_videoFilter = settings->addMenu("Video Filter");
      settings_videoFilter->setIcon(QIcon(":/16x16/image-x-generic.png"));

      settings_videoFilter_configure = settings_videoFilter->addAction("Configure Active Filter ...");
      settings_videoFilter_configure->setIcon(QIcon(":/16x16/preferences-desktop.png"));
      settings_videoFilter->addSeparator();

      settings_videoFilter->addAction(settings_videoFilter_none = new QbRadioAction("None", 0));
      settings_videoFilter_list.add(settings_videoFilter_none);

      lstring filterlist;
      filterlist.split(";", filter.dl_supported());
      for(unsigned i = 0; i < filterlist.size(); i++) {
        QbRadioAction *action = new QbRadioAction(utf8() << filterlist[i], 0);
        settings_videoFilter->addAction(action);
        settings_videoFilter_list.add(action);
      }
    }

    settings->addAction(settings_smoothVideo = new QbCheckAction("Smooth Video Output", 0));
    settings->addSeparator();
    settings->addAction(settings_muteAudio = new QbCheckAction("Mute Audio Output", 0));
    settings->addSeparator();
    settings_emulationSpeed = settings->addMenu("Emulation Speed");
    settings_emulationSpeed->setIcon(QIcon(":/16x16/appointment-new.png"));
      settings_emulationSpeed->addAction(settings_emulationSpeed_slowest = new QbRadioAction("50%", 0));
      settings_emulationSpeed->addAction(settings_emulationSpeed_slow = new QbRadioAction("75%", 0));
      settings_emulationSpeed->addAction(settings_emulationSpeed_normal = new QbRadioAction("100%", 0));
      settings_emulationSpeed->addAction(settings_emulationSpeed_fast = new QbRadioAction("150%", 0));
      settings_emulationSpeed->addAction(settings_emulationSpeed_fastest = new QbRadioAction("200%", 0));
      settings_emulationSpeed->addSeparator();
      settings_emulationSpeed->addAction(settings_emulationSpeed_syncVideo = new QbCheckAction("Sync Video", 0));
      settings_emulationSpeed->addAction(settings_emulationSpeed_syncAudio = new QbCheckAction("Sync Audio", 0));
    settings_configuration = settings->addAction("Configuration ...");
    settings_configuration->setIcon(QIcon(":/16x16/preferences-desktop.png"));
    settings_configuration->setMenuRole(QAction::PreferencesRole);

  tools = menuBar->addMenu("Tools");
    tools_cheatEditor = tools->addAction("Cheat Editor ...");
    tools_cheatEditor->setIcon(QIcon(":/16x16/accessories-text-editor.png"));
    tools_cheatFinder = tools->addAction("Cheat Finder ...");
    tools_cheatFinder->setIcon(QIcon(":/16x16/system-search.png"));
    tools_stateManager = tools->addAction("State Manager ...");
    tools_stateManager->setIcon(QIcon(":/16x16/system-file-manager.png"));
    tools->addSeparator();
    tools_captureScreenshot = tools->addAction("Capture Screenshot");
    tools_captureScreenshot->setIcon(QIcon(":/16x16/image-x-generic.png"));
    tools_debugger = tools->addAction("Debugger ...");
    tools_debugger->setIcon(QIcon(":/16x16/utilities-terminal.png"));
    #if !defined(DEBUGGER)
    tools_debugger->setVisible(false);
    #endif

  help = menuBar->addMenu("Help");
    help_documentation = help->addAction("Documentation ...");
    help_documentation->setIcon(QIcon(":/16x16/text-x-generic.png"));
    help_license = help->addAction("License ...");
    help_license->setIcon(QIcon(":/16x16/text-x-generic.png"));
    #if !defined(PLATFORM_OSX)
    help->addSeparator();
    #endif
    help_about = help->addAction("About ...");
    help_about->setIcon(QIcon(":/16x16/help-browser.png"));
    help_about->setMenuRole(QAction::AboutRole);

  //canvas
  canvasContainer = new CanvasObject;
  canvasContainer->setAcceptDrops(true); {
    canvasContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    canvasContainer->setObjectName("backdrop");

    canvasLayout = new QVBoxLayout; {
      canvasLayout->setMargin(0);
      canvasLayout->setAlignment(Qt::AlignCenter);

      canvas = new CanvasWidget;
      canvas->setAcceptDrops(true);
      canvas->setFocusPolicy(Qt::StrongFocus);
      canvas->setAttribute(Qt::WA_PaintOnScreen, true);  //disable Qt painting on focus / resize

      QPalette palette;
      palette.setColor(QPalette::Window, QColor(0, 0, 0));
      canvas->setPalette(palette);
      canvas->setAutoFillBackground(true);
    }
    canvasLayout->addWidget(canvas);
  }
  canvasContainer->setLayout(canvasLayout);

  //status bar
  statusBar = new QStatusBar;
  statusBar->showMessage("");
  systemState = new QLabel;
  statusBar->addPermanentWidget(systemState);

  //layout
  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  #if !defined(PLATFORM_OSX)
  layout->addWidget(menuBar);
  #endif
  layout->addWidget(canvasContainer);
  layout->addWidget(statusBar);
  setLayout(layout);

  //slots
  connect(system_load, SIGNAL(triggered()), this, SLOT(loadCartridge()));
  connect(system_loadSpecial_bsxSlotted, SIGNAL(triggered()), this, SLOT(loadBsxSlottedCartridge()));
  connect(system_loadSpecial_bsx, SIGNAL(triggered()), this, SLOT(loadBsxCartridge()));
  connect(system_loadSpecial_sufamiTurbo, SIGNAL(triggered()), this, SLOT(loadSufamiTurboCartridge()));
  connect(system_loadSpecial_superGameBoy, SIGNAL(triggered()), this, SLOT(loadSuperGameBoyCartridge()));
  connect(system_power, SIGNAL(triggered()), this, SLOT(power()));
  connect(system_reset, SIGNAL(triggered()), this, SLOT(reset()));
  connect(system_port1_none, SIGNAL(triggered()), this, SLOT(setPort1None()));
  connect(system_port1_joypad, SIGNAL(triggered()), this, SLOT(setPort1Joypad()));
  connect(system_port1_multitap, SIGNAL(triggered()), this, SLOT(setPort1Multitap()));
  connect(system_port1_mouse, SIGNAL(triggered()), this, SLOT(setPort1Mouse()));
  connect(system_port2_none, SIGNAL(triggered()), this, SLOT(setPort2None()));
  connect(system_port2_joypad, SIGNAL(triggered()), this, SLOT(setPort2Joypad()));
  connect(system_port2_multitap, SIGNAL(triggered()), this, SLOT(setPort2Multitap()));
  connect(system_port2_mouse, SIGNAL(triggered()), this, SLOT(setPort2Mouse()));
  connect(system_port2_superscope, SIGNAL(triggered()), this, SLOT(setPort2SuperScope()));
  connect(system_port2_justifier, SIGNAL(triggered()), this, SLOT(setPort2Justifier()));
  connect(system_port2_justifiers, SIGNAL(triggered()), this, SLOT(setPort2Justifiers()));
  connect(system_exit, SIGNAL(triggered()), this, SLOT(quit()));
  connect(settings_videoMode_1x, SIGNAL(triggered()), this, SLOT(setVideoMode1x()));
  connect(settings_videoMode_2x, SIGNAL(triggered()), this, SLOT(setVideoMode2x()));
  connect(settings_videoMode_3x, SIGNAL(triggered()), this, SLOT(setVideoMode3x()));
  connect(settings_videoMode_4x, SIGNAL(triggered()), this, SLOT(setVideoMode4x()));
  connect(settings_videoMode_max, SIGNAL(triggered()), this, SLOT(setVideoModeMax()));
  connect(settings_videoMode_correctAspectRatio, SIGNAL(triggered()), this, SLOT(toggleAspectCorrection()));
  connect(settings_videoMode_fullscreen, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));
  connect(settings_videoMode_ntsc, SIGNAL(triggered()), this, SLOT(setVideoNtsc()));
  connect(settings_videoMode_pal, SIGNAL(triggered()), this, SLOT(setVideoPal()));
  if(filter.opened()) {
    connect(settings_videoFilter_configure, SIGNAL(triggered()), this, SLOT(configureFilter()));
    for(unsigned i = 0; i < settings_videoFilter_list.size(); i++) {
      connect(settings_videoFilter_list[i], SIGNAL(triggered()), this, SLOT(setFilter()));
    }
  }
  connect(settings_smoothVideo, SIGNAL(triggered()), this, SLOT(toggleSmoothVideo()));
  connect(settings_muteAudio, SIGNAL(triggered()), this, SLOT(muteAudio()));
  connect(settings_emulationSpeed_slowest, SIGNAL(triggered()), this, SLOT(setSpeedSlowest()));
  connect(settings_emulationSpeed_slow, SIGNAL(triggered()), this, SLOT(setSpeedSlow()));
  connect(settings_emulationSpeed_normal, SIGNAL(triggered()), this, SLOT(setSpeedNormal()));
  connect(settings_emulationSpeed_fast, SIGNAL(triggered()), this, SLOT(setSpeedFast()));
  connect(settings_emulationSpeed_fastest, SIGNAL(triggered()), this, SLOT(setSpeedFastest()));
  connect(settings_emulationSpeed_syncVideo, SIGNAL(triggered()), this, SLOT(syncVideo()));
  connect(settings_emulationSpeed_syncAudio, SIGNAL(triggered()), this, SLOT(syncAudio()));
  connect(settings_configuration, SIGNAL(triggered()), this, SLOT(showConfigWindow()));
  connect(tools_cheatEditor, SIGNAL(triggered()), this, SLOT(showCheatEditor()));
  connect(tools_cheatFinder, SIGNAL(triggered()), this, SLOT(showCheatFinder()));
  connect(tools_stateManager, SIGNAL(triggered()), this, SLOT(showStateManager()));
  connect(tools_captureScreenshot, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
  connect(tools_debugger, SIGNAL(triggered()), this, SLOT(showDebugger()));
  connect(help_documentation, SIGNAL(triggered()), this, SLOT(showDocumentation()));
  connect(help_license, SIGNAL(triggered()), this, SLOT(showLicense()));
  connect(help_about, SIGNAL(triggered()), this, SLOT(showAbout()));

  syncUi();
}

void MainWindow::syncUi() {
  system_power->setEnabled(SNES::cartridge.loaded());
  system_power->setChecked (application.power == true);
  system_power->setEnabled(SNES::cartridge.loaded());
  system_reset->setEnabled(SNES::cartridge.loaded() && application.power);

  system_port1_none->setChecked      (SNES::config.controller_port1 == SNES::Input::DeviceNone);
  system_port1_joypad->setChecked    (SNES::config.controller_port1 == SNES::Input::DeviceJoypad);
  system_port1_multitap->setChecked  (SNES::config.controller_port1 == SNES::Input::DeviceMultitap);
  system_port1_mouse->setChecked     (SNES::config.controller_port1 == SNES::Input::DeviceMouse);
  system_port2_none->setChecked      (SNES::config.controller_port2 == SNES::Input::DeviceNone);
  system_port2_joypad->setChecked    (SNES::config.controller_port2 == SNES::Input::DeviceJoypad);
  system_port2_multitap->setChecked  (SNES::config.controller_port2 == SNES::Input::DeviceMultitap);
  system_port2_mouse->setChecked     (SNES::config.controller_port2 == SNES::Input::DeviceMouse);
  system_port2_superscope->setChecked(SNES::config.controller_port2 == SNES::Input::DeviceSuperScope);
  system_port2_justifier->setChecked (SNES::config.controller_port2 == SNES::Input::DeviceJustifier);
  system_port2_justifiers->setChecked(SNES::config.controller_port2 == SNES::Input::DeviceJustifiers);

  settings_videoMode_1x->setChecked (config.video.context->multiplier == 1);
  settings_videoMode_2x->setChecked (config.video.context->multiplier == 2);
  settings_videoMode_3x->setChecked (config.video.context->multiplier == 3);
  settings_videoMode_4x->setChecked (config.video.context->multiplier == 4);
  settings_videoMode_max->setChecked(config.video.context->multiplier >= 5);

  settings_videoMode_correctAspectRatio->setChecked(config.video.context->correctAspectRatio);
  settings_videoMode_fullscreen->setChecked(config.video.isFullscreen);
  settings_videoMode_ntsc->setChecked(config.video.context->region == 0);
  settings_videoMode_pal->setChecked (config.video.context->region == 1);

  if(filter.opened()) {
    //only enable configuration option if the active filter supports it ...
    settings_videoFilter_configure->setEnabled(filter.settings());

    for(unsigned i = 0; i < settings_videoFilter_list.size(); i++) {
      settings_videoFilter_list[i]->setChecked(config.video.context->swFilter == i);
    }
  }

  settings_smoothVideo->setChecked(config.video.context->hwFilter == 1);
  settings_muteAudio->setChecked(config.audio.mute);

  settings_emulationSpeed_slowest->setChecked(config.system.speed == 0);
  settings_emulationSpeed_slow->setChecked   (config.system.speed == 1);
  settings_emulationSpeed_normal->setChecked (config.system.speed == 2);
  settings_emulationSpeed_fast->setChecked   (config.system.speed == 3);
  settings_emulationSpeed_fastest->setChecked(config.system.speed == 4);

  settings_emulationSpeed_syncVideo->setChecked(config.video.synchronize);
  settings_emulationSpeed_syncAudio->setChecked(config.audio.synchronize);
}

bool MainWindow::isActive() {
  return isActiveWindow() && !isMinimized();
}

void MainWindow::loadCartridge() {
  diskBrowser->loadCartridge();
}

void MainWindow::loadBsxSlottedCartridge() {
  loaderWindow->loadBsxSlottedCartridge("", "");
}

void MainWindow::loadBsxCartridge() {
  loaderWindow->loadBsxCartridge(config.path.bsx, "");
}

void MainWindow::loadSufamiTurboCartridge() {
  loaderWindow->loadSufamiTurboCartridge(config.path.st, "", "");
}

void MainWindow::loadSuperGameBoyCartridge() {
  loaderWindow->loadSuperGameBoyCartridge(config.path.sgb, "");
}

void MainWindow::power() {
  system_power->toggleChecked();
  if(system_power->isChecked()) {
    utility.modifySystemState(Utility::PowerOn);
  } else {
    utility.modifySystemState(Utility::PowerOff);
  }
}

void MainWindow::reset() {
  utility.modifySystemState(Utility::Reset);
}

void MainWindow::setPort1None()       { SNES::config.controller_port1 = SNES::Input::DeviceNone;       utility.updateControllers(); syncUi(); }
void MainWindow::setPort1Joypad()     { SNES::config.controller_port1 = SNES::Input::DeviceJoypad;     utility.updateControllers(); syncUi(); }
void MainWindow::setPort1Multitap()   { SNES::config.controller_port1 = SNES::Input::DeviceMultitap;   utility.updateControllers(); syncUi(); }
void MainWindow::setPort1Mouse()      { SNES::config.controller_port1 = SNES::Input::DeviceMouse;      utility.updateControllers(); syncUi(); }
void MainWindow::setPort2None()       { SNES::config.controller_port2 = SNES::Input::DeviceNone;       utility.updateControllers(); syncUi(); }
void MainWindow::setPort2Joypad()     { SNES::config.controller_port2 = SNES::Input::DeviceJoypad;     utility.updateControllers(); syncUi(); }
void MainWindow::setPort2Multitap()   { SNES::config.controller_port2 = SNES::Input::DeviceMultitap;   utility.updateControllers(); syncUi(); }
void MainWindow::setPort2Mouse()      { SNES::config.controller_port2 = SNES::Input::DeviceMouse;      utility.updateControllers(); syncUi(); }
void MainWindow::setPort2SuperScope() { SNES::config.controller_port2 = SNES::Input::DeviceSuperScope; utility.updateControllers(); syncUi(); }
void MainWindow::setPort2Justifier()  { SNES::config.controller_port2 = SNES::Input::DeviceJustifier;  utility.updateControllers(); syncUi(); }
void MainWindow::setPort2Justifiers() { SNES::config.controller_port2 = SNES::Input::DeviceJustifiers; utility.updateControllers(); syncUi(); }

void MainWindow::quit() {
  hide();
  application.terminate = true;
}

void MainWindow::setVideoMode1x()  { config.video.context->multiplier = 1; utility.resizeMainWindow(); syncUi(); }
void MainWindow::setVideoMode2x()  { config.video.context->multiplier = 2; utility.resizeMainWindow(); syncUi(); }
void MainWindow::setVideoMode3x()  { config.video.context->multiplier = 3; utility.resizeMainWindow(); syncUi(); }
void MainWindow::setVideoMode4x()  { config.video.context->multiplier = 4; utility.resizeMainWindow(); syncUi(); }
void MainWindow::setVideoModeMax() { config.video.context->multiplier = 9; utility.resizeMainWindow(); syncUi(); }

void MainWindow::toggleAspectCorrection() {
  settings_videoMode_correctAspectRatio->toggleChecked();
  config.video.context->correctAspectRatio = settings_videoMode_correctAspectRatio->isChecked();
  utility.resizeMainWindow();
}

void MainWindow::toggleFullscreen() {
  settings_videoMode_fullscreen->toggleChecked();
  config.video.isFullscreen = settings_videoMode_fullscreen->isChecked();
  utility.updateFullscreenState();
  utility.resizeMainWindow();
  syncUi();
}

void MainWindow::setVideoNtsc() { config.video.context->region = 0; utility.updateVideoMode(); utility.resizeMainWindow(); syncUi(); }
void MainWindow::setVideoPal()  { config.video.context->region = 1; utility.updateVideoMode(); utility.resizeMainWindow(); syncUi(); }

void MainWindow::toggleSmoothVideo() {
  settings_smoothVideo->toggleChecked();
  config.video.context->hwFilter = settings_smoothVideo->isChecked();
  utility.updateHardwareFilter();
  syncUi();
}

void MainWindow::configureFilter() {
  QWidget *widget = filter.settings();
  if(widget) {
    widget->show();
    widget->activateWindow();
    widget->raise();
  }
}

void MainWindow::setFilter() {
  for(unsigned i = 0; i < settings_videoFilter_list.size(); i++) {
    if(sender() == settings_videoFilter_list[i]) {
      config.video.context->swFilter = i;
      utility.updateSoftwareFilter();
      syncUi();
      return;
    }
  }
}

void MainWindow::muteAudio() {
  settings_muteAudio->toggleChecked();
  config.audio.mute = settings_muteAudio->isChecked();
}

void MainWindow::setSpeedSlowest() { config.system.speed = 0; utility.updateEmulationSpeed(); syncUi(); }
void MainWindow::setSpeedSlow()    { config.system.speed = 1; utility.updateEmulationSpeed(); syncUi(); }
void MainWindow::setSpeedNormal()  { config.system.speed = 2; utility.updateEmulationSpeed(); syncUi(); }
void MainWindow::setSpeedFast()    { config.system.speed = 3; utility.updateEmulationSpeed(); syncUi(); }
void MainWindow::setSpeedFastest() { config.system.speed = 4; utility.updateEmulationSpeed(); syncUi(); }

void MainWindow::syncVideo() {
  settings_emulationSpeed_syncVideo->toggleChecked();
  config.video.synchronize = settings_emulationSpeed_syncVideo->isChecked();
  utility.updateAvSync();
}

void MainWindow::syncAudio() {
  settings_emulationSpeed_syncAudio->toggleChecked();
  config.audio.synchronize = settings_emulationSpeed_syncAudio->isChecked();
  utility.updateAvSync();
}

void MainWindow::showConfigWindow() { settingsWindow->show(); }

void MainWindow::showCheatEditor()  { toolsWindow->showCheatEditor(); }
void MainWindow::showCheatFinder()  { toolsWindow->showCheatFinder(); }
void MainWindow::showStateManager() { toolsWindow->showStateManager(); }

void MainWindow::saveScreenshot() {
  //tell SNES::Interface to save a screenshot at the next video_refresh() event
  interface.saveScreenshot = true;
}

void MainWindow::showDebugger() { debugger->show(); }

void MainWindow::showDocumentation()  {
  QFile file(":/documentation.html");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    htmlViewerWindow->show("Usage Documentation", file.readAll().constData());
    file.close();
  }
}

void MainWindow::showLicense() {
  QFile file(":/license.html");
  if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    htmlViewerWindow->show("License Agreement", file.readAll().constData());
    file.close();
  }
}
void MainWindow::showAbout() {
  aboutWindow->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QbWindow::closeEvent(event);
  quit();
}

//============
//CanvasObject
//============

//implement drag-and-drop support:
//drag cartridge image onto main window canvas area to load

void CanvasObject::dragEnterEvent(QDragEnterEvent *event) {
  if(event->mimeData()->hasUrls()) {
    //do not accept multiple files at once
    if(event->mimeData()->urls().count() == 1) event->acceptProposedAction();
  }
}

void CanvasObject::dropEvent(QDropEvent *event) {
  if(event->mimeData()->hasUrls()) {
    QList<QUrl> list = event->mimeData()->urls();
    if(list.count() == 1) utility.loadCartridgeNormal(list.at(0).toLocalFile().toUtf8().constData());
  }
}

//accept all key events for this widget to prevent system chime from playing on OS X
//key events are actually handled by Input class

void CanvasObject::keyPressEvent(QKeyEvent *event) {
}

void CanvasObject::keyReleaseEvent(QKeyEvent *event) {
}

//===========
//CanvasWidget
//============

//custom video render and mouse capture functionality

QPaintEngine* CanvasWidget::paintEngine() const {
  if(SNES::cartridge.loaded()) {
    video.refresh();
    return 0;
  }
  return QWidget::paintEngine();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
  //acquire exclusive mode access to mouse when video output widget is clicked
  //(will only acquire if cart is loaded, and mouse / lightgun is in use.)
  utility.acquireMouse();
}

void CanvasWidget::paintEvent(QPaintEvent *event) {
  event->ignore();
}
