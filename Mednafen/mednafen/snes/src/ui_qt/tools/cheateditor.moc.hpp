class CheatEditorWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTreeWidget *list;
  QGridLayout *gridLayout;
  QLabel *codeLabel;
  QLineEdit *codeEdit;
  QLabel *descLabel;
  QLineEdit *descEdit;
  QHBoxLayout *controlLayout;
  QPushButton *clearButton;

  void load(const char *filename);
  void save(const char *filename);
  void update();

  void synchronize();
  CheatEditorWindow();

private slots:
  void bind();
  void listChanged();
  void codeEdited();
  void descEdited();
  void clearSelected();
};

extern CheatEditorWindow *cheatEditorWindow;
