LoaderWindow::LoaderWindow() : QbWindow(config.geometry.loaderWindow) {
  setObjectName("loader-window");
  setMinimumWidth(520);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  grid = new QGridLayout;
  layout->addLayout(grid);

  baseLabel = new QLabel("Base cartridge:");
  grid->addWidget(baseLabel, 0, 0);

  baseFile = new QLineEdit;
  baseFile->setReadOnly(true);
  grid->addWidget(baseFile, 0, 1);

  baseBrowse = new QPushButton("Browse ...");
  grid->addWidget(baseBrowse, 0, 2);

  baseClear = new QPushButton("Clear");
  grid->addWidget(baseClear, 0, 3);

  slot1Label = new QLabel("Slot A cartridge:");
  grid->addWidget(slot1Label, 1, 0);

  slot1File = new QLineEdit;
  slot1File->setReadOnly(true);
  grid->addWidget(slot1File, 1, 1);

  slot1Browse = new QPushButton("Browse ...");
  grid->addWidget(slot1Browse, 1, 2);

  slot1Clear = new QPushButton("Clear");
  grid->addWidget(slot1Clear, 1, 3);

  slot2Label = new QLabel("Slot B cartridge:");
  grid->addWidget(slot2Label, 2, 0);

  slot2File = new QLineEdit;
  slot2File->setReadOnly(true);
  grid->addWidget(slot2File, 2, 1);

  slot2Browse = new QPushButton("Browse ...");
  grid->addWidget(slot2Browse, 2, 2);

  slot2Clear = new QPushButton("Clear");
  grid->addWidget(slot2Clear, 2, 3);

  load = new QPushButton("Load");
  grid->addWidget(load, 3, 2);

  cancel = new QPushButton("Cancel");
  grid->addWidget(cancel, 3, 3);

  connect(baseBrowse, SIGNAL(released()), this, SLOT(selectBaseCartridge()));
  connect(baseClear, SIGNAL(released()), this, SLOT(clearBaseCartridge()));
  connect(slot1Browse, SIGNAL(released()), this, SLOT(selectSlot1Cartridge()));
  connect(slot1Clear, SIGNAL(released()), this, SLOT(clearSlot1Cartridge()));
  connect(slot2Browse, SIGNAL(released()), this, SLOT(selectSlot2Cartridge()));
  connect(slot2Clear, SIGNAL(released()), this, SLOT(clearSlot2Cartridge()));
  connect(load, SIGNAL(released()), this, SLOT(onLoad()));
  connect(cancel, SIGNAL(released()), this, SLOT(close()));
}

void LoaderWindow::syncUi() {
  //only allow load when a base file is specified ...
  load->setEnabled(baseFile->text().length() > 0);
}

void LoaderWindow::loadBsxSlottedCartridge(const char *filebase, const char *fileSlot1) {
  hide();
  baseLabel->show(),  baseFile->show(),  baseBrowse->show(),  baseClear->show();
  slot1Label->show(), slot1File->show(), slot1Browse->show(), slot1Clear->show();
  slot2Label->hide(), slot2File->hide(), slot2Browse->hide(), slot2Clear->hide();

  slot1Label->setText("Slot cartridge:");

  baseFile->setText(filebase);
  slot1File->setText(fileSlot1);

  syncUi();
  mode = SNES::Cartridge::ModeBsxSlotted;
  showWindow("Load BS-X Slotted Cartridge");
}

void LoaderWindow::loadBsxCartridge(const char *fileBase, const char *fileSlot1) {
  hide();
  baseLabel->show(),  baseFile->show(),  baseBrowse->show(),  baseClear->show();
  slot1Label->show(), slot1File->show(), slot1Browse->show(), slot1Clear->show();
  slot2Label->hide(), slot2File->hide(), slot2Browse->hide(), slot2Clear->hide();

  slot1Label->setText("Slot cartridge:");

  baseFile->setText(fileBase);
  slot1File->setText(fileSlot1);

  syncUi();
  mode = SNES::Cartridge::ModeBsx;
  showWindow("Load BS-X Cartridge");
}

