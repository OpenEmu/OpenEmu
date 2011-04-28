#include "statemanager.moc"
StateManagerWindow *stateManagerWindow;

StateManagerWindow::StateManagerWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  list = new QTreeWidget;
  list->setColumnCount(2);
  list->setHeaderLabels(QStringList() << "Slot" << "Description");
  list->setAllColumnsShowFocus(true);
  list->sortByColumn(0, Qt::AscendingOrder);
  list->setRootIsDecorated(false);
  list->resizeColumnToContents(0);
  layout->addWidget(list);

  infoLayout = new QHBoxLayout;
  layout->addLayout(infoLayout);

  descriptionLabel = new QLabel("Description:");
  infoLayout->addWidget(descriptionLabel);

  descriptionText = new QLineEdit;
  infoLayout->addWidget(descriptionText);

  controlLayout = new QHBoxLayout;
  layout->addLayout(controlLayout);

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  controlLayout->addWidget(spacer);

  loadButton = new QPushButton("Load");
  controlLayout->addWidget(loadButton);

  saveButton = new QPushButton("Save");
  controlLayout->addWidget(saveButton);

  eraseButton = new QPushButton("Erase");
  controlLayout->addWidget(eraseButton);

  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(synchronize()));
  connect(list, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(loadAction()));
  connect(descriptionText, SIGNAL(textEdited(const QString&)), this, SLOT(writeDescription()));
  connect(loadButton, SIGNAL(released()), this, SLOT(loadAction()));
  connect(saveButton, SIGNAL(released()), this, SLOT(saveAction()));
  connect(eraseButton, SIGNAL(released()), this, SLOT(eraseAction()));

  synchronize();
}

void StateManagerWindow::reload() {
  list->clear();
  list->setSortingEnabled(false);

  if(SNES::cartridge.loaded() && cartridge.saveStatesSupported()) {
    for(unsigned n = 0; n < StateCount; n++) {
      QTreeWidgetItem *item = new QTreeWidgetItem(list);
      item->setData(0, Qt::UserRole, QVariant(n));
      char slot[16];
      sprintf(slot, "%2u", n + 1);
      item->setText(0, slot);
    }
    update();
  }

  list->setSortingEnabled(true);
  list->header()->setSortIndicatorShown(false);
  synchronize();
}

void StateManagerWindow::update() {
  //iterate all list items
  QList<QTreeWidgetItem*> items = list->findItems("", Qt::MatchContains);
  for(unsigned i = 0; i < items.count(); i++) {
    QTreeWidgetItem *item = items[i];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    if(isStateValid(n) == false) {
      item->setForeground(0, QBrush(QColor(128, 128, 128)));
      item->setForeground(1, QBrush(QColor(128, 128, 128)));
      item->setText(1, "Empty");
    } else {
      item->setForeground(0, QBrush(QColor(0, 0, 0)));
      item->setForeground(1, QBrush(QColor(0, 0, 0)));
      item->setText(1, getStateDescription(n));
    }
  }

  for(unsigned n = 0; n <= 1; n++) list->resizeColumnToContents(n);
}

void StateManagerWindow::synchronize() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();

    if(isStateValid(n)) {
      descriptionText->setText(getStateDescription(n));
      descriptionText->setEnabled(true);
      loadButton->setEnabled(true);
      eraseButton->setEnabled(true);
    } else {
      descriptionText->setText("");
      descriptionText->setEnabled(false);
      loadButton->setEnabled(false);
      eraseButton->setEnabled(false);
    }
    saveButton->setEnabled(true);
  } else {
    descriptionText->setText("");
    descriptionText->setEnabled(false);
    loadButton->setEnabled(false);
    saveButton->setEnabled(false);
    eraseButton->setEnabled(false);
  }
}

void StateManagerWindow::writeDescription() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    string description = descriptionText->text().toUtf8().constData();
    setStateDescription(n, description);
    update();
  }
}

void StateManagerWindow::loadAction() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    loadState(n);
  }
}

void StateManagerWindow::saveAction() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    saveState(n);
    writeDescription();
    synchronize();
    descriptionText->setFocus();
  }
}

