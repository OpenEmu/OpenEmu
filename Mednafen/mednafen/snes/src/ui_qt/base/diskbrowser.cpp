#include "diskbrowser.moc"
FolderCreator *folderCreator;
DiskBrowser *diskBrowser;

//=============
//FolderCreator
//=============

FolderCreator::FolderCreator() : QbWindow(config().geometry.folderCreator) {
  setObjectName("folder-creator");
  setWindowTitle("Create New Folder");

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  label = new QLabel("Folder name:");
  layout->addWidget(label);

  name = new QLineEdit;
  layout->addWidget(name);

  controlLayout = new QHBoxLayout;
  controlLayout->setAlignment(Qt::AlignRight);
  layout->addLayout(controlLayout);

  ok = new QPushButton("Ok");
  controlLayout->addWidget(ok);

  cancel = new QPushButton("Cancel");
  controlLayout->addWidget(cancel);

  connect(name, SIGNAL(returnPressed()), this, SLOT(createFolder()));
  connect(ok, SIGNAL(released()), this, SLOT(createFolder()));
  connect(cancel, SIGNAL(released()), this, SLOT(close()));
}

void FolderCreator::show() {
  name->setText("");
  QbWindow::show();
  name->setFocus();
}

void FolderCreator::createFolder() {
  if(name->text().length() == 0) {
    QMessageBox::warning(0, "Create New Folder", "<b>Note:</b> you must enter a folder name.");
  } else {
    string folderName = string()
    << diskBrowser->model->rootPath().toUtf8().constData()
    << "/"
    << name->text().toUtf8().constData();

    if(mkdir(folderName) == 0) {
      hide();
    } else {
      QMessageBox::warning(0, "Create new Folder", "<b>Error:</b> failed to create folder. Please ensure only valid characters were used in the folder name.");
    }
  }
}

//===============
//DiskBrowserView
//===============

void DiskBrowserView::keyPressEvent(QKeyEvent *event) {
  //enhance consistency: OS X by default doesn't activate items for these keypresses
  if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    emit activated(currentIndex());
    return;
  }

  //simulate popular file managers; backspace = go up one directory
  if(event->key() == Qt::Key_Backspace) {
    emit cdUp();
    return;
  }

  //fallback: unrecognized keypresses get handled by the widget itself
  QTreeView::keyPressEvent(event);
}

void DiskBrowserView::currentChanged(const QModelIndex &current, const QModelIndex &previous) {
  QAbstractItemView::currentChanged(current, previous);
  emit changed(current);
}

//================
//DiskBrowserImage
//================

void DiskBrowserImage::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  if(name != "") {
    QImage image(QString::fromUtf8(name));
    painter.drawImage(0, 0, image);
  }
}

//===========
//DiskBrowser
//===========

void DiskBrowser::inputEvent(uint16_t scancode) {
  if(!isActiveWindow() || isMinimized()) return;

  //provide very simple support for controlling the window via gamepads
  if(Joypad::isAnyHat(scancode)) {
    int16_t state = mapper().state(scancode);

    if(state == Joypad::HatUp) {
      QKeyEvent event((QEvent::Type)6, Qt::Key_Up, Qt::NoModifier);
      view->keyPressEvent(&event);
    }

    if(state == Joypad::HatDown) {
      QKeyEvent event((QEvent::Type)6, Qt::Key_Down, Qt::NoModifier);
      view->keyPressEvent(&event);
    }

    if(state == Joypad::HatLeft) {
      QKeyEvent event((QEvent::Type)6, Qt::Key_Backspace, Qt::NoModifier);
      view->keyPressEvent(&event);
    }

    if(state == Joypad::HatRight) {
      QKeyEvent event((QEvent::Type)6, Qt::Key_Return, Qt::NoModifier);
      view->keyPressEvent(&event);
    }
  }
}

