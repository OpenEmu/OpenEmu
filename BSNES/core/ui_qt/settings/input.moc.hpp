class InputSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *title;
  QHBoxLayout *selection;
  QComboBox *port;
  QComboBox *device;
  QTreeWidget *list;
  QHBoxLayout *controls;
  QPushButton *assign;
  QPushButton *assignAll;
  QPushButton *unassign;

  void syncUi();
  InputSettingsWindow();

public slots:
  void portChanged();
  void reloadList();
  void listChanged();
  void updateList();
  void assignKey();
  void assignAllKeys();
  void unassignKey();

private:
  array<InputGroup*> deviceItem;
  array<QTreeWidgetItem*> listItem;
} *inputSettingsWindow;
