#include "../ui-base.hpp"

#include "pixelshader.cpp"
#include "video.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "paths.cpp"
#include "advanced.cpp"

#include "settings.moc"
SettingsWindow *settingsWindow;

SettingsWindow::SettingsWindow() : QbWindow(config().geometry.settingsWindow) {
  setObjectName("settings-window");
  setWindowTitle("Configuration Settings");
  resize(600, 360);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  videoSettingsWindow    = new VideoSettingsWindow;
  audioSettingsWindow    = new AudioSettingsWindow;
  inputSettingsWindow    = new InputSettingsWindow;
  pathSettingsWindow     = new PathSettingsWindow;
  advancedSettingsWindow = new AdvancedSettingsWindow;

  tab = new QTabWidget;
  tab->addTab(videoSettingsWindow, QIcon(":/16x16/video-display.png"), "Video");
  tab->addTab(audioSettingsWindow, QIcon(":/16x16/audio-volume-high.png"), "Audio");
  tab->addTab(inputSettingsWindow, QIcon(":/16x16/input-gaming.png"), "Input");
  tab->addTab(pathSettingsWindow, QIcon(":/16x16/folder.png"), "Paths");
  tab->addTab(advancedSettingsWindow, QIcon(":/16x16/preferences-system.png"), "Advanced");
  layout->addWidget(tab);
}
