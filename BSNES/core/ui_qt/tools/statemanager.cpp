StateManagerWindow::StateManagerWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  title = new QLabel("Save State Manager");
  title->setProperty("class", "title");
  layout->addWidget(title);

  list = new QTreeWidget;
  list->setColumnCount(3);
  list->setHeaderLabels(QStringList() << "Slot" << "Last Updated" << "Description");
  list->setAllColumnsShowFocus(true);
  list->sortByColumn(0, Qt::AscendingOrder);
  list->setRootIsDecorated(false);
  list->setContextMenuPolicy(Qt::CustomContextMenu);
  layout->addWidget(list);
  layout->addSpacing(Style::WidgetSpacing);

  controlLayout = new QHBoxLayout;
  controlLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(controlLayout);
  layout->addSpacing(Style::WidgetSpacing);

  descLabel = new QLabel("Description:");
  controlLayout->addWidget(descLabel);

  descEdit = new QLineEdit;
  controlLayout->addWidget(descEdit);

  buttonLayout = new QHBoxLayout;
  buttonLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(buttonLayout);

  loadState = new QPushButton("Load");
  loadState->setToolTip("Load state from the currently selected slot");
  buttonLayout->addWidget(loadState);

  saveState = new QPushButton("Save");
  saveState->setToolTip("Save state to the currently selected slot");
  buttonLayout->addWidget(saveState);

  createState = new QPushButton("Create");
  createState->setToolTip("Save state to a new slot");
  buttonLayout->addWidget(createState);

  deleteState = new QPushButton("Delete");
  deleteState->setToolTip("Delete the currently selected slot");
  buttonLayout->addWidget(deleteState);

  reloadList();

  menu = new QMenu(list);
  loadStateItem = menu->addAction("&Load State From Selected Slot");
  saveStateItem = menu->addAction("&Save State To Selected Slot");
  createStateItem = menu->addAction("Save State To &New Slot");
  deleteStateItem = menu->addAction("Permanently &Delete Selected Slot");

  connect(list, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(popupMenu(const QPoint&)));
  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(listChanged()));
  connect(list, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(loadSelectedState()));
  connect(descEdit, SIGNAL(textEdited(const QString&)), this, SLOT(textEdited()));
  connect(loadState, SIGNAL(released()), this, SLOT(loadSelectedState()));
  connect(saveState, SIGNAL(released()), this, SLOT(saveSelectedState()));
  connect(createState, SIGNAL(released()), this, SLOT(createNewState()));
  connect(deleteState, SIGNAL(released()), this, SLOT(deleteSelectedState()));

  connect(loadStateItem, SIGNAL(triggered()), this, SLOT(loadSelectedState()));
  connect(saveStateItem, SIGNAL(triggered()), this, SLOT(saveSelectedState()));
  connect(createStateItem, SIGNAL(triggered()), this, SLOT(createNewState()));
  connect(deleteStateItem, SIGNAL(triggered()), this, SLOT(deleteSelectedState()));
}

void StateManagerWindow::syncUi() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  descEdit->setEnabled(items.count() > 0);
  if(descEdit->isEnabled() == false) descEdit->setText("");
  for(unsigned n = 0; n <= 1; n++) list->resizeColumnToContents(n);

  if(utility.saveStatesSupported() == false) {
    loadState->setEnabled(false);
    saveState->setEnabled(false);
    createState->setEnabled(false);
    deleteState->setEnabled(false);
  } else {
    loadState->setEnabled(items.count() == 1);
    saveState->setEnabled(items.count() == 1);
    createState->setEnabled(SNES::cartridge.loaded() && application.power);
    deleteState->setEnabled(items.count() == 1);
  }
}

void StateManagerWindow::popupMenu(const QPoint &point) {
  if(SNES::cartridge.loaded() == false) return;
  if(utility.saveStatesSupported() == false) return;
  QTreeWidgetItem *item = list->itemAt(point);
  if(!item && !application.power) return;

  if(item) list->setCurrentItem(item);
  loadStateItem->setVisible(item);
  saveStateItem->setVisible(item);
  createStateItem->setVisible(application.power);
  deleteStateItem->setVisible(item);
  menu->popup(QCursor::pos());
}

void StateManagerWindow::reloadList() {
  list->clear();
  list->setSortingEnabled(false);

  lstring state;
  utility.loadStateInfo(state);
  for(unsigned n = 0; n < state.size(); n++) {
    QTreeWidgetItem *item = new QTreeWidgetItem(list);
    lstring part;
    part.split("\t", state[n]);
    item->setData(0, Qt::UserRole, QVariant(n));
    item->setText(0, utf8() << part[0]);
    item->setText(1, utf8() << part[1]);
    item->setText(2, utf8() << part[2]);
  }

  list->setSortingEnabled(true);
  list->header()->setSortIndicatorShown(false);
  syncUi();
}

void StateManagerWindow::updateItem(QTreeWidgetItem *item) {
  unsigned n = item->data(0, Qt::UserRole).toUInt();
  lstring state, part;
  utility.loadStateInfo(state);
  part.split("\t", state[n]);
  item->setText(0, utf8() << part[0]);
  item->setText(1, utf8() << part[1]);
  item->setText(2, utf8() << part[2]);
}

void StateManagerWindow::listChanged() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    descEdit->setText(item->text(2));
  }

  syncUi();
}

void StateManagerWindow::textEdited() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    utility.setStateDescription(n, descEdit->text().toUtf8().data());
    updateItem(item);
  }
}

void StateManagerWindow::loadSelectedState() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    utility.loadState(n);
    toolsWindow->close();
  }
}

void StateManagerWindow::saveSelectedState() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    utility.saveState(n, descEdit->text().toUtf8().data());
    updateItem(item);
  }
}

void StateManagerWindow::createNewState() {
  utility.saveState(SNES::StateManager::SlotInvalid, "");
  reloadList();
}

void StateManagerWindow::deleteSelectedState() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    unsigned n = item->data(0, Qt::UserRole).toUInt();
    utility.deleteState(n);
    reloadList();
  }
}
