void HexEditor::keyPressEvent(QKeyEvent *event) {
  QTextCursor cursor = textCursor();
  unsigned x = cursor.position() % 56;
  unsigned y = cursor.position() / 56;

  int hexCode = -1;
  switch(event->key()) {
    case Qt::Key_0: hexCode =  0; break;
    case Qt::Key_1: hexCode =  1; break;
    case Qt::Key_2: hexCode =  2; break;
    case Qt::Key_3: hexCode =  3; break;
    case Qt::Key_4: hexCode =  4; break;
    case Qt::Key_5: hexCode =  5; break;
    case Qt::Key_6: hexCode =  6; break;
    case Qt::Key_7: hexCode =  7; break;
    case Qt::Key_8: hexCode =  8; break;
    case Qt::Key_9: hexCode =  9; break;
    case Qt::Key_A: hexCode = 10; break;
    case Qt::Key_B: hexCode = 11; break;
    case Qt::Key_C: hexCode = 12; break;
    case Qt::Key_D: hexCode = 13; break;
    case Qt::Key_E: hexCode = 14; break;
    case Qt::Key_F: hexCode = 15; break;
  }

  if(cursor.hasSelection() == false && hexCode != -1) {
    bool cursorOffsetValid = (x >= 8 && ((x - 8) % 3) != 2);
    if(cursorOffsetValid) {
      bool nibble = (x - 8) % 3;  //0 = top nibble, 1 = bottom nibble
      unsigned cursorOffset = y * 16 + ((x - 8) / 3);
      unsigned effectiveOffset = hexOffset + cursorOffset;
      if(effectiveOffset >= hexSize) effectiveOffset %= hexSize;

      uint8 data = reader(effectiveOffset);
      data &= (nibble == 0 ? 0x0f : 0xf0);
      data |= (nibble == 0 ? (hexCode << 4) : (hexCode << 0));
      writer(effectiveOffset, data);
      update();

      cursor.setPosition(y * 56 + x + 1);  //advance cursor
      setTextCursor(cursor);
    }
  } else {
    //allow navigation keys to move cursor, but block text input
    setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    QTextEdit::keyPressEvent(event);
    setTextInteractionFlags(Qt::TextEditorInteraction);
  }
}

void HexEditor::setOffset(unsigned newOffset) {
  hexOffset = newOffset;
  scrollbar->setSliderPosition(hexOffset / 16);
}

void HexEditor::setSize(unsigned newSize) {
  hexSize = newSize;
  scrollbar->setRange(0, hexSize / 16 - 16);
}

void HexEditor::update() {
  string output;
  char temp[256];
  unsigned offset = hexOffset;

  for(unsigned y = 0; y < 16; y++) {
    if(offset >= hexSize) break;
    sprintf(temp, "%.6x", offset & 0xffffff);
    output << "<font color='#606060'>" << temp << "</font>&nbsp;&nbsp;";

    for(unsigned x = 0; x < 16; x++) {
      if(offset >= hexSize) break;
      sprintf(temp, "%.2x", reader(offset++));
      output << "<font color='" << ((x & 1) ? "#000080" : "#0000ff") << "'>" << temp << "</font>";
      if(x != 15) output << "&nbsp;";
    }

    if(y != 15) output << "<br>";
  }

  setHtml(utf8() << output);
}

void HexEditor::sliderMoved() {
  unsigned offset = scrollbar->sliderPosition();
  hexOffset = offset * 16;
  update();
}

HexEditor::HexEditor() {
  QFont font(Style::Monospace);
  setFont(font);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setMinimumWidth((57 + 2) * fontMetrics().width(' '));
  setMinimumHeight((16 + 1) * fontMetrics().height());

  layout = new QHBoxLayout;
  layout->setAlignment(Qt::AlignRight);
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  scrollbar = new QScrollBar(Qt::Vertical);
  scrollbar->setSingleStep(1);
  scrollbar->setPageStep(16);
  layout->addWidget(scrollbar);

  connect(scrollbar, SIGNAL(actionTriggered(int)), this, SLOT(sliderMoved()));
}
