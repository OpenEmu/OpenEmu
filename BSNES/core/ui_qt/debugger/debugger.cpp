#include "hexeditor.cpp"
#include "breakpoint.cpp"
#include "memory.cpp"
#include "vramviewer.cpp"

Debugger::Debugger() : QbWindow(config.geometry.debugger) {
  setObjectName("debugger");
  setWindowTitle("Debugger");

  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  menu = new QMenuBar;
  layout->setMenuBar(menu);

  tools = menu->addMenu("Tools");
  tools_breakpoint = tools->addAction("Breakpoint Editor ...");
  tools_breakpoint->setIcon(QIcon(":/16x16/process-stop.png"));
  tools_memory = tools->addAction("Memory Editor ...");
  tools_memory->setIcon(QIcon(":/16x16/text-x-generic.png"));
  tools_vramViewer = tools->addAction("Video RAM Viewer ...");
  tools_vramViewer->setIcon(QIcon(":/16x16/image-x-generic.png"));

  miscOptions = menu->addMenu("Misc");
  miscOptions_clear = miscOptions->addAction("Clear Console");
  miscOptions_clear->setIcon(QIcon(":/16x16/document-new.png"));

  console = new QTextEdit;
  console->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  console->setReadOnly(true);
  console->setFont(QFont(Style::Monospace));
  console->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  console->setMinimumWidth((92 + 4) * console->fontMetrics().width(' '));
  console->setMinimumHeight((25 + 1) * console->fontMetrics().height());
  layout->addWidget(console);

  controlLayout = new QVBoxLayout;
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  commandLayout = new QHBoxLayout;
  controlLayout->addLayout(commandLayout);

  runBreak = new QPushButton("Break");
  commandLayout->addWidget(runBreak);
  commandLayout->addSpacing(Style::WidgetSpacing);

  stepInstruction = new QPushButton("Step");
  commandLayout->addWidget(stepInstruction);

  controlLayout->addSpacing(Style::WidgetSpacing);

  stepCPU = new QCheckBox("Step CPU");
  controlLayout->addWidget(stepCPU);

  stepSMP = new QCheckBox("Step SMP");
  controlLayout->addWidget(stepSMP);

  traceCPU = new QCheckBox("Trace CPU opcodes");
  controlLayout->addWidget(traceCPU);

  traceSMP = new QCheckBox("Trace SMP opcodes");
  controlLayout->addWidget(traceSMP);

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  controlLayout->addWidget(spacer);

  connect(tools_breakpoint, SIGNAL(triggered()), this, SLOT(showBreakpointEditor()));
  connect(tools_memory, SIGNAL(triggered()), this, SLOT(showMemoryEditor()));
  connect(tools_vramViewer, SIGNAL(triggered()), this, SLOT(showVramViewer()));
  connect(miscOptions_clear, SIGNAL(triggered()), this, SLOT(clear()));

  connect(runBreak, SIGNAL(released()), this, SLOT(toggleRunStatus()));
  connect(stepInstruction, SIGNAL(released()), this, SLOT(stepAction()));
  connect(stepCPU, SIGNAL(released()), this, SLOT(synchronize()));
  connect(stepSMP, SIGNAL(released()), this, SLOT(synchronize()));
  connect(traceCPU, SIGNAL(released()), this, SLOT(toggleTraceCPU()));
  connect(traceSMP, SIGNAL(released()), this, SLOT(toggleTraceSMP()));

  breakpointEditor = new BreakpointEditor;
  memoryEditor = new MemoryEditor;
  vramViewer = new VramViewer;

  frameCounter = 0;
  synchronize();
  resize(855, 425);
}

void Debugger::synchronize() {
  runBreak->setText(application.debug ? "Run" : "Break");
  stepInstruction->setEnabled(SNES::cartridge.loaded() && application.debug && (stepCPU->isChecked() || stepSMP->isChecked()));
  SNES::debugger.step_cpu = application.debug && stepCPU->isChecked();
  SNES::debugger.step_smp = application.debug && stepSMP->isChecked();

  memoryEditor->synchronize();
}

void Debugger::echo(const char *message) {
  console->moveCursor(QTextCursor::End);
  console->insertHtml(message);
}

void Debugger::clear() {
  console->setHtml("");
}

void Debugger::showBreakpointEditor() {
  breakpointEditor->show();
}

void Debugger::showMemoryEditor() {
  memoryEditor->show();
}

void Debugger::showVramViewer() {
  vramViewer->show();
  vramViewer->refresh();
}

void Debugger::toggleRunStatus() {
  application.debug = !application.debug;
  if(!application.debug) application.debugrun = false;
  synchronize();
}

void Debugger::stepAction() {
  application.debugrun = true;
}

void Debugger::tracerUpdate() {
  if(SNES::debugger.trace_cpu || SNES::debugger.trace_smp) {
    if(SNES::debugger.tracefile.open() == false) {
      SNES::debugger.tracefile.open(utf8() << config.path.data << "trace.log", file::mode_write);
    }
  } else if(!SNES::debugger.trace_cpu && !SNES::debugger.trace_smp) {
    if(SNES::debugger.tracefile.open() == true) {
      SNES::debugger.tracefile.close();
    }
  }
}

void Debugger::toggleTraceCPU() {
  SNES::debugger.trace_cpu = traceCPU->isChecked();
  tracerUpdate();
}

void Debugger::toggleTraceSMP() {
  SNES::debugger.trace_smp = traceSMP->isChecked();
  tracerUpdate();
}

void Debugger::event() {
  char t[256];

  switch(SNES::debugger.break_event) {
    case SNES::Debugger::BreakpointHit: {
      unsigned n = SNES::debugger.breakpoint_hit;
      echo(utf8() << "Breakpoint " << n << " hit (" << SNES::debugger.breakpoint[n].counter << ").<br>");

      if(SNES::debugger.breakpoint[n].mode == SNES::Debugger::Breakpoint::Exec) {
        if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::CPUBus) {
          SNES::debugger.step_cpu = true;
          SNES::cpu.disassemble_opcode(t);
          echo(utf8() << t << "<br>");
        }

        if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::APURAM) {
          SNES::debugger.step_smp = true;
          SNES::smp.disassemble_opcode(t);
          echo(utf8() << t << "<br>");
        }
      }
    } break;

    case SNES::Debugger::CPUStep: {
      SNES::cpu.disassemble_opcode(t);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(utf8() << "<font color='#0000a0'>" << s << "</font><br>");
    } break;

    case SNES::Debugger::SMPStep: {
      SNES::smp.disassemble_opcode(t);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(utf8() << "<font color='#a00000'>" << s << "</font><br>");
    } break;
  }
}

//called once every time a video frame is rendered, used to update "auto refresh" tool windows
void Debugger::frameTick() {
  if(++frameCounter >= (SNES::system.region() == SNES::System::NTSC ? 60 : 50)) {
    frameCounter = 0;
    memoryEditor->autoUpdate();
    vramViewer->autoUpdate();
  }
}
