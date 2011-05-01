class Debugger : public QbWindow {
  Q_OBJECT

public:
  QMenuBar *menu;
  QMenu *menu_tools;
  QAction *menu_tools_disassembler;
  QAction *menu_tools_breakpoint;
  QAction *menu_tools_memory;
  QAction *menu_tools_propertiesViewer;
  QMenu *menu_ppu;
  QAction *menu_ppu_layerToggle;
  QAction *menu_ppu_vramViewer;
  QAction *menu_ppu_oamViewer;
  QAction *menu_ppu_cgramViewer;
  QMenu *menu_misc;
  QAction *menu_misc_clear;
  QAction *menu_misc_options;

  QHBoxLayout *layout;
  QTextEdit *console;
  QVBoxLayout *controlLayout;
  QHBoxLayout *commandLayout;
  QPushButton *runBreak;
  QPushButton *stepInstruction;
  QCheckBox *stepCPU;
  QCheckBox *stepSMP;
  QCheckBox *traceCPU;
  QCheckBox *traceSMP;
  QCheckBox *traceMask;
  QWidget *spacer;

  void modifySystemState(unsigned);
  void echo(const char *message);
  void event();
  void frameTick();
  void autoUpdate();
  Debugger();

public slots:
  void clear();
  void synchronize();

  void toggleRunStatus();
  void stepAction();

private:
  unsigned frameCounter;
};

extern Debugger *debugger;
