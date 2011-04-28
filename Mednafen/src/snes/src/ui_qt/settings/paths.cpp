#include "paths.moc"
PathSettingsWindow *pathSettingsWindow;

PathSettingWidget::PathSettingWidget(string &pathValue_, const char *labelText, const char *pathDefaultLabel_, const char *pathBrowseLabel_) : pathValue(pathValue_) {
  pathDefaultLabel = pathDefaultLabel_;
  pathBrowseLabel = pathBrowseLabel_;

  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  label = new QLabel(labelText);
  layout->addWidget(label);

  controlLayout = new QHBoxLayout;
  controlLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(controlLayout);

  path = new QLineEdit;
  path->setReadOnly(true);
  controlLayout->addWidget(path);

  pathSelect = new QPushButton("Select ...");
  controlLayout->addWidget(pathSelect);

  pathDefault = new QPushButton("Default");
  controlLayout->addWidget(pathDefault);

  connect(pathSelect, SIGNAL(released()), this, SLOT(selectPath()));
  connect(pathDefault, SIGNAL(released()), this, SLOT(defaultPath()));
  updatePath();
}

void PathSettingWidget::assignPath(string newPath) {
  pathValue = string() << newPath << "/";
  updatePath();
}

void PathSettingWidget::updatePath() {
  if(pathValue == "") {
    path->setStyleSheet("color: #808080");
    path->setText(pathDefaultLabel);
  } else {
    path->setStyleSheet("color: #000000");
    path->setText(pathValue);
  }
}

void PathSettingWidget::selectPath() {
  diskBrowser->chooseFolder(
    bind(&PathSettingWidget::assignPath, this),
    config().path.current.folder,
    pathBrowseLabel
  );
}

void PathSettingWidget::defaultPath() {
  pathValue = "";
  updatePath();
}

PathSettingsWindow::PathSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  gamePath  = new PathSettingWidget(config().path.rom,   "Games:",         "Remember last path",  "Default Game Path");
  savePath  = new PathSettingWidget(config().path.save,  "Save RAM:",      "Same as loaded game", "Default Save RAM Path");
  statePath = new PathSettingWidget(config().path.state, "Save states:",   "Same as loaded game", "Default Save State Path");
  patchPath = new PathSettingWidget(config().path.patch, "UPS patches:",   "Same as loaded game", "Default UPS Patch Path");
  cheatPath = new PathSettingWidget(config().path.cheat, "Cheat codes:",   "Same as loaded game", "Default Cheat Code Path");
  dataPath  = new PathSettingWidget(config().path.data,  "Exported data:", "Same as loaded game", "Default Exported Data Path");

  layout->addWidget(gamePath);
  layout->addWidget(savePath);
  layout->addWidget(statePath);
  layout->addWidget(patchPath);
  layout->addWidget(cheatPath);
  layout->addWidget(dataPath);
}
