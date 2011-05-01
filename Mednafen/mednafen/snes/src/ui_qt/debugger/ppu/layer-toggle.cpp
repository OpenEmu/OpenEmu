#include "layer-toggle.moc"
LayerToggle *layerToggle;

LayerToggle::LayerToggle() : QbWindow(config().geometry.layerToggle) {
  setObjectName("layer-toggle");
  setWindowTitle("S-PPU Layer Toggle");

  layout = new QGridLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(Style::WindowMargin);
  layout->setHorizontalSpacing(Style::WidgetSpacing);
  layout->setVerticalSpacing(0);
  setLayout(layout);

  bg1Label = new QLabel("BG1:");
  layout->addWidget(bg1Label, 0, 0);

  bg1pri0 = new QCheckBox("Priority 0");
  bg1pri0->setChecked(true);
  layout->addWidget(bg1pri0, 0, 1);

  bg1pri1 = new QCheckBox("Priority 1");
  bg1pri1->setChecked(true);
  layout->addWidget(bg1pri1, 0, 2);

  bg2Label = new QLabel("BG2:");
  layout->addWidget(bg2Label, 1, 0);

  bg2pri0 = new QCheckBox("Priority 0");
  bg2pri0->setChecked(true);
  layout->addWidget(bg2pri0, 1, 1);

  bg2pri1 = new QCheckBox("Priority 1");
  bg2pri1->setChecked(true);
  layout->addWidget(bg2pri1, 1, 2);

  bg3Label = new QLabel("BG3:");
  layout->addWidget(bg3Label, 2, 0);

  bg3pri0 = new QCheckBox("Priority 0");
  bg3pri0->setChecked(true);
  layout->addWidget(bg3pri0, 2, 1);

  bg3pri1 = new QCheckBox("Priority 1");
  bg3pri1->setChecked(true);
  layout->addWidget(bg3pri1, 2, 2);

  bg4Label = new QLabel("BG4:");
  layout->addWidget(bg4Label, 3, 0);

  bg4pri0 = new QCheckBox("Priority 0");
  bg4pri0->setChecked(true);
  layout->addWidget(bg4pri0, 3, 1);

  bg4pri1 = new QCheckBox("Priority 1");
  bg4pri1->setChecked(true);
  layout->addWidget(bg4pri1, 3, 2);

  oamLabel = new QLabel("OAM:");
  layout->addWidget(oamLabel, 4, 0);

  oampri0 = new QCheckBox("Priority 0");
  oampri0->setChecked(true);
  layout->addWidget(oampri0, 4, 1);

  oampri1 = new QCheckBox("Priority 1");
  oampri1->setChecked(true);
  layout->addWidget(oampri1, 4, 2);

  oampri2 = new QCheckBox("Priority 2");
  oampri2->setChecked(true);
  layout->addWidget(oampri2, 4, 3);

  oampri3 = new QCheckBox("Priority 3");
  oampri3->setChecked(true);
  layout->addWidget(oampri3, 4, 4);

  connect(bg1pri0, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(bg1pri1, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(bg2pri0, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(bg2pri1, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(bg3pri0, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(bg3pri1, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(bg4pri0, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(bg4pri1, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(oampri0, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(oampri1, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(oampri2, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
  connect(oampri3, SIGNAL(stateChanged(int)), this, SLOT(toggleLayer()));
}

void LayerToggle::toggleLayer() {
  if(sender() == bg1pri0) SNES::ppu.bg1_enabled[0] = bg1pri0->isChecked();
  if(sender() == bg1pri1) SNES::ppu.bg1_enabled[1] = bg1pri1->isChecked();
  if(sender() == bg2pri0) SNES::ppu.bg2_enabled[0] = bg2pri0->isChecked();
  if(sender() == bg2pri1) SNES::ppu.bg2_enabled[1] = bg2pri1->isChecked();
  if(sender() == bg3pri0) SNES::ppu.bg3_enabled[0] = bg3pri0->isChecked();
  if(sender() == bg3pri1) SNES::ppu.bg3_enabled[1] = bg3pri1->isChecked();
  if(sender() == bg4pri0) SNES::ppu.bg4_enabled[0] = bg4pri0->isChecked();
  if(sender() == bg4pri1) SNES::ppu.bg4_enabled[1] = bg4pri1->isChecked();
  if(sender() == oampri0) SNES::ppu.oam_enabled[0] = oampri0->isChecked();
  if(sender() == oampri1) SNES::ppu.oam_enabled[1] = oampri1->isChecked();
  if(sender() == oampri2) SNES::ppu.oam_enabled[2] = oampri2->isChecked();
  if(sender() == oampri3) SNES::ppu.oam_enabled[3] = oampri3->isChecked();
}
