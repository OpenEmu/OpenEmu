class CheatEditorWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *title;
  QTreeWidget *list;
  QGridLayout *controlLayout;
  QLabel *descLabel;
  QLineEdit *descEdit;
  QLabel *codeLabel;
  QLineEdit *codeEdit;
  QHBoxLayout *buttonLayout;
  QPushButton *addCode;
  QPushButton *deleteCode;

  QMenu *menu;
  QAction *deleteCodeItem;
  QAction *addCodeItem;

  void syncUi();
  void updateItem(QTreeWidgetItem*);
  CheatEditorWindow();

public slots:
  void popupMenu(const QPoint&);
  void reloadList();
  void listChanged();
  void textEdited();
  void updateCodeStatus();
  void toggleCodeStatus();
  void addNewCode();
  void deleteSelectedCode();
} *cheatEditorWindow;
