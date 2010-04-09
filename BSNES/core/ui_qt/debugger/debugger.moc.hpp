#include "hexeditor.moc"
#include "breakpoint.moc"
#include "memory.moc"
#include "vramviewer.moc"

class Debugger : public QbWindow {
  Q_OBJECT

public:
  QMenuBar *menu;
  QMenu *tools;
  QAction *tools_breakpoint;
  QAction *tools_memory;
  QAction *tools_vramViewer;
  QMenu *miscOptions;
  QAction *miscOptions_clear;

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
  QWidget *spacer;

  void echo(const char *message);
  void tracerUpdate();
  void event();
  void frameTick();
  Debugger();

public slots:
  void showBreakpointEditor();
  void showMemoryEditor();
  void showVramViewer();
  void clear();
  void synchronize();

  void toggleRunStatus();
  void stepAction();
  void toggleTraceCPU();
  void toggleTraceSMP();

private:
  unsigned frameCounter;
} *debugger;
