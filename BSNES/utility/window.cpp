//show specified window in the center of the screen.
void Utility::showCentered(QWidget *window) {
  QRect deskRect = QApplication::desktop()->availableGeometry(window);

  #if defined(PLATFORM_X)
  if(window->isVisible() == false) window->showMinimized();
  window->resize(0, 0);

  //Xlib returns a frame size of 0,0 at first; wait for it to be valid
  for(unsigned counter = 0; counter < 4096; counter++) {
    if(window->frameSize() != window->size()) break;
    application.processEvents();
  }

  window->move(
    deskRect.center().x() - (window->frameSize().width()  / 2),
    deskRect.center().y() - (window->frameSize().height() / 2)
  );

  for(unsigned counter = 0; counter < 4096; counter++) {
    window->showNormal();
    application.processEvents();
    if(window->isMinimized() == false) break;
  }
  #elif defined(PLATFORM_OSX)
  window->showNormal();
  window->resize(0, 0);
  window->move(
    deskRect.center().x() - (window->frameSize().width()  / 2),
    deskRect.center().y() - (window->frameSize().height() / 2)
  );
  #elif defined(PLATFORM_WIN)
  if(window->isMinimized() == false) {
    //place window offscreen, so that it can be shown to get proper frameSize()
    window->move(std::numeric_limits<signed>::max(), std::numeric_limits<signed>::max());
  }
  window->showNormal();
  window->resize(0, 0);
  window->move(
    deskRect.center().x() - (window->frameSize().width()  / 2),
    deskRect.center().y() - (window->frameSize().height() / 2)
  );
  #endif

  //ensure window has focus
  application.processEvents();
  window->activateWindow();
  window->raise();
}

void Utility::updateFullscreenState() {
  video.clear();

  if(config.video.isFullscreen == false) {
    config.video.context = &config.video.windowed;
    mainWindow->window->showNormal();
    mainWindow->window->menuBar()->setVisible(true);
    mainWindow->window->statusBar()->setVisible(true);
  } else {
    config.video.context = &config.video.fullscreen;
    mainWindow->window->showFullScreen();
    mainWindow->window->menuBar()->setVisible(!config.system.autoHideMenus);
    mainWindow->window->statusBar()->setVisible(!config.system.autoHideMenus);
  }

  application.processEvents();
  #if defined(PLATFORM_X)
  //Xlib requires time to propogate fullscreen state change message to window manager;
  //if window is resized before this occurs, canvas may not resize correctly
  usleep(50000);
  #endif

  //refresh options that are unique to each video context
  resizeMainWindow();
  updateVideoMode();
  updateHardwareFilter();
  updateSoftwareFilter();
  mainWindow->syncUi();
}

//if max exceeds x: x is set to max, and y is scaled down to keep proportion to x
void Utility::constrainSize(unsigned &x, unsigned &y, unsigned max) {
  if(x > max) {
    double scalar = (double)max / (double)x;
    y = (unsigned)((double)y * (double)scalar);
    x = max;
  }
}

void Utility::resizeMainWindow() {
  for(unsigned i = 0; i < 8; i++) {
    unsigned multiplier = config.video.context->multiplier;
    unsigned width  = 256 * config.video.context->multiplier;
    unsigned height = (config.video.context->region == 0 ? 224 : 239) * config.video.context->multiplier;

    if(config.video.context->correctAspectRatio) {
      if(config.video.context->region == 0) {
        width = (double)width * config.video.ntscAspectRatio + 0.5;  //NTSC adjust
      } else {
        width = (double)width * config.video.palAspectRatio  + 0.5;  //PAL adjust
      }
    }

    if(config.video.isFullscreen == false) {
      //get effective desktop work area region (ignore Windows taskbar, OS X dock, etc.)
      QRect deskRect = QApplication::desktop()->availableGeometry(mainWindow->window);

      if(mainWindow->window->isVisible() == false) {
        #if defined(PLATFORM_X)
        mainWindow->window->showMinimized();
        #elif defined(PLATFORM_OSX)
        mainWindow->window->showNormal();
        #elif defined(PLATFORM_WIN)
        mainWindow->window->move(std::numeric_limits<signed>::max(), std::numeric_limits<signed>::max());
        mainWindow->window->showNormal();
        #endif
      }

      //calculate frame geometry (window border + menubar + statusbar)
      unsigned frameWidth  = mainWindow->window->frameSize().width()  - mainWindow->canvasContainer->size().width();
      unsigned frameHeight = mainWindow->window->frameSize().height() - mainWindow->canvasContainer->size().height();

      //ensure window size will not be larger than viewable desktop area
      constrainSize(height, width, deskRect.height() - frameHeight);
      constrainSize(width, height, deskRect.width()  - frameWidth );

      //resize window such that it is as small as possible to hold canvas of size (width, height)
      mainWindow->canvas->setFixedSize(width, height);
      mainWindow->window->resize(0, 0);

      //center window onscreen
      mainWindow->window->move(
        deskRect.center().x() - (mainWindow->window->frameSize().width()  / 2),
        deskRect.center().y() - (mainWindow->window->frameSize().height() / 2)
      );
    } else {
      constrainSize(height, width, mainWindow->canvasContainer->size().height());
      constrainSize(width, height, mainWindow->canvasContainer->size().width());

      //center canvas onscreen; ensure it is not larger than viewable area
      mainWindow->canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      mainWindow->canvas->setFixedSize(width, height);
      mainWindow->canvas->setMinimumSize(0, 0);
    }

    application.processEvents();
    usleep(2000);
  }

  //workaround for Qt/Xlib bug:
  //if window resize occurs with cursor over it, Qt shows Qt::Size*DiagCursor;
  //so force it to show Qt::ArrowCursor, as expected
  mainWindow->window->setCursor(Qt::ArrowCursor);
  mainWindow->canvasContainer->setCursor(Qt::ArrowCursor);
  mainWindow->canvas->setCursor(Qt::ArrowCursor);

  //workaround for DirectSound(?) bug:
  //window resizing sometimes breaks audio sync, this call re-initializes it
  updateAvSync();
}
