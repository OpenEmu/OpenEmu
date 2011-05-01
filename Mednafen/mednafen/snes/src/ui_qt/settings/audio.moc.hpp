class AudioSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QHBoxLayout *boxes;
  QLabel *frequencyLabel;
  QComboBox *frequency;
  QLabel *latencyLabel;
  QComboBox *latency;
  QGridLayout *sliders;
  QLabel *volumeLabel;
  QLabel *volumeValue;
  QSlider *volume;
  QLabel *frequencySkewLabel;
  QLabel *frequencySkewValue;
  QSlider *frequencySkew;

  void syncUi();
  AudioSettingsWindow();

public slots:
  void frequencyChange(int value);
  void latencyChange(int value);
  void volumeAdjust(int value);
  void frequencySkewAdjust(int value);
};

extern AudioSettingsWindow *audioSettingsWindow;
