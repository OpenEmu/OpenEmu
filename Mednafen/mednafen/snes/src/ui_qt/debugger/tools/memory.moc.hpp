class MemoryEditor : public QbWindow {
  Q_OBJECT

public:
  QHBoxLayout *layout;
  HexEditor *editor;
  QVBoxLayout *controlLayout;
  QComboBox *source;
  QLineEdit *addr;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  QWidget *spacer;
  QPushButton *exportButton;
  QPushButton *importButton;

  void autoUpdate();
  void synchronize();

  SNES::Debugger::MemorySource memorySource;
  uint8 reader(unsigned addr);
  void writer(unsigned addr, uint8 data);

  MemoryEditor();

public slots:
  void show();
  void sourceChanged(int);
  void refresh();
  void updateOffset();
  void exportMemory();
  void importMemory();
  void exportMemory(SNES::Memory&, const string&) const;
  void importMemory(SNES::Memory&, const string&) const;
};

extern MemoryEditor *memoryEditor;