void StateManagerWindow::eraseAction() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    eraseState(n);
    update();
    synchronize();
  }
}

string StateManagerWindow::filename() const {
  string name = filepath(nall::basename(cartridge.fileName), config().path.state);
  name << ".bsa";
  return name;
}

bool StateManagerWindow::isStateValid(unsigned slot) {
  if(SNES::cartridge.loaded() == false) return false;
  file fp;
  if(fp.open(filename(), file::mode_read) == false) return false;
  if(fp.size() < (slot + 1) * SNES::system.serialize_size()) { fp.close(); return false; }
  fp.seek(slot * SNES::system.serialize_size());
  uint32_t signature = fp.readl(4);
  if(signature == 0) { fp.close(); return false; }
  uint32_t version = fp.readl(4);
  if(version != bsnesSerializerVersion) { fp.close(); return false; }
  fp.close();
  return true;
}

string StateManagerWindow::getStateDescription(unsigned slot) {
  if(isStateValid(slot) == false) return "";
  file fp;
  fp.open(filename(), file::mode_read);
  char description[513];
  fp.seek(slot * SNES::system.serialize_size() + 12);
  fp.read((uint8_t*)description, 512);
  fp.close();
  description[512] = 0;
  return description;
}

void StateManagerWindow::setStateDescription(unsigned slot, const string &text) {
  if(isStateValid(slot) == false) return;
  file fp;
  fp.open(filename(), file::mode_readwrite);
  char description[512];
  memset(&description, 0, sizeof description);
  strncpy(description, text, 512);
  fp.seek(slot * SNES::system.serialize_size() + 12);
  fp.write((uint8_t*)description, 512);
  fp.close();
}

void StateManagerWindow::loadState(unsigned slot) {
  if(isStateValid(slot) == false) return;
  file fp;
  fp.open(filename(), file::mode_read);
  fp.seek(slot * SNES::system.serialize_size());
  unsigned size = SNES::system.serialize_size();
  uint8_t *data = new uint8_t[size];
  fp.read(data, size);
  fp.close();

  serializer state(data, size);
  delete[] data;

  if(SNES::system.unserialize(state) == true) {
    //toolsWindow->close();
  }
}

void StateManagerWindow::saveState(unsigned slot) {
  file fp;
  if(file::exists(filename()) == false) {
    //try and create the file, bail out on failure (eg read-only device)
    if(fp.open(filename(), file::mode_write) == false) return;
    fp.close();
  }

  SNES::system.runtosave();
  serializer state = SNES::system.serialize();

  fp.open(filename(), file::mode_readwrite);

  //user may save to slot #2 when slot #1 is empty; pad file to current slot if needed
  unsigned stateOffset = SNES::system.serialize_size() * slot;
  fp.seek(fp.size());
  while(fp.size() < stateOffset) fp.write(0x00);

  fp.seek(stateOffset);
  fp.write(state.data(), state.size());
  fp.close();
}

void StateManagerWindow::eraseState(unsigned slot) {
  if(isStateValid(slot) == false) return;
  file fp;
  fp.open(filename(), file::mode_readwrite);
  unsigned size = SNES::system.serialize_size();
  fp.seek(slot * size);
  for(unsigned i = 0; i < size; i++) fp.write(0x00);
  fp.close();

  //shrink state archive as much as possible:
  //eg if only slot #2 and slot #31 were valid, but slot #31 was erased,
  //file can be resized to only hold blank slot #1 + valid slot #2
  signed lastValidState = -1;
  for(signed i = StateCount - 1; i >= 0; i--) {
    if(isStateValid(i)) {
      lastValidState = i;
      break;
    }
  }

  if(lastValidState == -1) {
    //no states used, remove empty file
    unlink(filename());
  } else {
    unsigned neededFileSize = (lastValidState + 1) * SNES::system.serialize_size();
    file fp;
    if(fp.open(filename(), file::mode_readwrite)) {
      if(fp.size() > neededFileSize) fp.truncate(neededFileSize);
      fp.close();
    }
  }
}
