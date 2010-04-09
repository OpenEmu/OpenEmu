CheatFinderWindow::CheatFinderWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  title = new QLabel("Cheat Code Finder");
  title->setProperty("class", "title");
  layout->addWidget(title);

  list = new QTreeWidget;
  list->setColumnCount(3);
  list->setHeaderLabels(QStringList() << "Address" << "Current Value" << "Previous Value");
  list->setAllColumnsShowFocus(true);
  list->sortByColumn(0, Qt::AscendingOrder);
  list->setRootIsDecorated(false);
  layout->addWidget(list);
  layout->addSpacing(Style::WidgetSpacing);

  controlLayout = new QGridLayout;
  layout->addLayout(controlLayout);

  sizeLabel = new QLabel("Data size: ");
  controlLayout->addWidget(sizeLabel, 0, 0);

  sizeGroup = new QButtonGroup(this);

  size8bit = new QRadioButton("8-bit");
  size8bit->setChecked(true);
  sizeGroup->addButton(size8bit);
  controlLayout->addWidget(size8bit, 0, 1);

  size16bit = new QRadioButton("16-bit");
  sizeGroup->addButton(size16bit);
  controlLayout->addWidget(size16bit, 0, 2);

  size24bit = new QRadioButton("24-bit");
  sizeGroup->addButton(size24bit);
  controlLayout->addWidget(size24bit, 0, 3);

  size32bit = new QRadioButton("32-bit");
  sizeGroup->addButton(size32bit);
  controlLayout->addWidget(size32bit, 0, 4);

  compareLabel = new QLabel("Compare mode: ");
  controlLayout->addWidget(compareLabel, 1, 0);

  compareGroup = new QButtonGroup(this);

  compareEqual = new QRadioButton("Equal to");
  compareEqual->setChecked(true);
  compareGroup->addButton(compareEqual);
  controlLayout->addWidget(compareEqual, 1, 1);

  compareNotEqual = new QRadioButton("Not equal to");
  compareGroup->addButton(compareNotEqual);
  controlLayout->addWidget(compareNotEqual, 1, 2);

  compareLessThan = new QRadioButton("Less than");
  compareGroup->addButton(compareLessThan);
  controlLayout->addWidget(compareLessThan, 1, 3);

  compareGreaterThan = new QRadioButton("Greater than");
  compareGroup->addButton(compareGreaterThan);
  controlLayout->addWidget(compareGreaterThan, 1, 4);

  valueLabel = new QLabel("Search value: ");
  controlLayout->addWidget(valueLabel, 2, 0);

  actionLayout = new QHBoxLayout;
  actionLayout->setSpacing(Style::WidgetSpacing);
  controlLayout->addLayout(actionLayout, 2, 1, 1, 4);

  valueEdit = new QLineEdit;
  actionLayout->addWidget(valueEdit);

  searchButton = new QPushButton("Search");
  actionLayout->addWidget(searchButton);

  resetButton = new QPushButton("Reset");
  actionLayout->addWidget(resetButton);

  connect(valueEdit, SIGNAL(returnPressed()), this, SLOT(searchMemory()));
  connect(searchButton, SIGNAL(released()), this, SLOT(searchMemory()));
  connect(resetButton, SIGNAL(released()), this, SLOT(resetSearch()));
  synchronize();
}

void CheatFinderWindow::synchronize() {
  if(SNES::cartridge.loaded() == false || application.power == false) {
    list->clear();
    for(unsigned n = 0; n < 3; n++) list->resizeColumnToContents(n);
    valueEdit->setEnabled(false);
    valueEdit->setText("");
    searchButton->setEnabled(false);
    resetButton->setEnabled(false);
  } else {
    valueEdit->setEnabled(true);
    searchButton->setEnabled(true);
    resetButton->setEnabled(true);
  }
}

void CheatFinderWindow::refreshList() {
  list->clear();
  list->setSortingEnabled(false);

  unsigned size = 0;
  if(size16bit->isChecked()) size = 1;
  if(size24bit->isChecked()) size = 2;
  if(size32bit->isChecked()) size = 3;

  for(unsigned i = 0; i < addrList.size() && i < 256; i++) {
    QTreeWidgetItem *item = new QTreeWidgetItem(list);

    unsigned addr = addrList[i];
    unsigned data = read(addr, size);
    unsigned prev = dataList[i];

    char temp[256];

    sprintf(temp, "%.6x", 0x7e0000 + addr);
    item->setText(0, temp);

    sprintf(temp, "%u (0x%x)", data, data);
    item->setText(1, temp);

    sprintf(temp, "%u (0x%x)", prev, prev);
    item->setText(2, temp);
  }

  list->setSortingEnabled(true);
  list->header()->setSortIndicatorShown(false);
  for(unsigned n = 0; n < 3; n++) list->resizeColumnToContents(n);
}

void CheatFinderWindow::searchMemory() {
  unsigned size = 0;
  if(size16bit->isChecked()) size = 1;
  if(size24bit->isChecked()) size = 2;
  if(size32bit->isChecked()) size = 3;

  unsigned data;
  string text = valueEdit->text().toUtf8().constData();

  //auto-detect input data type
  if(strbegin(text, "0x")) data = strhex(text + 2);
  else if(strbegin(text, "-")) data = strsigned(text);
  else data = strunsigned(text);

  if(addrList.size() == 0) {
    //search for the first time: enqueue all possible values so they are all searched
    for(unsigned addr = 0; addr < SNES::memory::wram.size(); addr++) {
      addrList.add(addr);
      dataList.add(read(addr, size));
    }
  }

  array<unsigned> newAddrList, newDataList, oldDataList;

  for(unsigned i = 0; i < addrList.size(); i++) {
    unsigned thisAddr = addrList[i];
    unsigned thisData = read(thisAddr, size);

    if((compareEqual->isChecked()       && thisData == data)
    || (compareNotEqual->isChecked()    && thisData != data)
    || (compareLessThan->isChecked()    && thisData <  data)
    || (compareGreaterThan->isChecked() && thisData >  data)
    ) {
      newAddrList.add(thisAddr);
      newDataList.add(thisData);
      oldDataList.add(dataList[i]);
    }
  }

  //first refresh the list with the old data values (for the previous value column)
  addrList = newAddrList;
  dataList = oldDataList;
  refreshList();

  //and now update the list with the new data values (for the next search)
  dataList = newDataList;
}

void CheatFinderWindow::resetSearch() {
  addrList.reset();
  dataList.reset();
  refreshList();
}

//size = 0 (8-bit), 1 (16-bit), 2 (24-bit), 3 (32-bit)
unsigned CheatFinderWindow::read(unsigned addr, unsigned size) {
  unsigned data = 0;

  for(unsigned n = 0; n <= size; n++) {
    if(addr + n >= SNES::memory::wram.size()) break;
    data |= SNES::memory::wram[addr + n] << (n << 3);
  }

  return data;
}
