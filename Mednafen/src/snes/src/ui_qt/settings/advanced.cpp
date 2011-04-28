#include "advanced.moc"
AdvancedSettingsWindow *advancedSettingsWindow;

AdvancedSettingsWindow::AdvancedSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  driverLayout = new QGridLayout;
  driverLayout->setHorizontalSpacing(Style::WidgetSpacing);
  layout->addLayout(driverLayout);
  layout->addSpacing(Style::WidgetSpacing);

  videoLabel = new QLabel("Video driver:");
  driverLayout->addWidget(videoLabel, 0, 0);

  audioLabel = new QLabel("Audio driver:");
  driverLayout->addWidget(audioLabel, 0, 1);

  inputLabel = new QLabel("Input driver:");
  driverLayout->addWidget(inputLabel, 0, 2);

  videoDriver = new QComboBox;
  driverLayout->addWidget(videoDriver, 1, 0);

  audioDriver = new QComboBox;
  driverLayout->addWidget(audioDriver, 1, 1);

  inputDriver = new QComboBox;
  driverLayout->addWidget(inputDriver, 1, 2);

  driverInfo = new QLabel("<small>Note: driver changes require restart to take effect.</small>");
  driverInfo->setStyleSheet("margin-left: -3px; margin-top: 5px;");
  driverLayout->addWidget(driverInfo, 2, 0, 1, 3);

  regionTitle = new QLabel("Hardware region:");
  layout->addWidget(regionTitle);

  regionLayout = new QHBoxLayout;
  regionLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(regionLayout);
  layout->addSpacing(Style::WidgetSpacing);

  regionGroup = new QButtonGroup(this);

  regionAuto = new QRadioButton("Auto-detect");
  regionAuto->setToolTip("Automatically select hardware region on cartridge load");
  regionGroup->addButton(regionAuto);
  regionLayout->addWidget(regionAuto);

  regionNTSC = new QRadioButton("NTSC");
  regionNTSC->setToolTip("Force NTSC region (Japan, Korea, US)");
  regionGroup->addButton(regionNTSC);
  regionLayout->addWidget(regionNTSC);

  regionPAL = new QRadioButton("PAL");
  regionPAL->setToolTip("Force PAL region (Europe, ...)");
  regionGroup->addButton(regionPAL);
  regionLayout->addWidget(regionPAL);

  portTitle = new QLabel("Expansion port device:");
  layout->addWidget(portTitle);

  portLayout = new QHBoxLayout;
  portLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(portLayout);
  layout->addSpacing(Style::WidgetSpacing);

  portGroup = new QButtonGroup(this);

  portSatellaview = new QRadioButton("Satellaview");
  portGroup->addButton(portSatellaview);
  portLayout->addWidget(portSatellaview);

  portNone = new QRadioButton("None");
  portGroup->addButton(portNone);
  portLayout->addWidget(portNone);

  portSpacer = new QWidget;
  portLayout->addWidget(portSpacer);

  focusTitle = new QLabel("When main window does not have focus:");
  layout->addWidget(focusTitle);

  focusLayout = new QHBoxLayout;
  focusLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(focusLayout);
  layout->addSpacing(Style::WidgetSpacing);

  focusButtonGroup = new QButtonGroup(this);

  focusPause = new QRadioButton("Pause emulation");
  focusPause->setToolTip("Ideal for prolonged multi-tasking");
  focusButtonGroup->addButton(focusPause);
  focusLayout->addWidget(focusPause);

  focusIgnore = new QRadioButton("Ignore input");
  focusIgnore->setToolTip("Ideal for light multi-tasking when using keyboard");
  focusButtonGroup->addButton(focusIgnore);
  focusLayout->addWidget(focusIgnore);

  focusAllow = new QRadioButton("Allow input");
  focusAllow->setToolTip("Ideal for light multi-tasking when using joypad(s)");
  focusButtonGroup->addButton(focusAllow);
  focusLayout->addWidget(focusAllow);

  rewindTitle = new QLabel("Rewind support:");
  layout->addWidget(rewindTitle);

  rewindEnable = new QCheckBox("Enable");
  layout->addWidget(rewindEnable);

  initializeUi();

  connect(videoDriver, SIGNAL(currentIndexChanged(int)), this, SLOT(videoDriverChange(int)));
  connect(audioDriver, SIGNAL(currentIndexChanged(int)), this, SLOT(audioDriverChange(int)));
  connect(inputDriver, SIGNAL(currentIndexChanged(int)), this, SLOT(inputDriverChange(int)));
  connect(regionAuto, SIGNAL(pressed()), this, SLOT(setRegionAuto()));
  connect(regionNTSC, SIGNAL(pressed()), this, SLOT(setRegionNTSC()));
  connect(regionPAL, SIGNAL(pressed()), this, SLOT(setRegionPAL()));
  connect(portSatellaview, SIGNAL(pressed()), this, SLOT(setPortSatellaview()));
  connect(portNone, SIGNAL(pressed()), this, SLOT(setPortNone()));
  connect(focusPause, SIGNAL(pressed()), this, SLOT(pauseWithoutFocus()));
  connect(focusIgnore, SIGNAL(pressed()), this, SLOT(ignoreInputWithoutFocus()));
  connect(focusAllow, SIGNAL(pressed()), this, SLOT(allowInputWithoutFocus()));
  connect(rewindEnable, SIGNAL(stateChanged(int)), this, SLOT(toggleRewindEnable()));
}

