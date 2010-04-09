InputSettingsWindow::InputSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  title = new QLabel("Input Configuration Editor");
  title->setProperty("class", "title");
  layout->addWidget(title);

  selection = new QHBoxLayout; {
    port = new QComboBox;
    port->addItem("Controller Port 1");
    port->addItem("Controller Port 2");
    port->addItem("User Interface");
    selection->addWidget(port);

    device = new QComboBox;
    selection->addWidget(device);
  }
  selection->setSpacing(Style::WidgetSpacing);
  layout->addLayout(selection);
  layout->addSpacing(Style::WidgetSpacing);

  list = new QTreeWidget;
  list->setColumnCount(3);
  list->setHeaderLabels(QStringList() << "Hidden" << "Name" << "Assignment");
  list->setAllColumnsShowFocus(true);
  list->setRootIsDecorated(false);
  list->hideColumn(0);  //used for default sorting
  layout->addWidget(list);
  layout->addSpacing(Style::WidgetSpacing);

  controls = new QHBoxLayout; {
    assign = new QPushButton("Assign ...");
    controls->addWidget(assign);

    assignAll = new QPushButton("Assign All ...");
    controls->addWidget(assignAll);

    unassign = new QPushButton("Unassign");
    controls->addWidget(unassign);
  }
  controls->setSpacing(Style::WidgetSpacing);
  layout->addLayout(controls);

  connect(port, SIGNAL(currentIndexChanged(int)), this, SLOT(portChanged()));
  connect(device, SIGNAL(currentIndexChanged(int)), this, SLOT(reloadList()));
  connect(list, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(assignKey()));
  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(listChanged()));
  connect(assign, SIGNAL(released()), this, SLOT(assignKey()));
  connect(assignAll, SIGNAL(released()), this, SLOT(assignAllKeys()));
  connect(unassign, SIGNAL(released()), this, SLOT(unassignKey()));

  portChanged();
}

void InputSettingsWindow::syncUi() {
  QList<QTreeWidgetItem*> itemList = list->selectedItems();
  assign->setEnabled(itemList.count() == 1);
  //allow rapid assign for controllers from both ports, but not for UI shortcuts
  assignAll->setEnabled(port->currentIndex() < 2);
  unassign->setEnabled(itemList.count() == 1);
}

//when port combobox item is changed, device list needs to be repopulated
void InputSettingsWindow::portChanged() {
  disconnect(device, SIGNAL(currentIndexChanged(int)), this, SLOT(reloadList()));

  device->clear();
  deviceItem.reset();

  int index = port->currentIndex();
  if(index < 2) {
    //this is a controller port
    for(unsigned i = 0; i < inputPool.size(); i++) {
      //only add devices for selected port
      if(inputPool[i]->port == index) {
        device->addItem(inputPool[i]->name);
        deviceItem.add(inputPool[i]);
      }
    }
  } else {
    //user interface controls
    for(unsigned i = 0; i < inputUiPool.size(); i++) {
      device->addItem(inputUiPool[i]->name);
      deviceItem.add(inputUiPool[i]);
    }
  }

  reloadList();
  connect(device, SIGNAL(currentIndexChanged(int)), this, SLOT(reloadList()));
}

//when device combobox item is changed, object list needs to be repopulated
void InputSettingsWindow::reloadList() {
  list->clear();
  list->setSortingEnabled(false);
  listItem.reset();

  int index = device->currentIndex();
  if(index < deviceItem.size()) {
    InputGroup &group = *deviceItem[index];
    for(unsigned i = 0; i < group.size(); i++) {
      QTreeWidgetItem *item = new QTreeWidgetItem(list);
      item->setText(0, utf8() << (int)(1000000 + i));
      item->setText(1, group[i]->name);
      item->setText(2, (const char*)group[i]->id);
      listItem.add(item);
    }
  }

  list->setSortingEnabled(true);
  list->sortByColumn(0, Qt::AscendingOrder);  //set default sorting on list change, overriding user setting
  list->resizeColumnToContents(1);  //shrink name column
  syncUi();
}

void InputSettingsWindow::listChanged() {
  syncUi();
}

//InputCaptureWindow calls this after a successful key assignment change:
//need to update list of values to show new key assignment value.
void InputSettingsWindow::updateList() {
  int index = device->currentIndex();
  if(index < deviceItem.size()) {
    InputGroup &group = *deviceItem[index];

    for(unsigned i = 0; i < listItem.size(); i++) {
      listItem[i]->setText(2, (const char*)group[i]->id);
    }
  }
}

void InputSettingsWindow::assignKey() {
  int index = device->currentIndex();
  if(index < deviceItem.size()) {
    InputGroup &group = *deviceItem[index];

    QTreeWidgetItem *item = list->currentItem();
    if(item && item->isSelected()) {
      signed i = listItem.find(item);
      if(i >= 0) inputCaptureWindow->activate(group[i]);
    }
  }
}

void InputSettingsWindow::assignAllKeys() {
  int index = port->currentIndex();
  if(index < 2) {
    index = device->currentIndex();
    if(index < deviceItem.size()) {
      inputCaptureWindow->activate(deviceItem[index]);
    }
  }
}

void InputSettingsWindow::unassignKey() {
  int index = device->currentIndex();
  if(index < deviceItem.size()) {
    InputGroup &group = *deviceItem[index];

    QTreeWidgetItem *item = list->currentItem();
    if(item && item->isSelected()) {
      signed i = listItem.find(item);
      if(i >= 0) {
        group[i]->id = "none";
        inputManager.bind();  //update key bindings to reflect object ID change above
        item->setText(2, (const char*)group[i]->id);
      }
    }
  }
}