void DiskBrowser::chooseFolder(const function<void (string)> &callback_, string &currentPath_, const char *title) {
  callback = callback_;
  currentPath = &currentPath_;
  browseMode = Folder;

  hide();
  group->hide();
  ok->setText("Choose");
  setWindowTitle(string() << title);
  setPath(*currentPath);
  setNameFilters("Folders ()");
  show();
}

void DiskBrowser::chooseFile(const function<void (string)> &callback_, string &currentPath_, const char *title) {
  callback = callback_;
  currentPath = &currentPath_;
  browseMode = File;

  hide();
  group->hide();
  ok->setText("Choose");
  setWindowTitle(string() << title);
  setPath(*currentPath);
  setNameFilters("All Files (*)");
  show();
}

void DiskBrowser::loadCartridge() {
  currentPath = &config().path.current.cartridge;
  browseMode = Cartridge;

  hide();
  group->setVisible(config().diskBrowser.showPanel);
  ok->setText("Load");
  setWindowTitle("Load Cartridge");
  setPath(config().path.rom == "" ? *currentPath : config().path.rom);
  setNameFilters(string()
  << "SNES cartridges (*.sfc" << reader.extensionList << reader.compressionList << ");;"
  << "BS-X cartridges (*.bs" << reader.compressionList << ");;"
  << "Sufami Turbo cartridges (*.st" << reader.compressionList << ");;"
  << "Game Boy cartridges (*.gb *.sgb *.gbc" << reader.compressionList << ");;"
  << "All files (*)"
  );
  filter->setCurrentIndex(config().path.current.filter);
  show();
}

void DiskBrowser::loadBaseCartridge() {
  currentPath = &config().path.current.cartridge;
  browseMode = BaseCartridge;

  hide();
  group->setVisible(config().diskBrowser.showPanel);
  ok->setText("Load");
  setWindowTitle("Load Base Cartridge");
  setPath(config().path.rom == "" ? *currentPath : config().path.rom);
  setNameFilters(string()
  << "SNES cartridges (*.sfc" << reader.extensionList << reader.compressionList << ");;"
  << "All files (*)"
  );
  show();
}

void DiskBrowser::loadBsxCartridge() {
  currentPath = &config().path.current.bsx;
  browseMode = BsxCartridge;

  hide();
  group->setVisible(config().diskBrowser.showPanel);
  ok->setText("Load");
  setWindowTitle("Load BS-X Cartridge");
  setPath(config().path.rom == "" ? *currentPath : config().path.rom);
  setNameFilters(string()
  << "BS-X cartridges (*.bs" << reader.compressionList << ");;"
  << "All files (*)"
  );
  show();
}

void DiskBrowser::loadSufamiTurboCartridge1() {
  currentPath = &config().path.current.st;
  browseMode = SufamiTurboCartridge1;

  hide();
  group->setVisible(config().diskBrowser.showPanel);
  ok->setText("Load");
  setWindowTitle("Load Slot-A Sufami Turbo Cartridge");
  setPath(config().path.rom == "" ? *currentPath : config().path.rom);
  setNameFilters(string()
  << "Sufami Turbo cartridges (*.st" << reader.compressionList << ");;"
  << "All files (*)"
  );
  show();
}

void DiskBrowser::loadSufamiTurboCartridge2() {
  currentPath = &config().path.current.st;
  browseMode = SufamiTurboCartridge2;

  hide();
  group->setVisible(config().diskBrowser.showPanel);
  ok->setText("Load");
  setWindowTitle("Load Slot-B Sufami Turbo Cartridge");
  setPath(config().path.rom == "" ? *currentPath : config().path.rom);
  setNameFilters(string()
  << "Sufami Turbo Cartridges (*.st" << reader.compressionList << ");;"
  << "All files (*)"
  );
  show();
}

