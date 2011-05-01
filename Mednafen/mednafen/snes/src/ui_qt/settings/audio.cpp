#include "audio.moc"
AudioSettingsWindow *audioSettingsWindow;

AudioSettingsWindow::AudioSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  boxes = new QHBoxLayout;
  layout->addLayout(boxes);

  frequencyLabel = new QLabel("Frequency:");
  frequencyLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  boxes->addWidget(frequencyLabel);

  frequency = new QComboBox;
  frequency->addItem("32000hz");
  frequency->addItem("44100hz");
  frequency->addItem("48000hz");
  frequency->addItem("96000hz");
  boxes->addWidget(frequency);

  latencyLabel = new QLabel("Latency:");
  latencyLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  boxes->addWidget(latencyLabel);

  latency = new QComboBox;
  latency->addItem("20ms");
  latency->addItem("40ms");
  latency->addItem("60ms");
  latency->addItem("80ms");
  latency->addItem("100ms");
  latency->addItem("120ms");
  boxes->addWidget(latency);

  sliders = new QGridLayout;
  layout->addLayout(sliders);

  volumeLabel = new QLabel("Volume:");
  volumeLabel->setToolTip("Warning: any volume other than 100% will result in a slight audio quality loss");
  sliders->addWidget(volumeLabel, 0, 0);

  volumeValue = new QLabel;
  volumeValue->setAlignment(Qt::AlignHCenter);
  volumeValue->setMinimumWidth(volumeValue->fontMetrics().width("262144hz"));
  sliders->addWidget(volumeValue, 0, 1);

  volume = new QSlider(Qt::Horizontal);
  volume->setMinimum(0);
  volume->setMaximum(200);
  sliders->addWidget(volume, 0, 2);

  frequencySkewLabel = new QLabel("Input frequency:");
  frequencySkewLabel->setToolTip(
    "Adjusts audio resampling rate.\n"
    "When both video sync and audio sync are enabled, use this setting to fine-tune the output.\n"
    "Lower the input frequency to clean audio output, eliminating crackling / popping.\n"
    "Raise the input frequency to smooth video output, eliminating duplicated frames."
  );
  sliders->addWidget(frequencySkewLabel, 1, 0);

  frequencySkewValue = new QLabel;
  frequencySkewValue->setAlignment(Qt::AlignHCenter);
  sliders->addWidget(frequencySkewValue, 1, 1);

  frequencySkew = new QSlider(Qt::Horizontal);
  frequencySkew->setMinimum(31500);
  frequencySkew->setMaximum(32500);
  sliders->addWidget(frequencySkew, 1, 2);

  connect(frequency, SIGNAL(currentIndexChanged(int)), this, SLOT(frequencyChange(int)));
  connect(latency, SIGNAL(currentIndexChanged(int)), this, SLOT(latencyChange(int)));
  connect(volume, SIGNAL(valueChanged(int)), this, SLOT(volumeAdjust(int)));
  connect(frequencySkew, SIGNAL(valueChanged(int)), this, SLOT(frequencySkewAdjust(int)));

  syncUi();
}

void AudioSettingsWindow::syncUi() {
  int n;

  n = config().audio.outputFrequency;
       if(n <= 32000) frequency->setCurrentIndex(0);
  else if(n <= 44100) frequency->setCurrentIndex(1);
  else if(n <= 48000) frequency->setCurrentIndex(2);
  else if(n <= 96000) frequency->setCurrentIndex(3);
  else frequency->setCurrentIndex(0);

  n = config().audio.latency;
  latency->setCurrentIndex((n - 20) / 20);

  n = config().audio.volume;
  volumeValue->setText(string() << n << "%");
  volume->setSliderPosition(n);

  n = config().audio.inputFrequency;
  frequencySkewValue->setText(string() << n << "hz");
  frequencySkew->setSliderPosition(n);
}

void AudioSettingsWindow::frequencyChange(int value) {
  switch(value) { default:
    case 0: config().audio.outputFrequency = 32000; break;
    case 1: config().audio.outputFrequency = 44100; break;
    case 2: config().audio.outputFrequency = 48000; break;
    case 3: config().audio.outputFrequency = 96000; break;
  }
  audio.set(Audio::Frequency, config().audio.outputFrequency);
  utility.updateEmulationSpeed();
}

void AudioSettingsWindow::latencyChange(int value) {
  value = max(0, min(5, value));
  config().audio.latency = 20 + value * 20;
  audio.set(Audio::Latency, config().audio.latency);
}

void AudioSettingsWindow::volumeAdjust(int value) {
  config().audio.volume = value;
  audio.set(Audio::Volume, config().audio.volume);
  syncUi();
}

void AudioSettingsWindow::frequencySkewAdjust(int value) {
  config().audio.inputFrequency = value;
  utility.updateEmulationSpeed();
  syncUi();
}
