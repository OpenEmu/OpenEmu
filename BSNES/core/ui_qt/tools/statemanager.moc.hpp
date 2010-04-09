class StateManagerWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *title;
  QTreeWidget *list;
  QHBoxLayout *controlLayout;
  QLabel *descLabel;
  QLineEdit *descEdit;
  QHBoxLayout *buttonLayout;
  QPushButton *loadState;
  QPushButton *saveState;
  QPushButton *createState;
  QPushButton *deleteState;

  QMenu *menu;
  QAction *loadStateItem;
  QAction *saveStateItem;
  QAction *createStateItem;
  QAction *deleteStateItem;

  void syncUi();
  void reloadList();
  void updateItem(QTreeWidgetItem*);
  StateManagerWindow();

public slots:
  void popupMenu(const QPoint&);
  void listChanged();
  void textEdited();
  void loadSelectedState();
  void saveSelectedState();
  void createNewState();
  void deleteSelectedState();
} *stateManagerWindow;
