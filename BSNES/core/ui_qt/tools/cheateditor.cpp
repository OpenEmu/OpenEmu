CheatEditorWindow::CheatEditorWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  title = new QLabel("Cheat Code Editor");
  title->setProperty("class", "title");
  layout->addWidget(title);

  list = new QTreeWidget;
  list->setColumnCount(3);
  list->setHeaderLabels(QStringList() << "Slot" << "Code" << "Description");
  list->setAllColumnsShowFocus(true);
  list->sortByColumn(0, Qt::AscendingOrder);
  list->setRootIsDecorated(false);
  list->setContextMenuPolicy(Qt::CustomContextMenu);
  layout->addWidget(list);
  layout->addSpacing(Style::WidgetSpacing);

  controlLayout = new QGridLayout;
  controlLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(controlLayout);
  layout->addSpacing(Style::WidgetSpacing);

  descLabel = new QLabel("Description:");
  descLabel->setAlignment(Qt::AlignRight);
  controlLayout->addWidget(descLabel, 0, 0);

  descEdit = new QLineEdit;
  controlLayout->addWidget(descEdit, 0, 1);

  codeLabel = new QLabel("Code(s):");
  codeLabel->setAlignment(Qt::AlignRight);
  controlLayout->addWidget(codeLabel, 1, 0);

  codeEdit = new QLineEdit;
  controlLayout->addWidget(codeEdit, 1, 1);

  buttonLayout = new QHBoxLayout;
  buttonLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(buttonLayout);

  addCode = new QPushButton("Add Slot");
  buttonLayout->addWidget(addCode);

  deleteCode = new QPushButton("Delete Slot");
  buttonLayout->addWidget(deleteCode);

  reloadList();

  menu = new QMenu(list);
  deleteCodeItem = menu->addAction("&Delete Selected Code");
  addCodeItem = menu->addAction("&Add New Code");

  connect(list, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(popupMenu(const QPoint&)));
  connect(list, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(updateCodeStatus()));
  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(listChanged()));
  connect(list, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(toggleCodeStatus()));
  connect(descEdit, SIGNAL(textEdited(const QString&)), this, SLOT(textEdited()));
  connect(codeEdit, SIGNAL(textEdited(const QString&)), this, SLOT(textEdited()));
  connect(addCode, SIGNAL(released()), this, SLOT(addNewCode()));
  connect(deleteCode, SIGNAL(released()), this, SLOT(deleteSelectedCode()));

  connect(addCodeItem, SIGNAL(triggered()), this, SLOT(addNewCode()));
  connect(deleteCodeItem, SIGNAL(triggered()), this, SLOT(deleteSelectedCode()));
}

void CheatEditorWindow::syncUi() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  descEdit->setEnabled(items.count() > 0);
  codeEdit->setEnabled(items.count() > 0);
  if(descEdit->isEnabled() == false) descEdit->setText("");
  if(codeEdit->isEnabled() == false) codeEdit->setText("");
  for(unsigned n = 0; n <= 2; n++) list->resizeColumnToContents(n);

  bool badCode = false;
  string scode = codeEdit->text().toUtf8().data();
  if(scode.length() > 0) {
    SNES::Cheat::cheat_t code;
    if(SNES::cheat.decode(scode, code) == false) badCode = true;
  }

  if(badCode == false) {
    codeEdit->setStyleSheet("");
  } else {
    codeEdit->setStyleSheet("background: #ffc0c0;");
  }

  addCode->setEnabled(SNES::cartridge.loaded() == true);
  deleteCode->setEnabled(list->selectedItems().count() == 1);
}

void CheatEditorWindow::updateItem(QTreeWidgetItem *item) {
  unsigned n = item->data(0, Qt::UserRole).toUInt();
  SNES::Cheat::cheat_t code, temp;
  SNES::cheat.get(n, code);

  if(SNES::cheat.decode(code.code, temp) == false) {
    item->setForeground(1, QBrush(QColor(224, 0, 0)));
  } else {
    lstring part;
    part.split("+", code.code);
    if(part.size() > 1) {
      item->setForeground(1, QBrush(QColor(0, 128, 0)));
    } else {
      item->setForeground(1, QBrush(QColor(0, 0, 128)));
    }
  }

  string scode = code.code;
  scode.replace(" ", "");
  lstring lcode;
  lcode.split("+", scode);
  if(lcode.size() > 1) lcode[0] << "+" << lcode.size() - 1;

  item->setCheckState(1, code.enabled ? Qt::Checked : Qt::Unchecked);
  item->setText(1, utf8() << lcode[0]);
  item->setText(2, utf8() << code.desc);
}

void CheatEditorWindow::popupMenu(const QPoint &point) {
  if(SNES::cartridge.loaded() == false) return;

  QTreeWidgetItem *item = list->itemAt(point);
  if(item) list->setCurrentItem(item);
  deleteCodeItem->setVisible(item);
  menu->popup(QCursor::pos());
}

void CheatEditorWindow::reloadList() {
  list->clear();
  list->setSortingEnabled(false);

  if(SNES::cartridge.loaded()) {
    for(unsigned n = 0; n < SNES::cheat.count(); n++) {
      QTreeWidgetItem *item = new QTreeWidgetItem(list);
      item->setData(0, Qt::UserRole, QVariant(n));
      char slot[16];
      sprintf(slot, "%3u", n + 1);
      item->setText(0, utf8() << slot);
      updateItem(item);
    }
  }

  list->setSortingEnabled(true);
  list->header()->setSortIndicatorShown(false);
  syncUi();
}

void CheatEditorWindow::listChanged() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();

    SNES::Cheat::cheat_t code;
    SNES::cheat.get(n, code);

    descEdit->setText(utf8() << code.desc);
    codeEdit->setText(utf8() << code.code);
  }

  syncUi();
}

void CheatEditorWindow::textEdited() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();

    string scode = codeEdit->text().toUtf8().data();
    string sdesc = descEdit->text().toUtf8().data();

    SNES::Cheat::cheat_t code;
    SNES::cheat.get(n, code);
    SNES::cheat.edit(n, code.enabled, scode, sdesc);

    updateItem(item);
    syncUi();
  }
}

//user ticked checkbox, set code enable state to checkbox state
void CheatEditorWindow::updateCodeStatus() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    item->checkState(1) == Qt::Checked ? SNES::cheat.enable(n) : SNES::cheat.disable(n);
  }
}

//user double-clicked line item, toggle current code enable state
void CheatEditorWindow::toggleCodeStatus() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    SNES::cheat.enabled(n) == false ? SNES::cheat.enable(n) : SNES::cheat.disable(n);
    item->setCheckState(1, SNES::cheat.enabled(n) ? Qt::Checked : Qt::Unchecked);
  }
}

void CheatEditorWindow::addNewCode() {
  SNES::cheat.add(false, "", "");
  reloadList();
}

void CheatEditorWindow::deleteSelectedCode() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    SNES::cheat.remove(n);
    reloadList();
  }
}
