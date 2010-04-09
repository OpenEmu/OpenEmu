class VideoSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *title;
  QGridLayout *sliders;
  QLabel *lcontrast;
  QSlider *contrast;
  QLabel *lbrightness;
  QSlider *brightness;
  QLabel *lgamma;
  QSlider *gamma;
  QHBoxLayout *options;
  QCheckBox *enableGammaRamp;

  void syncUi();
  VideoSettingsWindow();

public slots:
  void gammaRampToggle(int);
  void contrastAdjust(int);
  void brightnessAdjust(int);
  void gammaAdjust(int);
} *videoSettingsWindow;
