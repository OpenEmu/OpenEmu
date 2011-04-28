class AdvancedSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;

  QGridLayout *driverLayout;
  QLabel *videoLabel;
  QLabel *audioLabel;
  QLabel *inputLabel;
  QComboBox *videoDriver;
  QComboBox *audioDriver;
  QComboBox *inputDriver;
  QLabel *driverInfo;

  QLabel *regionTitle;
  QHBoxLayout *regionLayout;
  QButtonGroup *regionGroup;
  QRadioButton *regionAuto;
  QRadioButton *regionNTSC;
  QRadioButton *regionPAL;

  QLabel *portTitle;
  QHBoxLayout *portLayout;
  QButtonGroup *portGroup;
  QRadioButton *portSatellaview;
  QRadioButton *portNone;
  QWidget *portSpacer;

  QLabel *focusTitle;
  QHBoxLayout *focusLayout;
  QButtonGroup *focusButtonGroup;
  QRadioButton *focusPause;
  QRadioButton *focusIgnore;
  QRadioButton *focusAllow;

  QLabel *rewindTitle;
  QCheckBox *rewindEnable;

  void initializeUi();
  AdvancedSettingsWindow();

public slots:
  void videoDriverChange(int index);
  void audioDriverChange(int index);
  void inputDriverChange(int index);
  void setRegionAuto();
  void setRegionNTSC();
  void setRegionPAL();
  void setPortSatellaview();
  void setPortNone();
  void pauseWithoutFocus();
  void ignoreInputWithoutFocus();
  void allowInputWithoutFocus();
  void toggleRewindEnable();
};

extern AdvancedSettingsWindow *advancedSettingsWindow;