void LoaderWindow::loadSufamiTurboCartridge(const char *fileBase, const char *fileSlot1, const char *fileSlot2) {
  hide();
  baseLabel->show(),  baseFile->show(),  baseBrowse->show(),  baseClear->show();
  slot1Label->show(), slot1File->show(), slot1Browse->show(), slot1Clear->show();
  slot2Label->show(), slot2File->show(), slot2Browse->show(), slot2Clear->show();

  slot1Label->setText("Slot A cartridge:");
  slot2Label->setText("Slot B cartridge:");

  baseFile->setText(fileBase);
  slot1File->setText(fileSlot1);
  slot2File->setText(fileSlot2);

  syncUi();
  mode = SNES::Cartridge::ModeSufamiTurbo;
  showWindow("Load Sufami Turbo Cartridge");
}

void LoaderWindow::loadSuperGameBoyCartridge(const char *fileBase, const char *fileSlot1) {
  hide();
  baseLabel->show(),  baseFile->show(),  baseBrowse->show(),  baseClear->show();
  slot1Label->show(), slot1File->show(), slot1Browse->show(), slot1Clear->show();
  slot2Label->hide(), slot2File->hide(), slot2Browse->hide(), slot2Clear->hide();

  slot1Label->setText("Game Boy cartridge:");

  baseFile->setText(fileBase);
  slot1File->setText(fileSlot1);

  syncUi();
  mode = SNES::Cartridge::ModeSuperGameBoy;
  showWindow("Load Super Game Boy Cartridge");
}

void LoaderWindow::showWindow(const char *title) {
  setWindowTitle(title);
  show();
  shrink();
  load->setFocus();
}

void LoaderWindow::selectBaseCartridge(const char *filename) {
  baseFile->setText(utf8() << filename);
  syncUi();
}

void LoaderWindow::selectSlot1Cartridge(const char *filename) {
  slot1File->setText(utf8() << filename);
  syncUi();
}

void LoaderWindow::selectSlot2Cartridge(const char *filename) {
  slot2File->setText(utf8() << filename);
  syncUi();
}

void LoaderWindow::selectBaseCartridge() {
  diskBrowser->loadBaseCartridge();
}

void LoaderWindow::clearBaseCartridge() {
  baseFile->setText("");
  syncUi();
}

void LoaderWindow::selectSlot1Cartridge() {
  switch(mode) {
    case SNES::Cartridge::ModeBsx:
    case SNES::Cartridge::ModeBsxSlotted:
      diskBrowser->loadBsxCartridge();
      break;
    case SNES::Cartridge::ModeSufamiTurbo:
      diskBrowser->loadSufamiTurboCartridge1();
      break;
    case SNES::Cartridge::ModeSuperGameBoy:
      diskBrowser->loadSuperGameBoyCartridge();
      break;
  }
}

void LoaderWindow::clearSlot1Cartridge() {
  slot1File->setText("");
  syncUi();
}

void LoaderWindow::selectSlot2Cartridge() {
  diskBrowser->loadSufamiTurboCartridge2();
}

void LoaderWindow::clearSlot2Cartridge() {
  slot2File->setText("");
  syncUi();
}

void LoaderWindow::onLoad() {
  hide();
  string base  = baseFile->text().toUtf8().data();
  string slot1 = slot1File->text().toUtf8().data();
  string slot2 = slot2File->text().toUtf8().data();

  switch(mode) {
    case SNES::Cartridge::ModeBsxSlotted: {
      utility.loadCartridgeBsxSlotted(base, slot1);
    } break;

    case SNES::Cartridge::ModeBsx: {
      config.path.bsx = base;
      utility.loadCartridgeBsx(base, slot1);
    } break;

    case SNES::Cartridge::ModeSufamiTurbo: {
      config.path.st = base;
      utility.loadCartridgeSufamiTurbo(base, slot1, slot2);
    } break;

    case SNES::Cartridge::ModeSuperGameBoy: {
      config.path.sgb = base;
      utility.loadCartridgeSuperGameBoy(base, slot1);
    } break;
  }
}
