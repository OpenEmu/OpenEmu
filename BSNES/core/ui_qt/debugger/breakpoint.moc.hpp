class BreakpointItem : public QWidget {
  Q_OBJECT

public:
  QHBoxLayout *layout;
  QCheckBox *enabled;
  QLineEdit *addr;
  QLineEdit *data;
  QComboBox *mode;
  QComboBox *source;
  BreakpointItem(unsigned id);

public slots:
  void toggle();

private:
  const unsigned id;
};

class BreakpointEditor : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  BreakpointItem *breakpoint[SNES::Debugger::Breakpoints];

  BreakpointEditor();
} *breakpointEditor;
