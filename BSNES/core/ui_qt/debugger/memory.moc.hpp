class MemoryEditor : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QHBoxLayout *controlLayout;
  QWidget *spacer;
  QLineEdit *addr;
  QComboBox *source;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  HexEditor *editor;

  void autoUpdate();
  void synchronize();

  SNES::Debugger::MemorySource memorySource;
  uint8 reader(unsigned addr);
  void writer(unsigned addr, uint8 data);

  MemoryEditor();

public slots:
  void refresh();
  void sourceChanged(int);
} *memoryEditor;