void DiskBrowser::loadSuperGameBoyCartridge() {
  currentPath = &config().path.current.sgb;
  browseMode = SuperGameBoyCartridge;

  hide();
  group->setVisible(config().diskBrowser.showPanel);
  ok->setText("Load");
  setWindowTitle("Load Super Game Boy Cartridge");
  setPath(config().path.rom == "" ? *currentPath : config().path.rom);
  setNameFilters(string()
  << "Game Boy cartridges (*.gb *.sgb *.gbc" << reader.compressionList << ");;"
  << "All files (*)"
  );
  show();
}

string DiskBrowser::queryImageInformation() {
  string text;
  string filename;
  if(currentFilename(filename) == true) {
    Cartridge::Information info;
    if(cartridge.information(filename, info)) {
      unsigned size = file::size(filename);
      text << "<small><table>";
      text << "<tr><td><b>Title: </b></td><td>" << info.name << "</td></tr>";
      text << "<tr><td><b>Region: </b></td><td>" << info.region << "</td></tr>";
      text << "<tr><td><b>ROM: </b></td><td>" << info.romSize * 8 / 1024 / 1024 << "mbit</td></tr>";
      text << "<tr><td><b>RAM: </b></td><td>";
      info.ramSize ? text << info.ramSize * 8 / 1024 << "kbit</td></tr>" : text << "None</td></tr>";
      text << "</table></small>";
    }
  }
  return text;
}

void DiskBrowser::activateItem(const QModelIndex &item) {
  if(browseMode == Folder) {
    setPath(model->filePath(item));
  } else {
    loadSelected();
  }
}

void DiskBrowser::changeItem(const QModelIndex &item) {
  if(browseMode == Folder) {
    ok->setEnabled(model->isDir(item));
    image->name = "";
    info->setText("");
    applyPatch->setVisible(false);
  } else {
    string filename;
    currentFilename(filename);

    if(filename.length() == 0) {
      //nothing selected?
      ok->setEnabled(false);
      image->name = "";
      info->setText("");
      applyPatch->setVisible(false);
    } else {
      ok->setEnabled(true);
      image->name = nall::basename(filename) << ".png";
      if(file::exists(image->name) == false) image->name = "";
      info->setText(string() << queryImageInformation());
      string patch = nall::basename(filename) << ".ups";
      applyPatch->setVisible(file::exists(patch));
    }
  }

  image->update();
}

void DiskBrowser::loadSelected() {
  string filename;
  bool loadable = currentFilename(filename);

  if(browseMode == Folder || loadable == true) {
    QModelIndex item = view->currentIndex();
    if(currentPath) *currentPath = dir(model->filePath(item).toUtf8().constData());
    hide();

    if(browseMode == Folder || browseMode == File) {
      callback(filename);
    } else if(browseMode == Cartridge) {
      //quick-loading mode: determine load type via active filter
      config().path.current.filter = filter->currentIndex();

      if(config().path.current.filter == 1) {  //"BS-X cartridges"
        if(config().path.bsx == "") {
          loaderWindow->loadBsxCartridge("", filename);
        } else {
          cartridge.loadBsx(config().path.bsx, filename);
        }
      } else if(config().path.current.filter == 2) {  //"Sufami Turbo cartridges"
        if(config().path.st == "") {
          loaderWindow->loadSufamiTurboCartridge("", filename, "");
        } else {
          cartridge.loadSufamiTurbo(config().path.st, filename, "");
        }
      } else if(config().path.current.filter == 3) {  //"Game Boy cartridges"
        if(SNES::supergameboy.opened() == false) {
          QMessageBox::warning(0, "Warning", "Super Game Boy support missing - cartridge cannot be loaded.");
        } else if(config().path.sgb == "") {
          loaderWindow->loadSuperGameBoyCartridge("", filename);
        } else {
          cartridge.loadSuperGameBoy(config().path.sgb, filename);
        }
      } else {  //"SNES cartridges" (0) or "All files" (4)
        cartridge.loadNormal(filename);
      }
    } else if(browseMode == BaseCartridge) {
      loaderWindow->selectBaseCartridge(filename);
    } else if(browseMode == BsxCartridge) {
      loaderWindow->selectSlot1Cartridge(filename);
    } else if(browseMode == SufamiTurboCartridge1) {
      loaderWindow->selectSlot1Cartridge(filename);
    } else if(browseMode == SufamiTurboCartridge2) {
      loaderWindow->selectSlot2Cartridge(filename);
    } else if(browseMode == SuperGameBoyCartridge) {
      loaderWindow->selectSlot1Cartridge(filename);
    }
  } else {
    //this is a standard folder in ROM loading mode; enter into the folder
    QModelIndex item = view->currentIndex();
    setPath(model->filePath(item));
  }
}

