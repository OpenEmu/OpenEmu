class StateManagerWindow : public QWidget {
  Q_OBJECT

public:
  enum { StateCount = 32 };

  QVBoxLayout *layout;
  QTreeWidget *list;
  QHBoxLayout *infoLayout;
  QLabel *descriptionLabel;
  QLineEdit *descriptionText;
  QHBoxLayout *controlLayout;
  QWidget *spacer;
  QPushButton *loadButton;
  QPushButton *saveButton;
  QPushButton *eraseButton;

  void reload();
  void update();

  StateManagerWindow();

public slots:
  void synchronize();
  void writeDescription();
  void loadAction();
  void saveAction();
  void eraseAction();

private:
  string filename() const;
  bool isStateValid(unsigned slot);
  string getStateDescription(unsigned slot);
  void setStateDescription(unsigned slot, const string&);
  void loadState(unsigned slot);
  void saveState(unsigned slot);
  void eraseState(unsigned slot);
};

extern StateManagerWindow *stateManagerWindow;
