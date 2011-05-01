#include "../ui-base.hpp"

#include "tools.moc"
ToolsWindow *toolsWindow;

#include "cheateditor.cpp"
#include "cheatfinder.cpp"
#include "statemanager.cpp"

ToolsWindow::ToolsWindow() : QbWindow(config().geometry.toolsWindow) {
  setObjectName("tools-window");
  setWindowTitle("Tools");
  resize(600, 360);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  cheatEditorWindow = new CheatEditorWindow;
  cheatFinderWindow = new CheatFinderWindow;
  stateManagerWindow = new StateManagerWindow;

  tab = new QTabWidget;
  tab->addTab(cheatEditorWindow, QIcon(":/16x16/accessories-text-editor.png"), "Cheat Editor");
  tab->addTab(cheatFinderWindow, QIcon(":/16x16/system-search.png"), "Cheat Finder");
  tab->addTab(stateManagerWindow, QIcon(":/16x16/system-file-manager.png"), "State Manager");
  layout->addWidget(tab);
}