//

void DiskBrowser::setPath(const QString &reqPath) {
  disconnect(path, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePath()));

  QString effectivePath = reqPath;
  if(effectivePath == "") {
    effectivePath = QString::fromUtf8(config().path.startup);
  }
  if(effectivePath == "<root>" || QDir(reqPath).exists() == false) {
    effectivePath = "";
    newFolder->setEnabled(false);
  } else {
    newFolder->setEnabled(true);
  }
  path->clear();
  model->setRootPath(effectivePath);
  view->setRootIndex(model->index(effectivePath));
  view->setCurrentIndex(view->rootIndex());
  view->setFocus();

  if(effectivePath.length() > 0) {
    QDir directory(effectivePath);
    while(true) {
      path->addItem(directory.absolutePath());
      if(directory.isRoot()) break;
      directory.cdUp();
    }
  }
  path->addItem("<root>");

  connect(path, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePath()));
}

void DiskBrowser::setNameFilters(const QString &filters) {
  disconnect(filter, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFilter()));

  filter->clear();

  string filterData = filters.toUtf8().constData();
  lstring filterPart;
  filterPart.split(";;", filterData);

  for(unsigned i = 0; i < filterPart.size(); i++) {
    filter->addItem(filterPart[i]);
  }

  connect(filter, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFilter()));
  updateFilter();
}

void DiskBrowser::cdUp() {
  folderCreator->hide();
  //if we aren't already at the root node, select the second node, which is one path higher than the current
  if(path->count() > 1) path->setCurrentIndex(1);
}

void DiskBrowser::updatePath() {
  setPath(path->currentText());
}

void DiskBrowser::updateFilter() {
  QString currentText = filter->currentText();
  if(currentText.length() == 0) {
    model->setNameFilters(QStringList() << "*");  
  } else {
    string filters = currentText.toUtf8().constData();
    filters = substr(filters, strpos(filters, "("));
    ltrim(filters, "(");
    rtrim(filters, ")");
    lstring filterPart;
    filterPart.split(" ", filters);
    QStringList filterList;
    for(unsigned i = 0; i < filterPart.size(); i++) filterList << (const char*)filterPart[i];
    model->setNameFilters(filterList);
  }
}

//true means filename can be loaded directly
//false means it cannot (eg this is a folder and we are attempting to load a ROM)
bool DiskBrowser::currentFilename(string &filename) {
  bool loadable = false;
  QModelIndex item = view->currentIndex();
  filename = model->filePath(item).toUtf8().constData();

  if(browseMode != Folder) {
    if(model->isDir(item)) {
      if(browseMode != File) {
        //folders ending in ".sfc" are treated as "packages", and loaded directly
        if(striend(filename, ".sfc")) {
          QDir directory(filename);
          directory.setNameFilters(QStringList() << "*.sfc");
          QStringList list = directory.entryList(QDir::Files | QDir::NoDotAndDotDot);
          if(list.count() == 1) {
            filename << "/" << list[0].toUtf8().constData();
            loadable = true;
          }
        }
      }
    } else {
      loadable = true;
    }
  }

  return loadable;
}

void DiskBrowser::toggleApplyPatches() {
  config().file.applyPatches = applyPatch->isChecked();
}

