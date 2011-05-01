class CPUDisassembler : public QWidget {
public:
  QVBoxLayout *layout;
  QTextEdit *view;
  CPUDisassembler();
};

class SMPDisassembler : public QWidget {
public:
  QVBoxLayout *layout;
  QTextEdit *view;
  SMPDisassembler();
};

class Disassembler : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTabWidget *tab;

  enum Source { CPU, SMP };
  void refresh(Source, unsigned);
  Disassembler();
};

extern CPUDisassembler *cpuDisassembler;
extern SMPDisassembler *smpDisassembler;
extern Disassembler *disassembler;
