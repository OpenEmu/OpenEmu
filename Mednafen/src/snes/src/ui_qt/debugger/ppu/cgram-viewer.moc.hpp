class CgramViewer : public QbWindow {
  Q_OBJECT

public:
  QHBoxLayout *layout;
  struct Canvas : public QWidget {
    QImage *image;
    void mousePressEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);
    Canvas();
  } *canvas;
  QVBoxLayout *controlLayout;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  QLabel *colorInfo;

  void setSelection(unsigned);
  void autoUpdate();
  CgramViewer();

public slots:
  void show();
  void refresh();

private:
  unsigned currentSelection;
};

extern CgramViewer *cgramViewer;
