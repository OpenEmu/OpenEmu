void Application::init() {
  if(config().system.crashedOnLastRun == true) {
    //emulator crashed on last run, disable all drivers
    QMessageBox::warning(0, "bsnes Crash Notification", string() <<
    "<p><b>Warning:</b><br>bsnes crashed while attempting to initialize device "
    "drivers the last time it was run.</p>"
    "<p>To prevent this from occurring again, all drivers have been disabled. Please "
    "go to Settings->Configuration->Advanced and choose new driver settings, and then "
    "restart the emulator for the changes to take effect. <i>Video, audio and input "
    "will not work until you do this!</i></p>"
    "<p><b>Settings that caused failure on last run:</b><br>"
    << "Video driver: " << config().system.video << "<br>"
    << "Audio driver: " << config().system.audio << "<br>"
    << "Input driver: " << config().system.input << "<br></p>"
    );

    config().system.video = "None";
    config().system.audio = "None";
    config().system.input = "None";
  }

  if(config().system.video == "") config().system.video = video.default_driver();
  if(config().system.audio == "") config().system.audio = audio.default_driver();
  if(config().system.input == "") config().system.input = input.default_driver();

  mainWindow = new MainWindow;
  loaderWindow = new LoaderWindow;
  htmlViewerWindow = new HtmlViewerWindow;
  aboutWindow = new AboutWindow;
  diskBrowser = new DiskBrowser;

  //window must be onscreen and visible before initializing video interface
  utility.updateSystemState();
  utility.resizeMainWindow();
  utility.updateFullscreenState();
  QApplication::processEvents();

  #if defined(DEBUGGER)
  debugger = new Debugger;
  #endif
  settingsWindow = new SettingsWindow;
  toolsWindow = new ToolsWindow;

  //if emulator crashes while initializing drivers, next run will disable them all.
  //this will allow user to choose different driver settings.
  config().system.crashedOnLastRun = true;
  config().save(configFilename);

  video.driver(config().system.video);
  video.set(Video::Handle, (uintptr_t)mainWindow->canvas->winId());
  video.set("QWidget", (QWidget*)mainWindow->canvas);
  if(video.init() == false) {
    QMessageBox::warning(0, "bsnes", string() <<
      "<p><b>Warning:</b> " << config().system.video << " video driver failed to initialize. "
      "Video driver has been disabled.</p>"
      "<p>Please go to Settings->Configuration->Advanced and choose a different driver, and "
      "then restart the emulator for the changes to take effect.</p>"
    );
    video.driver("None");
    video.init();
  }

  audio.driver(config().system.audio);
  audio.set(Audio::Handle, (uintptr_t)mainWindow->canvas->winId());
  audio.set(Audio::Frequency, config().audio.outputFrequency);
  audio.set(Audio::Latency, config().audio.latency);
  audio.set(Audio::Volume, config().audio.volume);
  if(audio.init() == false) {
    QMessageBox::warning(0, "bsnes", string() <<
      "<p><b>Warning:</b> " << config().system.audio << " audio driver failed to initialize. "
      "Audio driver has been disabled.</p>"
      "<p>Please go to Settings->Configuration->Advanced and choose a different driver, and "
      "then restart the emulator for the changes to take effect.</p>"
    );
    audio.driver("None");
    audio.init();
  }

  input.driver(config().system.input);
  input.set("Handle", (uintptr_t)mainWindow->canvas->winId());
  if(input.init() == false) {
    QMessageBox::warning(0, "bsnes", string() <<
      "<p><b>Warning:</b> " << config().system.input << " input driver failed to initialize. "
      "Input driver has been disabled.</p>"
      "<p>Please go to Settings->Configuration->Advanced and choose a different driver, and "
      "then restart the emulator for the changes to take effect.</p>"
    );
    input.driver("None");
    input.init();
  }

  //didn't crash, note this in the config file now in case a different kind of crash occurs later
  config().system.crashedOnLastRun = false;
  config().save(configFilename);

  //no sense showing unusable options ...
  pixelShaderWindow->setVisible(video.cap(Video::FragmentShader) || video.cap(Video::VertexShader));

  utility.resizeMainWindow();
  utility.updateAvSync();
  utility.updateVideoMode();
  utility.updateColorFilter();
  utility.updatePixelShader();
  utility.updateHardwareFilter();
  utility.updateSoftwareFilter();
  utility.updateEmulationSpeed();
  utility.updateControllers();
}
