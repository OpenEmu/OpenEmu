class VramViewer : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QHBoxLayout *controlLayout;
  QRadioButton *depth2bpp;
  QRadioButton *depth4bpp;
  QRadioButton *depth8bpp;
  QRadioButton *depthMode7;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  struct Canvas : public QWidget {
    QImage *image;
    void paintEvent(QPaintEvent*);
    Canvas();
  } *canvas;

  void autoUpdate();
  VramViewer();

public slots:
  void show();
  void refresh();
  void setDepth2bpp();
  void setDepth4bpp();
  void setDepth8bpp();
  void setDepthMode7();

private:
  unsigned bpp;
  void refresh2bpp(const uint8_t*, uint32_t*);
  void refresh4bpp(const uint8_t*, uint32_t*);
  void refresh8bpp(const uint8_t*, uint32_t*);
  void refreshMode7(const uint8_t*, uint32_t*);
};

extern VramViewer *vramViewer;
