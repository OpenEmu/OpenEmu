#include "cheateditor.moc"
#include "cheatfinder.moc"
#include "statemanager.moc"

class ToolsWindow : public QbWindow {
  Q_OBJECT

public:
  QHBoxLayout *layout;
  QSplitter *splitter;
  QTreeWidget *list;
  QTreeWidgetItem *cheatEditor;
  QTreeWidgetItem *cheatFinder;
  QTreeWidgetItem *stateManager;
  QWidget *panel;
  QStackedLayout *panelLayout;

  void showCheatEditor();
  void showCheatFinder();
  void showStateManager();
  ToolsWindow();

public slots:
  void itemChanged();
} *toolsWindow;
