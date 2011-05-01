class PropertiesWidget : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTreeWidget *list;

  PropertiesWidget(SNES::ChipDebugger &object);

public slots:
  void refresh();

private:
  SNES::ChipDebugger &object;
};

class PropertiesViewer : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTabWidget *tabWidget;
  QHBoxLayout *controlLayout;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;

  void autoUpdate();
  PropertiesViewer();

public slots:
  void refresh();
  void show();
};

extern PropertiesWidget *cpuPropertiesTab;
extern PropertiesWidget *ppuPropertiesTab;
extern PropertiesViewer *propertiesViewer;
