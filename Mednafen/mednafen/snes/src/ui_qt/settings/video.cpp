#include "video.moc"
VideoSettingsWindow *videoSettingsWindow;

VideoSettingsWindow::VideoSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  sliders = new QGridLayout;
  layout->addLayout(sliders);

  contrastLabel = new QLabel("Contrast adjust:");
  sliders->addWidget(contrastLabel, 0, 0);

  contrastValue = new QLabel;
  contrastValue->setAlignment(Qt::AlignHCenter);
  contrastValue->setMinimumWidth(contrastValue->fontMetrics().width("+100%"));
  sliders->addWidget(contrastValue, 0, 1);

  contrast = new QSlider(Qt::Horizontal);
  contrast->setMinimum(-95);
  contrast->setMaximum(+95);
  sliders->addWidget(contrast, 0, 2);

  brightnessLabel = new QLabel("Brightness adjust:");
  sliders->addWidget(brightnessLabel, 1, 0);

  brightnessValue = new QLabel;
  brightnessValue->setAlignment(Qt::AlignHCenter);
  sliders->addWidget(brightnessValue, 1, 1);

  brightness = new QSlider(Qt::Horizontal);
  brightness->setMinimum(-95);
  brightness->setMaximum(+95);
  sliders->addWidget(brightness, 1, 2);

  gammaLabel = new QLabel("Gamma adjust:");
  sliders->addWidget(gammaLabel, 2, 0);

  gammaValue = new QLabel;
  gammaValue->setAlignment(Qt::AlignHCenter);
  sliders->addWidget(gammaValue, 2, 1);

  gamma = new QSlider(Qt::Horizontal);
  gamma->setMinimum(-95);
  gamma->setMaximum(+95);
  sliders->addWidget(gamma, 2, 2);

  scanlineLabel = new QLabel("Scanline adjust:");
  sliders->addWidget(scanlineLabel, 3, 0);

  scanlineValue = new QLabel;
  scanlineValue->setAlignment(Qt::AlignHCenter);
  sliders->addWidget(scanlineValue, 3, 1);

  scanline = new QSlider(Qt::Horizontal);
  scanline->setMinimum(0);
  scanline->setMaximum(20);
  scanline->setPageStep(4);
  sliders->addWidget(scanline, 3, 2);

  options = new QHBoxLayout;
  layout->addLayout(options);

  enableGammaRamp = new QCheckBox("Simulate NTSC TV gamma ramp");
  enableGammaRamp->setToolTip("Lower monitor gamma to more accurately match a CRT television");
  options->addWidget(enableGammaRamp);

  pixelShaderWindow = new PixelShaderWindow;
  layout->addWidget(pixelShaderWindow);

  connect(contrast, SIGNAL(valueChanged(int)), this, SLOT(contrastAdjust(int)));
  connect(brightness, SIGNAL(valueChanged(int)), this, SLOT(brightnessAdjust(int)));
  connect(gamma, SIGNAL(valueChanged(int)), this, SLOT(gammaAdjust(int)));
  connect(scanline, SIGNAL(valueChanged(int)), this, SLOT(scanlineAdjust(int)));
  connect(enableGammaRamp, SIGNAL(stateChanged(int)), this, SLOT(gammaRampToggle(int)));

  syncUi();
}

void VideoSettingsWindow::syncUi() {
  int n;

  n = config().video.contrastAdjust;
  contrastValue->setText(string() << (n > 0 ? "+" : "") << n << "%");
  contrast->setSliderPosition(n);

  n = config().video.brightnessAdjust;
  brightnessValue->setText(string() << (n > 0 ? "+" : "") << n << "%");
  brightness->setSliderPosition(n);

  n = config().video.gammaAdjust;
  gammaValue->setText(string() << (n > 0 ? "+" : "") << n << "%");
  gamma->setSliderPosition(n);

  n = config().video.scanlineAdjust;
  scanlineValue->setText(string() << n << "%");
  scanline->setSliderPosition(n / 5);

  enableGammaRamp->setChecked(config().video.enableGammaRamp);
}

void VideoSettingsWindow::contrastAdjust(int value) {
  config().video.contrastAdjust = value;
  syncUi();
  utility.updateColorFilter();
}

void VideoSettingsWindow::brightnessAdjust(int value) {
  config().video.brightnessAdjust = value;
  syncUi();
  utility.updateColorFilter();
}

void VideoSettingsWindow::gammaAdjust(int value) {
  config().video.gammaAdjust = value;
  syncUi();
  utility.updateColorFilter();
}

void VideoSettingsWindow::scanlineAdjust(int value) {
  config().video.scanlineAdjust = value * 5;
  syncUi();
  scanlineFilter.setIntensity(value * 5);
}

void VideoSettingsWindow::gammaRampToggle(int state) {
  config().video.enableGammaRamp = (state == Qt::Checked);
  syncUi();
  utility.updateColorFilter();
}
