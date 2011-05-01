#include "memory.moc"
MemoryEditor *memoryEditor;

MemoryEditor::MemoryEditor() : QbWindow(config().geometry.memoryEditor) {
  setObjectName("memory-editor");
  setWindowTitle("Memory Editor");

  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  editor = new HexEditor;
  editor->reader = bind(&MemoryEditor::reader, this);
  editor->writer = bind(&MemoryEditor::writer, this);
  editor->setSize(16 * 1024 * 1024);
  memorySource = SNES::Debugger::CPUBus;
  layout->addWidget(editor);

  controlLayout = new QVBoxLayout;
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  source = new QComboBox;
  source->addItem("S-CPU bus");
  source->addItem("S-APU bus");
  source->addItem("S-PPU VRAM");
  source->addItem("S-PPU OAM");
  source->addItem("S-PPU CGRAM");
  controlLayout->addWidget(source);
  controlLayout->addSpacing(2);

  addr = new QLineEdit;
  controlLayout->addWidget(addr);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  controlLayout->addWidget(spacer);

  exportButton = new QPushButton("Export");
  controlLayout->addWidget(exportButton);

  importButton = new QPushButton("Import");
  controlLayout->addWidget(importButton);

  connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceChanged(int)));
  connect(addr, SIGNAL(textEdited(const QString&)), this, SLOT(updateOffset()));
  connect(addr, SIGNAL(returnPressed()), this, SLOT(updateOffset()));
  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(exportButton, SIGNAL(released()), this, SLOT(exportMemory()));
  connect(importButton, SIGNAL(released()), this, SLOT(importMemory()));
}

void MemoryEditor::autoUpdate() {
  if(SNES::cartridge.loaded() && autoUpdateBox->isChecked()) editor->update();
}

void MemoryEditor::synchronize() {
  if(SNES::cartridge.loaded() == false) {
    editor->setHtml("");
    editor->scrollbar->setEnabled(false);
    source->setEnabled(false);
    addr->setEnabled(false);
    autoUpdateBox->setEnabled(false);
    refreshButton->setEnabled(false);
    exportButton->setEnabled(false);
    importButton->setEnabled(false);
  } else {
    editor->scrollbar->setEnabled(true);
    source->setEnabled(true);
    addr->setEnabled(true);
    autoUpdateBox->setEnabled(true);
    refreshButton->setEnabled(true);
    exportButton->setEnabled(true);
    importButton->setEnabled(true);
  }
}

void MemoryEditor::show() {
  QbWindow::show();
  refresh();
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

void MemoryEditor::refresh() {
  if(SNES::cartridge.loaded() == false) {
    editor->setHtml("");
  } else {
    editor->update();
  }
}

void MemoryEditor::updateOffset() {
  editor->setOffset(strhex(addr->text().toUtf8().data()));
  refresh();
}

void MemoryEditor::exportMemory() {
  string basename = filepath(nall::basename(cartridge.fileName), config().path.data);

  exportMemory(SNES::memory::cartram, string() << basename << "-sram.bin");
  exportMemory(SNES::memory::wram, string() << basename << "-wram.bin");
  exportMemory(SNES::memory::apuram, string() << basename << "-apuram.bin");
  exportMemory(SNES::memory::vram, string() << basename << "-vram.bin");
  exportMemory(SNES::memory::oam, string() << basename << "-oam.bin");
  exportMemory(SNES::memory::cgram, string() << basename << "-cgram.bin");
}

void MemoryEditor::importMemory() {
  string basename = filepath(nall::basename(cartridge.fileName), config().path.data);

  importMemory(SNES::memory::cartram, string() << basename << "-sram.bin");
  importMemory(SNES::memory::wram, string() << basename << "-wram.bin");
  importMemory(SNES::memory::apuram, string() << basename << "-apuram.bin");
  importMemory(SNES::memory::vram, string() << basename << "-vram.bin");
  importMemory(SNES::memory::oam, string() << basename << "-oam.bin");
  importMemory(SNES::memory::cgram, string() << basename << "-cgram.bin");
  refresh();  //in case import changed values that are currently being displayed ...
}

void MemoryEditor::exportMemory(SNES::Memory &memory, const string &filename) const {
  file fp;
  if(fp.open(filename, file::mode_write)) {
    for(unsigned i = 0; i < memory.size(); i++) fp.write(memory.read(i));
    fp.close();
  }
}

void MemoryEditor::importMemory(SNES::Memory &memory, const string &filename) const {
  file fp;
  if(fp.open(filename, file::mode_read)) {
    unsigned filesize = fp.size();
    for(unsigned i = 0; i < memory.size() && i < filesize; i++) memory.write(i, fp.read());
    fp.close();
  }
}

uint8 MemoryEditor::reader(unsigned addr) {
  return SNES::debugger.read(memorySource, addr);
}

void MemoryEditor::writer(unsigned addr, uint8 data) {
  SNES::debugger.write(memorySource, addr, data);
}
