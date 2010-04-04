class InputCaptureWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QHBoxLayout *hlayout;
  QLabel *title;
  QPushButton *mouseAxes;
  QPushButton *mouseButtons;
  QWidget *imageSpacer;
  struct ImageWidget : public QWidget {
    void paintEvent(QPaintEvent*);
  } *imageWidget;

  void activate(InputObject *object);
  void activate(InputGroup *group);
  void inputEvent(uint16_t code, bool forceAssign = false);
  void closeEvent(QCloseEvent*);
  InputCaptureWindow();

public slots:
  void assignMouseAxis();
  void assignMouseButton();

private:
  InputObject *activeObject;
  InputGroup *activeGroup;
  unsigned groupIndex;
  bool inputLock;
} *inputCaptureWindow;

class InputMouseCaptureWindow : public QbWindow {
  Q_OBJECT

public:
  enum Mode { AxisMode, ButtonMode };

  QVBoxLayout *layout;
  QLabel *info;
  QLabel *captureBox;
  QHBoxLayout *buttonLayout;
  QPushButton *xAxis;
  QPushButton *yAxis;

  void activate(Mode);
  void inputEvent(uint16_t code);
  InputMouseCaptureWindow();

public slots:
  void assignAxisX();
  void assignAxisY();

private:
  Mode activeMode;
  signed activeMouse;
} *inputMouseCaptureWindow;

class InputCalibrationWindow : public QbWindow {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *info;
  QHBoxLayout *controlLayout;
  QPushButton *ok;

  void activate(unsigned joy);
  void closeEvent(QCloseEvent*);
  InputCalibrationWindow();

public slots:
  void dismiss();

private:
  int activeJoypad;
} *inputCalibrationWindow;
