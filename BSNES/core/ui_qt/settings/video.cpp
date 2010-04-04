VideoSettingsWindow::VideoSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  title = new QLabel("Video Settings");
  title->setProperty("class", "title");
  layout->addWidget(title);

  sliders = new QGridLayout; {
    lcontrast = new QLabel("Contrast adjust: +100%");
    sliders->addWidget(lcontrast, 0, 0);

    contrast = new QSlider(Qt::Horizontal);
    contrast->setMinimum(-95);
    contrast->setMaximum(+95);
    sliders->addWidget(contrast, 0, 1);

    lbrightness = new QLabel("Brightness adjust: +100%");
    sliders->addWidget(lbrightness, 1, 0);

    brightness = new QSlider(Qt::Horizontal);
    brightness->setMinimum(-95);
    brightness->setMaximum(+95);
    sliders->addWidget(brightness, 1, 1);

    lgamma = new QLabel("Gamma adjust: +100%");
    sliders->addWidget(lgamma, 2, 0);

    gamma = new QSlider(Qt::Horizontal);
    gamma->setMinimum(-95);
    gamma->setMaximum(+95);
    sliders->addWidget(gamma, 2, 1);
  }
  sliders->setSpacing(Style::WidgetSpacing);
  layout->addLayout(sliders);
  layout->addSpacing(Style::WidgetSpacing);

  options = new QHBoxLayout; {
    options->setMargin(0);

    enableGammaRamp = new QCheckBox("Simulate NTSC TV gamma ramp");
    enableGammaRamp->setToolTip("Lower monitor gamma to more accurately match a CRT television");
    options->addWidget(enableGammaRamp);
  }
  options->setSpacing(Style::WidgetSpacing);
  layout->addLayout(options);

  connect(contrast, SIGNAL(valueChanged(int)), this, SLOT(contrastAdjust(int)));
  connect(brightness, SIGNAL(valueChanged(int)), this, SLOT(brightnessAdjust(int)));
  connect(gamma, SIGNAL(valueChanged(int)), this, SLOT(gammaAdjust(int)));
  connect(enableGammaRamp, SIGNAL(stateChanged(int)), this, SLOT(gammaRampToggle(int)));

  syncUi();
}

void VideoSettingsWindow::syncUi() {
  int n;

  n = config.video.contrastAdjust;
  lcontrast->setText(utf8() << "Contrast adjust: " << (n > 0 ? "+" : "") << n << "%");
  contrast->setSliderPosition(n);

  n = config.video.brightnessAdjust;
  lbrightness->setText(utf8() << "Brightness adjust: " << (n > 0 ? "+" : "") << n << "%");
  brightness->setSliderPosition(n);

  n = config.video.gammaAdjust;
  lgamma->setText(utf8() << "Gamma adjust: " << (n > 0 ? "+" : "") << n << "%");
  gamma->setSliderPosition(n);

  enableGammaRamp->setChecked(config.video.enableGammaRamp);
}

void VideoSettingsWindow::gammaRampToggle(int state) {
  config.video.enableGammaRamp = (state == Qt::Checked);
  syncUi();
  utility.updateColorFilter();
}

void VideoSettingsWindow::contrastAdjust(int value) {
  config.video.contrastAdjust = value;
  syncUi();
  utility.updateColorFilter();
}

void VideoSettingsWindow::brightnessAdjust(int value) {
  config.video.brightnessAdjust = value;
  syncUi();
  utility.updateColorFilter();
}

void VideoSettingsWindow::gammaAdjust(int value) {
  config.video.gammaAdjust = value;
  syncUi();
  utility.updateColorFilter();
}
