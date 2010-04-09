MemoryEditor::MemoryEditor() : QbWindow(config.geometry.memoryEditor) {
  setObjectName("memory-editor");
  setWindowTitle("Memory Editor");

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  controlLayout = new QHBoxLayout;
  controlLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(controlLayout);

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  controlLayout->addWidget(spacer);

  addr = new QLineEdit;
  addr->setFixedWidth(80);
  controlLayout->addWidget(addr);

  source = new QComboBox;
  source->addItem("S-CPU bus");
  source->addItem("S-APU bus");
  source->addItem("S-PPU VRAM");
  source->addItem("S-PPU OAM");
  source->addItem("S-PPU CGRAM");
  controlLayout->addWidget(source);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);

  editor = new HexEditor;
  editor->reader = bind(&MemoryEditor::reader, this);
  editor->writer = bind(&MemoryEditor::writer, this);
  editor->setSize(16 * 1024 * 1024);
  memorySource = SNES::Debugger::CPUBus;
  layout->addWidget(editor);

  connect(addr, SIGNAL(textEdited(const QString&)), this, SLOT(refresh()));
  connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceChanged(int)));
  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
}

void MemoryEditor::autoUpdate() {
  if(SNES::cartridge.loaded() && autoUpdateBox->isChecked()) editor->update();
}

void MemoryEditor::synchronize() {
  if(SNES::cartridge.loaded() == false) {
    addr->setEnabled(false);
    source->setEnabled(false);
    autoUpdateBox->setEnabled(false);
    refreshButton->setEnabled(false);
    editor->setHtml("");
    editor->scrollbar->setEnabled(false);
  } else {
    addr->setEnabled(true);
    source->setEnabled(true);
    autoUpdateBox->setEnabled(true);
    refreshButton->setEnabled(true);
    editor->scrollbar->setEnabled(true);
  }
}

void MemoryEditor::refresh() {
  if(SNES::cartridge.loaded() == false) {
    editor->setHtml("");
    return;
  }

  editor->setOffset(strhex(addr->text().toUtf8().data()));
  editor->update();
}

void MemoryEditor::sourceChanged(int index) {
  switch(index) { default:
    case 0: memorySource = SNES::Debugger::CPUBus; editor->setSize(16 * 1024 * 1024); break;
    case 1: memorySource = SNES::Debugger::APURAM; editor->setSize(64 * 1024);        break;
    case 2: memorySource = SNES::Debugger::VRAM;   editor->setSize(64 * 1024);        break;
    case 3: memorySource = SNES::Debugger::OAM;    editor->setSize(544);              break;
    case 4: memorySource = SNES::Debugger::CGRAM;  editor->setSize(512);              break;
  }

  editor->setOffset(strhex(addr->text().toUtf8().data()));
  editor->update();
}

uint8 MemoryEditor::reader(unsigned addr) {
  return SNES::debugger.read(memorySource, addr);
}

void MemoryEditor::writer(unsigned addr, uint8 data) {
  SNES::debugger.write(memorySource, addr, data);
}
