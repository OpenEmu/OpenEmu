class HexEditor : public QTextEdit {
  Q_OBJECT

public:
  QHBoxLayout *layout;
  QScrollBar *scrollbar;

  function<uint8 (unsigned)> reader;
  function<void (unsigned, uint8)> writer;
  void keyPressEvent(QKeyEvent*);
  unsigned hexOffset;
  unsigned hexSize;

  void setOffset(unsigned newOffset);
  void setSize(unsigned newSize);

  void update();
  HexEditor();

public slots:
  void sliderMoved();
};
