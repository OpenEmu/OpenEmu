#include "../ui-base.hpp"

#if defined(DEBUGGER)

#include "debugger.moc"
Debugger *debugger;

#include "hexeditor.cpp"
#include "tracer.cpp"

#include "tools/disassembler.cpp"
#include "tools/breakpoint.cpp"
#include "tools/memory.cpp"
#include "tools/properties.cpp"

#include "ppu/layer-toggle.cpp"
#include "ppu/vram-viewer.cpp"
#include "ppu/oam-viewer.cpp"
#include "ppu/cgram-viewer.cpp"

#include "misc/debugger-options.cpp"

Debugger::Debugger() : QbWindow(config().geometry.debugger) {
  setObjectName("debugger");
  setWindowTitle("Debugger");

  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  menu = new QMenuBar;
  layout->setMenuBar(menu);

  menu_tools = menu->addMenu("Tools");
  menu_tools_disassembler = menu_tools->addAction("Disassembler ...");
  menu_tools_breakpoint = menu_tools->addAction("Breakpoint Editor ...");
  menu_tools_memory = menu_tools->addAction("Memory Editor ...");
  menu_tools_propertiesViewer = menu_tools->addAction("Properties Viewer ...");

  menu_ppu = menu->addMenu("S-PPU");
  menu_ppu_layerToggle = menu_ppu->addAction("Layer Toggle ...");
  menu_ppu_vramViewer = menu_ppu->addAction("Video RAM Viewer ...");
  menu_ppu_oamViewer = menu_ppu->addAction("Sprite Viewer ...");
  menu_ppu_cgramViewer = menu_ppu->addAction("Palette Viewer ...");

  menu_misc = menu->addMenu("Misc");
  menu_misc_clear = menu_misc->addAction("Clear Console");
  menu_misc_options = menu_misc->addAction("Options ...");

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

  stepCPU = new QCheckBox("Step S-CPU");
  controlLayout->addWidget(stepCPU);

  stepSMP = new QCheckBox("Step S-SMP");
  controlLayout->addWidget(stepSMP);

  traceCPU = new QCheckBox("Trace S-CPU opcodes");
  controlLayout->addWidget(traceCPU);

  traceSMP = new QCheckBox("Trace S-SMP opcodes");
  controlLayout->addWidget(traceSMP);

  traceMask = new QCheckBox("Enable trace mask");
  controlLayout->addWidget(traceMask);

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  controlLayout->addWidget(spacer);

  tracer = new Tracer;
  disassembler = new Disassembler;
  breakpointEditor = new BreakpointEditor;
  memoryEditor = new MemoryEditor;
  propertiesViewer = new PropertiesViewer;
  layerToggle = new LayerToggle;
  vramViewer = new VramViewer;
  oamViewer = new OamViewer;
  cgramViewer = new CgramViewer;
  debuggerOptions = new DebuggerOptions;

  connect(menu_tools_disassembler, SIGNAL(triggered()), disassembler, SLOT(show()));
  connect(menu_tools_breakpoint, SIGNAL(triggered()), breakpointEditor, SLOT(show()));
  connect(menu_tools_memory, SIGNAL(triggered()), memoryEditor, SLOT(show()));
  connect(menu_tools_propertiesViewer, SIGNAL(triggered()), propertiesViewer, SLOT(show()));

  connect(menu_ppu_layerToggle, SIGNAL(triggered()), layerToggle, SLOT(show()));
  connect(menu_ppu_vramViewer, SIGNAL(triggered()), vramViewer, SLOT(show()));
  connect(menu_ppu_oamViewer, SIGNAL(triggered()), oamViewer, SLOT(show()));
  connect(menu_ppu_cgramViewer, SIGNAL(triggered()), cgramViewer, SLOT(show()));

  connect(menu_misc_clear, SIGNAL(triggered()), this, SLOT(clear()));
  connect(menu_misc_options, SIGNAL(triggered()), debuggerOptions, SLOT(show()));

  connect(runBreak, SIGNAL(released()), this, SLOT(toggleRunStatus()));
  connect(stepInstruction, SIGNAL(released()), this, SLOT(stepAction()));
  connect(stepCPU, SIGNAL(released()), this, SLOT(synchronize()));
  connect(stepSMP, SIGNAL(released()), this, SLOT(synchronize()));
  connect(traceCPU, SIGNAL(stateChanged(int)), tracer, SLOT(setCpuTraceState(int)));
  connect(traceSMP, SIGNAL(stateChanged(int)), tracer, SLOT(setSmpTraceState(int)));
  connect(traceMask, SIGNAL(stateChanged(int)), tracer, SLOT(setTraceMaskState(int)));

  frameCounter = 0;
  synchronize();
  resize(855, 425);
}

void Debugger::modifySystemState(unsigned state) {
  string usagefile = filepath(nall::basename(cartridge.fileName), config().path.data);
  usagefile << "-usage.bin";
  file fp;

  if(state == Utility::LoadCartridge) {
    if(config().debugger.cacheUsageToDisk && fp.open(usagefile, file::mode_read)) {
      fp.read(SNES::cpu.usage, 1 << 24);
      fp.read(SNES::smp.usage, 1 << 16);
      fp.close();
    } else {
      memset(SNES::cpu.usage, 0x00, 1 << 24);
      memset(SNES::smp.usage, 0x00, 1 << 16);
    }
  }

  if(state == Utility::UnloadCartridge) {
    if(config().debugger.cacheUsageToDisk && fp.open(usagefile, file::mode_write)) {
      fp.write(SNES::cpu.usage, 1 << 24);
      fp.write(SNES::smp.usage, 1 << 16);
      fp.close();
    }
  }
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

void Debugger::toggleRunStatus() {
  application.debug = !application.debug;
  if(!application.debug) application.debugrun = false;
  synchronize();
}

void Debugger::stepAction() {
  application.debugrun = true;
}

void Debugger::event() {
  char t[256];

  switch(SNES::debugger.break_event) {
    case SNES::Debugger::BreakpointHit: {
      unsigned n = SNES::debugger.breakpoint_hit;
      echo(string() << "Breakpoint " << n << " hit (" << SNES::debugger.breakpoint[n].counter << ").<br>");

      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::CPUBus) {
        SNES::debugger.step_cpu = true;
        SNES::cpu.disassemble_opcode(t, SNES::cpu.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a00000'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::CPU, SNES::cpu.opcode_pc);
      }

      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::APURAM) {
        SNES::debugger.step_smp = true;
        SNES::smp.disassemble_opcode(t, SNES::smp.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a00000'>" << t << "</font><br>");
        disassembler->refresh(Disassembler::SMP, SNES::smp.opcode_pc);
      }
    } break;

    case SNES::Debugger::CPUStep: {
      SNES::cpu.disassemble_opcode(t, SNES::cpu.regs.pc);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#0000a0'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::CPU, SNES::cpu.regs.pc);
    } break;

    case SNES::Debugger::SMPStep: {
      SNES::smp.disassemble_opcode(t, SNES::smp.regs.pc);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#a00000'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::SMP, SNES::smp.regs.pc);
    } break;
  }

  autoUpdate();
}

//called once every time a video frame is rendered, used to update "auto refresh" tool windows
void Debugger::frameTick() {
  if(++frameCounter >= (SNES::system.region() == SNES::System::NTSC ? 60 : 50)) {
    frameCounter = 0;
    autoUpdate();
  }
}

void Debugger::autoUpdate() {
  memoryEditor->autoUpdate();
  propertiesViewer->autoUpdate();
  vramViewer->autoUpdate();
  oamViewer->autoUpdate();
  cgramViewer->autoUpdate();
}

#endif
