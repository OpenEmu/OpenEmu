#include "cheateditor.moc"
CheatEditorWindow *cheatEditorWindow;

CheatEditorWindow::CheatEditorWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  list = new QTreeWidget;
  list->setColumnCount(3);
  list->setHeaderLabels(QStringList() << "Slot" << "Code" << "Description");
  list->setColumnWidth(1, list->fontMetrics().width("  89AB-CDEF+...  "));
  list->setAllColumnsShowFocus(true);
  list->sortByColumn(0, Qt::AscendingOrder);
  list->setRootIsDecorated(false);
  list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  list->resizeColumnToContents(0);
  layout->addWidget(list);

  gridLayout = new QGridLayout;
  layout->addLayout(gridLayout);

  codeLabel = new QLabel("Code(s):");
  codeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  gridLayout->addWidget(codeLabel, 0, 0);

  codeEdit = new QLineEdit;
  gridLayout->addWidget(codeEdit, 0, 1);

  descLabel = new QLabel("Description:");
  descLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  gridLayout->addWidget(descLabel, 1, 0);

  descEdit = new QLineEdit;
  gridLayout->addWidget(descEdit, 1, 1);

  controlLayout = new QHBoxLayout;
  controlLayout->setAlignment(Qt::AlignRight);
  layout->addLayout(controlLayout);

  clearButton = new QPushButton("Clear Selected");
  controlLayout->addWidget(clearButton);

  synchronize();

  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(listChanged()));
  connect(list, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(bind()));
  connect(codeEdit, SIGNAL(textEdited(const QString&)), this, SLOT(codeEdited()));
  connect(descEdit, SIGNAL(textEdited(const QString&)), this, SLOT(descEdited()));
  connect(clearButton, SIGNAL(released()), this, SLOT(clearSelected()));
}

void CheatEditorWindow::synchronize() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() == 1) {
    descEdit->setEnabled(true);
    codeEdit->setEnabled(true);
  } else {
    descEdit->setText("");
    codeEdit->setText("");
    descEdit->setEnabled(false);
    codeEdit->setEnabled(false);
  }
  clearButton->setEnabled(items.count() > 0);
}

void CheatEditorWindow::load(const char *filename) {
  list->clear();
  list->setSortingEnabled(false);
  SNES::cheat.reset();

  string data;
  lstring line;

  if(data.readfile(filename)) {
    data.replace("\r", "");
    line.split("\n", data);
  }

  for(unsigned i = 0; i < 128; i++) {
    lstring part;
    if(line.size() > i) part.qsplit(",", line[i]);
    for(unsigned n = 0; n <= 2; n++) trim(part[n], " ");
    trim(part[2], "\"");
    part[2].replace("\\q", "\"");

    QTreeWidgetItem *item = new QTreeWidgetItem(list);
    item->setData(0, Qt::UserRole, QVariant(i));
    item->setText(0, string::printf("%3u", i + 1));
    item->setCheckState(0, part[0] == "enabled" ? Qt::Checked : Qt::Unchecked);
    item->setText(1, part[1]);
    item->setText(2, part[2]);
  }

  list->resizeColumnToContents(0);
  list->setSortingEnabled(true);
  list->header()->setSortIndicatorShown(false);

  bind();
  update();
}

void CheatEditorWindow::save(const char *filename) {
  bool empty = true;
  string data[128];

  QList<QTreeWidgetItem*> items = list->findItems("", Qt::MatchContains);
  for(unsigned i = 0; i < items.count(); i++) {
    QTreeWidgetItem *item = items[i];
    unsigned index = item->data(0, Qt::UserRole).toUInt();
    string code = item->text(1).toUtf8().constData();
    string desc = item->text(2).toUtf8().constData();
    desc.replace("\"", "\\q");
    if((code != "") || (desc != "")) empty = false;

    data[index] << (item->checkState(0) == Qt::Checked ? "enabled," : "disabled,");
    data[index] << code << ",";
    data[index] << "\"" << desc << "\"\r\n";
  }

  if(empty == true) {
    unlink(filename);
  } else {
    file fp;
    if(fp.open(filename, file::mode_write)) {
      for(unsigned i = 0; i < 128; i++) fp.print(data[i]);
      fp.close();
    }
  }

  list->clear();
  SNES::cheat.reset();
  SNES::cheat.synchronize();
}

void CheatEditorWindow::update() {
  QList<QTreeWidgetItem*> items = list->findItems("", Qt::MatchContains);
  for(unsigned i = 0; i < items.count(); i++) {
    QTreeWidgetItem *item = items[i];
    string code = item->text(1).toUtf8().constData();
    string desc = item->text(2).toUtf8().constData();
    if((code != "") || (desc != "")) {
      item->setForeground(0, QBrush(QColor(0, 0, 0)));
    } else {
      //highlight empty slots in gray
      item->setForeground(0, QBrush(QColor(128, 128, 128)));
    }
    unsigned index = item->data(0, Qt::UserRole).toUInt();
    if(SNES::cheat[index].addr.size() > 0) {
      item->setForeground(1, QBrush(QColor(0, 0, 0)));
    } else {
      //highlight invalid codes in red
      //(this will also highlight empty codes, but as there is no text, it's not an issue)
      item->setForeground(1, QBrush(QColor(255, 0, 0)));
    }
  }
}

void CheatEditorWindow::bind() {
  QList<QTreeWidgetItem*> items = list->findItems("", Qt::MatchContains);
  for(unsigned i = 0; i < items.count(); i++) {
    QTreeWidgetItem *item = items[i];
    unsigned index = item->data(0, Qt::UserRole).toUInt();
    SNES::cheat[index] = item->text(1).toUtf8().constData();
    SNES::cheat[index].enabled = item->checkState(0) == Qt::Checked;
  }
  SNES::cheat.synchronize();
}

void CheatEditorWindow::listChanged() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() > 0) {
    QTreeWidgetItem *item = items[0];
    codeEdit->setText(item->text(1));
    descEdit->setText(item->text(2));
  }
  synchronize();
}

void CheatEditorWindow::codeEdited() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() == 1) {
    QTreeWidgetItem *item = items[0];
    item->setText(1, codeEdit->text());
  }
  bind();
  update();
}

void CheatEditorWindow::descEdited() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  if(items.count() == 1) {
    QTreeWidgetItem *item = items[0];
    item->setText(2, descEdit->text());
  }
  update();
}

void CheatEditorWindow::clearSelected() {
  QList<QTreeWidgetItem*> items = list->selectedItems();
  for(unsigned i = 0; i < items.count(); i++) {
    QTreeWidgetItem *item = items[i];
    item->setText(1, "");
    item->setText(2, "");
  }
  codeEdit->setText("");
  descEdit->setText("");
  bind();
  update();
}
