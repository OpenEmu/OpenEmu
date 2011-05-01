#include "cgram-viewer.moc"
CgramViewer *cgramViewer;

void CgramViewer::show() {
  QbWindow::show();
  refresh();
}

void CgramViewer::refresh() {
  if(SNES::cartridge.loaded() == false) {
    canvas->image->fill(0x000000);
    colorInfo->setText("");
    canvas->update();
    return;
  }

  uint32_t *buffer = (uint32_t*)canvas->image->bits();
  for(unsigned i = 0; i < 256; i++) {
    unsigned x = i % 16;
    unsigned y = i / 16;

    uint16_t color = SNES::memory::cgram[i * 2 + 0];
    color |= SNES::memory::cgram[i * 2 + 1] << 8;

    uint8_t r = (color >>  0) & 31;
    uint8_t g = (color >>  5) & 31;
    uint8_t b = (color >> 10) & 31;

    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);

    uint32_t output = (r << 16) | (g << 8) | (b << 0);

    for(unsigned py = 0; py < 16; py++) {
      for(unsigned px = 0; px < 16; px++) {
        buffer[(y * 16 + py) * (16 * 16) + (x * 16 + px)] = output;
      }
    }

    if(i != currentSelection) continue;

    //draw a dotted box black-and-white around the selected color
    for(unsigned py = 0; py < 16; py++) {
      for(unsigned px = 0; px < 16; px++) {
        if(py == 0 || py == 15 || px == 0 || px == 15) {
          uint32_t color = ((px + py) & 2) ? 0xffffff : 0x000000;
          buffer[(y * 16 + py) * (16 * 16) + (x * 16 + px)] = color;
        }
      }
    }

    string text;
    char temp[256];
    text << "<table>";
    text << "<tr><td>Index:</td><td>" << currentSelection << "</td></tr>";
    sprintf(temp, "%.4x", color);
    text << "<tr><td>Value:</td><td>0x" << temp << "</td></tr>";
    text << "<tr><td>Red:</td><td>" << (unsigned)(r >> 3) << "</td></tr>";
    text << "<tr><td>Green:</td><td>" << (unsigned)(g >> 3) << "</td></tr>";
    text << "<tr><td>Blue:</td><td>" << (unsigned)(b >> 3) << "</td></tr>";
    text << "</table>";
    colorInfo->setText(text);
  }

  canvas->update();
}

void CgramViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void CgramViewer::setSelection(unsigned index) {
  currentSelection = index;
  refresh();
}

CgramViewer::CgramViewer() : QbWindow(config().geometry.cgramViewer) {
  currentSelection = 0;

  setObjectName("cgram-viewer");
  setWindowTitle("Palette Viewer");

  layout = new QHBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  canvas = new Canvas;
  canvas->setFixedSize(16 * 16, 16 * 16);
  layout->addWidget(canvas);

  controlLayout = new QVBoxLayout;
  controlLayout->setAlignment(Qt::AlignTop);
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);
  controlLayout->addSpacing(Style::WidgetSpacing);

  colorInfo = new QLabel;
  controlLayout->addWidget(colorInfo);

  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
}

void CgramViewer::Canvas::mousePressEvent(QMouseEvent *event) {
  cgramViewer->setSelection((event->y() / 16) * 16 + (event->x() / 16));
}

void CgramViewer::Canvas::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.drawImage(0, 0, *image);
}

CgramViewer::Canvas::Canvas() {
  image = new QImage(16 * 16, 16 * 16, QImage::Format_RGB32);
  image->fill(0x000000);
}
