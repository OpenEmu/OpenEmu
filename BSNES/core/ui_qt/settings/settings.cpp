#include "video.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "paths.cpp"
#include "advanced.cpp"
#include "utility/inputcapture.cpp"

SettingsWindow::SettingsWindow() : QbWindow(config.geometry.settingsWindow) {
  setObjectName("settings-window");
  setWindowTitle("Configuration Settings");
  resize(625, 360);

  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  splitter = new QSplitter;
  layout->addWidget(splitter);

  //use QTreeWidget instead of QListWidget, as only the former has setAllColumnsShowFocus()
  //this is needed to have the dotted-selection rectangle encompass the icons
  list = new QTreeWidget;
  list->addTopLevelItem(video = new QTreeWidgetItem(QStringList() << "Video"));
  list->addTopLevelItem(audio = new QTreeWidgetItem(QStringList() << "Audio"));
  list->addTopLevelItem(input = new QTreeWidgetItem(QStringList() << "Input"));
  list->addTopLevelItem(paths = new QTreeWidgetItem(QStringList() << "Paths"));
  list->addTopLevelItem(advanced = new QTreeWidgetItem(QStringList() << "Advanced"));
  list->setCurrentItem(input);  //select most frequently used panel by default
  list->setHeaderHidden(true);
  list->setRootIsDecorated(false);
  list->setAllColumnsShowFocus(true);
  list->setIconSize(QSize(22, 22));

  video->setIcon(0, QIcon(":/22x22/video-display.png"));
  audio->setIcon(0, QIcon(":/22x22/audio-volume-high.png"));
  input->setIcon(0, QIcon(":/22x22/input-gaming.png"));
  paths->setIcon(0, QIcon(":/22x22/folder.png"));
  advanced->setIcon(0, QIcon(":/22x22/preferences-system.png"));

  panel = new QWidget;
  panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  splitter->addWidget(list);
  splitter->addWidget(panel);
  splitter->setStretchFactor(0, 2);
  splitter->setStretchFactor(1, 5);

  videoSettingsWindow    = new VideoSettingsWindow;
  audioSettingsWindow    = new AudioSettingsWindow;
  inputSettingsWindow    = new InputSettingsWindow;
  pathSettingsWindow     = new PathSettingsWindow;
  advancedSettingsWindow = new AdvancedSettingsWindow;

  inputCaptureWindow  = new InputCaptureWindow;

  panelLayout = new QStackedLayout(panel);
  panelLayout->addWidget(videoSettingsWindow);
  panelLayout->addWidget(audioSettingsWindow);
  panelLayout->addWidget(inputSettingsWindow);
  panelLayout->addWidget(pathSettingsWindow);
  panelLayout->addWidget(advancedSettingsWindow);
  panel->setLayout(panelLayout);

  connect(list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(itemChanged()));

  itemChanged();
}

void SettingsWindow::itemChanged() {
  QTreeWidgetItem *item = list->currentItem();

  if(item == video)    panelLayout->setCurrentWidget(videoSettingsWindow);
  if(item == audio)    panelLayout->setCurrentWidget(audioSettingsWindow);
  if(item == input)    panelLayout->setCurrentWidget(inputSettingsWindow);
  if(item == paths)    panelLayout->setCurrentWidget(pathSettingsWindow);
  if(item == advanced) panelLayout->setCurrentWidget(advancedSettingsWindow);
}
