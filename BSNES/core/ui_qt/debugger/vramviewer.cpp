VramViewer::VramViewer() : QbWindow(config.geometry.vramViewer) {
  setObjectName("vram-viewer");
  setWindowTitle("Video RAM Viewer");

  layout = new QVBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setAlignment(Qt::AlignCenter);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  controlLayout = new QHBoxLayout;
  controlLayout->setSizeConstraint(QLayout::SetMinimumSize);
  controlLayout->setAlignment(Qt::AlignRight);
  layout->addLayout(controlLayout);

  depth2bpp = new QRadioButton("2 BPP");
  controlLayout->addWidget(depth2bpp);

  depth4bpp = new QRadioButton("4 BPP");
  controlLayout->addWidget(depth4bpp);

  depth8bpp = new QRadioButton("8 BPP");
  controlLayout->addWidget(depth8bpp);

  depthMode7 = new QRadioButton("Mode 7");
  controlLayout->addWidget(depthMode7);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);

  canvas = new Canvas;
  canvas->setFixedSize(512, 512);
  layout->addWidget(canvas);

  bpp = 2;
  depth2bpp->setChecked(true);

  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(depth2bpp,  SIGNAL(pressed()), this, SLOT(setDepth2bpp()));
  connect(depth4bpp,  SIGNAL(pressed()), this, SLOT(setDepth4bpp()));
  connect(depth8bpp,  SIGNAL(pressed()), this, SLOT(setDepth8bpp()));
  connect(depthMode7, SIGNAL(pressed()), this, SLOT(setDepthMode7()));
}

void VramViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void VramViewer::refresh() {
  canvas->image->fill(0x800000);
  if(SNES::cartridge.loaded()) {
    const uint8_t *source = SNES::memory::vram.data();
    uint32_t *dest = (uint32_t*)canvas->image->bits();
    if(bpp == 2) refresh2bpp (source, dest);
    if(bpp == 4) refresh4bpp (source, dest);
    if(bpp == 8) refresh8bpp (source, dest);
    if(bpp == 7) refreshMode7(source, dest);
  }
  canvas->update();
}

void VramViewer::refresh2bpp(const uint8_t *source, uint32_t *dest) {
  for(unsigned ty = 0; ty < 64; ty++) {
    for(unsigned tx = 0; tx < 64; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        uint8_t d0 = source[0];
        uint8_t d1 = source[1];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 1 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 2 : 0;
          pixel *= 0x55;
          dest[(ty * 8 + py) * 512 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
        }
        source += 2;
      }
    }
  }
}

void VramViewer::refresh4bpp(const uint8_t *source, uint32_t *dest) {
  for(unsigned ty = 0; ty < 32; ty++) {
    for(unsigned tx = 0; tx < 64; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        uint8_t d0 = source[ 0];
        uint8_t d1 = source[ 1];
        uint8_t d2 = source[16];
        uint8_t d3 = source[17];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 1 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 2 : 0;
          pixel |= (d2 & (0x80 >> px)) ? 4 : 0;
          pixel |= (d3 & (0x80 >> px)) ? 8 : 0;
          pixel *= 0x11;
          dest[(ty * 8 + py) * 512 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
        }
        source += 2;
      }
      source += 16;
    }
  }
}

void VramViewer::refresh8bpp(const uint8_t *source, uint32_t *dest) {
  for(unsigned ty = 0; ty < 16; ty++) {
    for(unsigned tx = 0; tx < 64; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        uint8_t d0 = source[ 0];
        uint8_t d1 = source[ 1];
        uint8_t d2 = source[16];
        uint8_t d3 = source[17];
        uint8_t d4 = source[32];
        uint8_t d5 = source[33];
        uint8_t d6 = source[48];
        uint8_t d7 = source[49];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 0x01 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 0x02 : 0;
          pixel |= (d2 & (0x80 >> px)) ? 0x04 : 0;
          pixel |= (d3 & (0x80 >> px)) ? 0x08 : 0;
          pixel |= (d4 & (0x80 >> px)) ? 0x10 : 0;
          pixel |= (d5 & (0x80 >> px)) ? 0x20 : 0;
          pixel |= (d6 & (0x80 >> px)) ? 0x40 : 0;
          pixel |= (d7 & (0x80 >> px)) ? 0x80 : 0;
          dest[(ty * 8 + py) * 512 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
        }
        source += 2;
      }
      source += 48;
    }
  }
}

void VramViewer::refreshMode7(const uint8_t *source, uint32_t *dest) {
  for(unsigned ty = 0; ty < 16; ty++) {
    for(unsigned tx = 0; tx < 16; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = source[1];
          dest[(ty * 8 + py) * 512 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
          source += 2;
        }
      }
    }
  }
}

void VramViewer::setDepth2bpp()  { bpp = 2; refresh(); }
void VramViewer::setDepth4bpp()  { bpp = 4; refresh(); }
void VramViewer::setDepth8bpp()  { bpp = 8; refresh(); }
void VramViewer::setDepthMode7() { bpp = 7; refresh(); }

void VramViewer::Canvas::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.drawImage(0, 0, *image);
}

VramViewer::Canvas::Canvas() {
  image = new QImage(512, 512, QImage::Format_RGB32);
  image->fill(0x800000);
}