void DiskBrowser::toggleShowPanel() {
  showPanel->setChecked(!showPanel->isChecked());
  config().diskBrowser.showPanel = showPanel->isChecked();
  group->setVisible(config().diskBrowser.showPanel);
}

DiskBrowser::DiskBrowser() : QbWindow(config().geometry.diskBrowser) {
  setObjectName("disk-browser");
  resize(720, 480);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  topLayout = new QHBoxLayout;
  layout->addLayout(topLayout);

  browseLayout = new QVBoxLayout;
  topLayout->addLayout(browseLayout);

  pathLayout = new QHBoxLayout;
  browseLayout->addLayout(pathLayout);

  path = new QComboBox;
  path->setEditable(true);
  path->setMinimumContentsLength(16);
  path->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
  path->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  pathLayout->addWidget(path);

  newFolder = new QPushButton;
  newFolder->setIconSize(QSize(16, 16));
  newFolder->setIcon(QIcon(":/16x16/folder-new.png"));
  pathLayout->addWidget(newFolder);

  view = new DiskBrowserView;
  view->setIconSize(QSize(16, 16));
  browseLayout->addWidget(view);

  panelLayout = new QVBoxLayout;
  topLayout->addLayout(panelLayout);

  group = new QGroupBox;
  panelLayout->addWidget(group);

  groupLayout = new QVBoxLayout;
  group->setLayout(groupLayout);

  info = new QLabel;
  info->setFixedWidth(256);
  groupLayout->addWidget(info);

  image = new DiskBrowserImage;
  image->setFixedSize(256, 239);
  groupLayout->addWidget(image);

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  groupLayout->addWidget(spacer);

  applyPatch = new QCheckBox("Apply UPS patch");
  applyPatch->setChecked(config().file.applyPatches);
  groupLayout->addWidget(applyPatch);

  controlLayout = new QHBoxLayout;
  layout->addLayout(controlLayout);

  filter = new QComboBox;
  filter->setMinimumContentsLength(16);
  filter->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
  filter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  controlLayout->addWidget(filter);

  options = new QPushButton("Options");
  controlLayout->addWidget(options);

  menu = new QMenu;
  options->setMenu(menu);

  menu->addAction(showPanel = new QbCheckAction("Show Side Panel", 0));
  showPanel->setChecked(config().diskBrowser.showPanel);

  ok = new QPushButton("Ok");
  ok->setEnabled(false);
  controlLayout->addWidget(ok);

  cancel = new QPushButton("Cancel");
  controlLayout->addWidget(cancel);

  model = new QFileSystemModel;
  model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
  model->setNameFilterDisables(false);

  view->setModel(model);
  view->setExpandsOnDoubleClick(false);
  view->setAllColumnsShowFocus(true);
  view->setUniformRowHeights(true);
  view->setSortingEnabled(true);
  view->sortByColumn(0, Qt::AscendingOrder);
  view->setColumnHidden(1, true);
  view->setColumnHidden(2, true);
  view->setColumnHidden(3, true);
  view->setHeaderHidden(true);

  folderCreator = new FolderCreator;

  connect(newFolder, SIGNAL(released()), folderCreator, SLOT(show()));
  connect(view, SIGNAL(cdUp()), this, SLOT(cdUp()));
  connect(view, SIGNAL(activated(const QModelIndex&)), this, SLOT(activateItem(const QModelIndex&)));
  connect(view, SIGNAL(changed(const QModelIndex&)), this, SLOT(changeItem(const QModelIndex&)));
  connect(ok, SIGNAL(released()), this, SLOT(loadSelected()));
  connect(cancel, SIGNAL(released()), this, SLOT(close()));

  connect(applyPatch, SIGNAL(stateChanged(int)), this, SLOT(toggleApplyPatches()));
  connect(showPanel, SIGNAL(triggered()), this, SLOT(toggleShowPanel()));
}