void AdvancedSettingsWindow::initializeUi() {
  lstring part;

  part.split(";", video.driver_list());
  for(unsigned i = 0; i < part.size(); i++) {
    videoDriver->addItem(part[i]);
    if(part[i] == config().system.video) videoDriver->setCurrentIndex(i);
  }

  part.split(";", audio.driver_list());
  for(unsigned i = 0; i < part.size(); i++) {
    audioDriver->addItem(part[i]);
    if(part[i] == config().system.audio) audioDriver->setCurrentIndex(i);
  }

  part.split(";", input.driver_list());
  for(unsigned i = 0; i < part.size(); i++) {
    inputDriver->addItem(part[i]);
    if(part[i] == config().system.input) inputDriver->setCurrentIndex(i);
  }

  regionAuto->setChecked(SNES::config.region == SNES::System::Autodetect);
  regionNTSC->setChecked(SNES::config.region == SNES::System::NTSC);
  regionPAL->setChecked (SNES::config.region == SNES::System::PAL);

  portSatellaview->setChecked(SNES::config.expansion_port == SNES::System::ExpansionBSX);
  portNone->setChecked       (SNES::config.expansion_port == SNES::System::ExpansionNone);

  focusPause->setChecked (config().input.focusPolicy == Configuration::Input::FocusPolicyPauseEmulation);
  focusIgnore->setChecked(config().input.focusPolicy == Configuration::Input::FocusPolicyIgnoreInput);
  focusAllow->setChecked (config().input.focusPolicy == Configuration::Input::FocusPolicyAllowInput);

  rewindEnable->setChecked(config().system.rewindEnabled);
}

void AdvancedSettingsWindow::videoDriverChange(int index) {
  if(index >= 0) config().system.video = videoDriver->itemText(index).toUtf8().data();
}

void AdvancedSettingsWindow::audioDriverChange(int index) {
  if(index >= 0) config().system.audio = audioDriver->itemText(index).toUtf8().data();
}

void AdvancedSettingsWindow::inputDriverChange(int index) {
  if(index >= 0) config().system.input = inputDriver->itemText(index).toUtf8().data();
}

void AdvancedSettingsWindow::setRegionAuto() { SNES::config.region = SNES::System::Autodetect; }
void AdvancedSettingsWindow::setRegionNTSC() { SNES::config.region = SNES::System::NTSC; }
void AdvancedSettingsWindow::setRegionPAL()  { SNES::config.region = SNES::System::PAL; }

void AdvancedSettingsWindow::setPortSatellaview() { SNES::config.expansion_port = SNES::System::ExpansionBSX; }
void AdvancedSettingsWindow::setPortNone()        { SNES::config.expansion_port = SNES::System::ExpansionNone; }

void AdvancedSettingsWindow::pauseWithoutFocus()       { config().input.focusPolicy = Configuration::Input::FocusPolicyPauseEmulation; }
void AdvancedSettingsWindow::ignoreInputWithoutFocus() { config().input.focusPolicy = Configuration::Input::FocusPolicyIgnoreInput; }
void AdvancedSettingsWindow::allowInputWithoutFocus()  { config().input.focusPolicy = Configuration::Input::FocusPolicyAllowInput; }

void AdvancedSettingsWindow::toggleRewindEnable() {
  config().system.rewindEnabled = rewindEnable->isChecked();
  state.resetHistory();
}
