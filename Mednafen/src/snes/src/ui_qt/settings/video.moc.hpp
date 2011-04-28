class VideoSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QGridLayout *sliders;
  QLabel *contrastLabel;
  QLabel *contrastValue;
  QSlider *contrast;
  QLabel *brightnessLabel;
  QLabel *brightnessValue;
  QSlider *brightness;
  QLabel *gammaLabel;
  QLabel *gammaValue;
  QSlider *gamma;
  QLabel *scanlineLabel;
  QLabel *scanlineValue;
  QSlider *scanline;
  QHBoxLayout *options;
  QCheckBox *enableGammaRamp;

  void syncUi();
  VideoSettingsWindow();

public slots:
  void contrastAdjust(int);
  void brightnessAdjust(int);
  void gammaAdjust(int);
  void scanlineAdjust(int);
  void gammaRampToggle(int);
};

extern VideoSettingsWindow *videoSettingsWindow;
