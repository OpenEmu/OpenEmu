#include "oam-viewer.moc"
OamViewer *oamViewer;

void OamViewer::show() {
  QbWindow::show();
  refresh();
}

void OamViewer::refresh() {
  canvas->image->fill(0x000000);

  QList<QTreeWidgetItem*> items = list->findItems("", Qt::MatchContains);
  for(unsigned v = 0; v < items.count(); v++) {
    QTreeWidgetItem *item = items[v];
    unsigned i = item->data(0, Qt::UserRole).toUInt();

    uint8_t d0 = SNES::memory::oam[(i << 2) + 0];
    uint8_t d1 = SNES::memory::oam[(i << 2) + 1];
    uint8_t d2 = SNES::memory::oam[(i << 2) + 2];
    uint8_t d3 = SNES::memory::oam[(i << 2) + 3];
    uint8_t d4 = SNES::memory::oam[512 + (i >> 2)];
    bool x    = d4 & (1 << ((i & 3) << 1));
    bool size = d4 & (2 << ((i & 3) << 1));

    unsigned width, height;
    switch(SNES::ppu.oam_base_size()) { default:
      case 0: width = !size ?  8 : 16; height = !size ?  8 : 16; break;
      case 1: width = !size ?  8 : 32; height = !size ?  8 : 32; break;
      case 2: width = !size ?  8 : 64; height = !size ?  8 : 64; break;
      case 3: width = !size ? 16 : 32; height = !size ? 16 : 32; break;
      case 4: width = !size ? 16 : 64; height = !size ? 16 : 64; break;
      case 5: width = !size ? 32 : 64; height = !size ? 32 : 64; break;
      case 6: width = !size ? 16 : 32; height = !size ? 32 : 64; break;
      case 7: width = !size ? 16 : 32; height = !size ? 32 : 32; break;
    }

    signed xpos = (x << 8) + d0;
    if(xpos > 256) xpos = sclip<9>(xpos);
    unsigned ypos = d1;
    unsigned character = d2;
    unsigned priority = (d3 >> 4) & 3;
    unsigned palette = (d3 >> 1) & 7;
    string flags;
    if(d3 & 0x80) flags << "V";
    if(d3 & 0x40) flags << "H";
    if(d3 & 0x01) flags << "N";

    item->setText(1, string() << width << "x" << height);
    item->setText(2, string() << xpos);
    item->setText(3, string() << ypos);
    item->setText(4, string() << character);
    item->setText(5, string() << priority);
    item->setText(6, string() << palette);
    item->setText(7, flags);
  }

  for(unsigned i = 0; i <= 7; i++) list->resizeColumnToContents(i);
  canvas->update();
}

void OamViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

OamViewer::OamViewer() : QbWindow(config().geometry.oamViewer) {
  setObjectName("oam-viewer");
  setWindowTitle("Sprite Viewer");

  layout = new QHBoxLayout;
  layout->setAlignment(Qt::AlignLeft);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  list = new QTreeWidget;
  list->setColumnCount(8);
  list->setHeaderLabels(QStringList() << "#" << "Size" << "X" << "Y" << "Char" << "Pri" << "Pal" << "Flags");
  list->setAllColumnsShowFocus(true);
  list->setAlternatingRowColors(true);
  list->setRootIsDecorated(false);
  list->setSortingEnabled(false);
  layout->addWidget(list);

  for(unsigned i = 0; i < 128; i++) {
    QTreeWidgetItem *item = new QTreeWidgetItem(list);
    item->setData(0, Qt::UserRole, QVariant(i));
    item->setTextAlignment(0, Qt::AlignHCenter);
    item->setTextAlignment(1, Qt::AlignHCenter);
    item->setTextAlignment(2, Qt::AlignRight);
    item->setTextAlignment(3, Qt::AlignRight);
    item->setTextAlignment(4, Qt::AlignRight);
    item->setTextAlignment(5, Qt::AlignRight);
    item->setTextAlignment(6, Qt::AlignRight);
    item->setTextAlignment(7, Qt::AlignLeft);
    item->setText(0, string() << i);
  }

  controlLayout = new QVBoxLayout;
  controlLayout->setAlignment(Qt::AlignTop);
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  canvas = new Canvas;
  canvas->setFixedSize(128, 128);
  controlLayout->addWidget(canvas);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);

  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
}

void OamViewer::Canvas::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.drawImage(0, 0, *image);
}

OamViewer::Canvas::Canvas() {
  image = new QImage(128, 128, QImage::Format_RGB32);
  image->fill(0x000000);
}
