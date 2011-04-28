class OamViewer : public QbWindow {
  Q_OBJECT

public:
  QHBoxLayout *layout;
  QTreeWidget *list;
  QVBoxLayout *controlLayout;
  struct Canvas : public QWidget {
    QImage *image;
    void paintEvent(QPaintEvent*);
    Canvas();
  } *canvas;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;

  void autoUpdate();
  OamViewer();

public slots:
  void show();
  void refresh();
};

extern OamViewer *oamViewer;
